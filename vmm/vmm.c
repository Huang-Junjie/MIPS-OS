#include <error.h>
#include <kmalloc.h>
#include <mmu.h>
#include <pmm.h>
#include <swap.h>
#include <vmm.h>
/*
  vmm design include two parts: mm_struct (mm) & vma_struct (vma)
  mm is the memory manager for the set of continuous virtual memory
  area which have the same PDT. vma is a continuous virtual memory area.
  There a linear link list for vma & a redblack link list for vma in mm.
---------------
  mm related functions:
   golbal functions
     struct mm_struct * mm_create(void)
     void mm_destroy(struct mm_struct *mm)
     int do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr)
--------------
  vma related functions:
   global functions
     struct vma_struct * vma_create (uintptr_t vm_start, uintptr_t vm_end,...)
     void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma)
     struct vma_struct * find_vma(struct mm_struct *mm, uintptr_t addr)
   local functions
     inline void check_vma_overlap(struct vma_struct *prev, struct vma_struct
*next)
---------------
   check correctness functions
     void check_vmm(void);
     void check_vma_struct(void);
     void check_pgfault(void);
*/

static void check_vmm(void);
static void check_vma_struct(void);
static void check_pgfault(void);

// mm_create -  alloc a mm_struct & initialize it.
struct mm_struct *mm_create(void) {
  struct mm_struct *mm = kmalloc(sizeof(struct mm_struct));

  if (mm != NULL) {
    list_init(&(mm->mmap_list));
    mm->mmap_cache = NULL;
    mm->pgdir = NULL;
    mm->map_count = 0;

    if (swap_init_ok)
      swap_init_mm(mm);
    else
      mm->sm_priv = NULL;
    set_mm_count(mm, 0);
  }
  return mm;
}

// vma_create - alloc a vma_struct & initialize it. (addr range:
// vm_start~vm_end)
struct vma_struct *vma_create(uintptr_t vm_start, uintptr_t vm_end,
                              uint32_t vm_flags) {
  struct vma_struct *vma = kmalloc(sizeof(struct vma_struct));

  if (vma != NULL) {
    vma->vm_start = vm_start;
    vma->vm_end = vm_end;
    vma->vm_flags = vm_flags;
  }
  return vma;
}

// find_vma - find a vma  (vma->vm_start <= addr < vma_vm_end)
struct vma_struct *find_vma(struct mm_struct *mm, uintptr_t addr) {
  struct vma_struct *vma = NULL;
  if (mm != NULL) {
    vma = mm->mmap_cache;
    if (!(vma != NULL && vma->vm_start <= addr && vma->vm_end > addr)) {
      bool found = 0;
      list_entry_t *list = &(mm->mmap_list), *le = list;
      while ((le = list_next(le)) != list) {
        vma = le2vma(le, list_link);
        if (vma->vm_start <= addr && addr < vma->vm_end) {
          found = 1;
          break;
        }
      }
      if (!found) {
        vma = NULL;
      }
    }
    if (vma != NULL) {
      mm->mmap_cache = vma;
    }
  }
  return vma;
}

// check_vma_overlap - check if vma1 overlaps vma2 ?
static inline void check_vma_overlap(struct vma_struct *prev,
                                     struct vma_struct *next) {
  assert(prev->vm_start < prev->vm_end);
  assert(prev->vm_end <= next->vm_start);
  assert(next->vm_start < next->vm_end);
}

// insert_vma_struct -insert vma in mm's list link
void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma) {
  assert(vma->vm_start < vma->vm_end);
  list_entry_t *list = &(mm->mmap_list);
  list_entry_t *le_prev = list, *le_next;

  list_entry_t *le = list;
  while ((le = list_next(le)) != list) {
    struct vma_struct *mmap_prev = le2vma(le, list_link);
    if (mmap_prev->vm_start > vma->vm_start) {
      break;
    }
    le_prev = le;
  }

  le_next = list_next(le_prev);

  /* check overlap */
  if (le_prev != list) {
    check_vma_overlap(le2vma(le_prev, list_link), vma);
  }
  if (le_next != list) {
    check_vma_overlap(vma, le2vma(le_next, list_link));
  }

  vma->vm_mm = mm;
  list_add_after(le_prev, &(vma->list_link));

  mm->map_count++;
}

// mm_destroy - free mm and mm internal fields
void mm_destroy(struct mm_struct *mm) {
  assert(mm_count(mm) == 0);
  list_entry_t *list = &(mm->mmap_list), *le;
  while ((le = list_next(list)) != list) {
    list_del(le);
    kfree(le2vma(le, list_link));  // kfree vma
  }
  kfree(mm);  // kfree mm
  mm = NULL;
}

int mm_map(struct mm_struct *mm, uintptr_t addr, size_t len,
           uint32_t vm_flags) {
  assert(mm != NULL);
  struct vma_struct *vma;
  uintptr_t start = ROUNDDOWN(addr, PGSIZE), end = ROUND(addr + len, PGSIZE);

  if (find_vma(mm, start) || find_vma(mm, end)) {
    return -E_INVAL;
  }

  if ((vma = vma_create(start, end, vm_flags)) == NULL) {
    return -E_NO_MEM;
  }

  insert_vma_struct(mm, vma);
  return 0;
}

int dup_mmap(struct mm_struct *to, struct mm_struct *from) {
  assert(to != NULL && from != NULL);
  list_entry_t *list = &(from->mmap_list), *le = list;
  while ((le = list_prev(le)) != list) {
    struct vma_struct *vma, *nvma;
    vma = le2vma(le, list_link);
    nvma = vma_create(vma->vm_start, vma->vm_end, vma->vm_flags);
    if (nvma == NULL) {
      return -E_NO_MEM;
    }

    insert_vma_struct(to, nvma);

    bool share = 0;
    if (copy_range(to->pgdir, from->pgdir, vma->vm_start, vma->vm_end, share) !=
        0) {
      return -E_NO_MEM;
    }
  }
  return 0;
}

void exit_mmap(struct mm_struct *mm) {
  assert(mm != NULL && mm_count(mm) == 0);
  pde_t *pgdir = mm->pgdir;
  list_entry_t *list = &(mm->mmap_list), *le = list;
  while ((le = list_next(le)) != list) {
    struct vma_struct *vma = le2vma(le, list_link);
    unmap_range(pgdir, vma->vm_start, vma->vm_end);
  }
  while ((le = list_next(le)) != list) {
    struct vma_struct *vma = le2vma(le, list_link);
    exit_range(pgdir, vma->vm_start, vma->vm_end);
  }
}

// vmm_init - initialize virtual memory management
//          - now just call check_vmm to check correctness of vmm
void vmm_init(void) { check_vmm(); }

// check_vmm - check correctness of vmm
static void check_vmm(void) {
  size_t nr_free_pages_store = nr_free_pages();

  check_vma_struct();
  check_pgfault();

  //   assert(nr_free_pages_store == nr_free_pages());

  printf("check_vmm() succeeded.\n");
}

static void check_vma_struct(void) {
  size_t nr_free_pages_store = nr_free_pages();

  struct mm_struct *mm = mm_create();
  assert(mm != NULL);

  int step1 = 10, step2 = step1 * 10;

  int i;
  for (i = step1; i >= 1; i--) {
    struct vma_struct *vma = vma_create(i * 5, i * 5 + 2, 0);
    assert(vma != NULL);
    insert_vma_struct(mm, vma);
  }

  for (i = step1 + 1; i <= step2; i++) {
    struct vma_struct *vma = vma_create(i * 5, i * 5 + 2, 0);
    assert(vma != NULL);
    insert_vma_struct(mm, vma);
  }

  list_entry_t *le = list_next(&(mm->mmap_list));

  for (i = 1; i <= step2; i++) {
    assert(le != &(mm->mmap_list));
    struct vma_struct *mmap = le2vma(le, list_link);
    assert(mmap->vm_start == i * 5 && mmap->vm_end == i * 5 + 2);
    le = list_next(le);
  }

  for (i = 5; i <= 5 * step2; i += 5) {
    struct vma_struct *vma1 = find_vma(mm, i);
    assert(vma1 != NULL);
    struct vma_struct *vma2 = find_vma(mm, i + 1);
    assert(vma2 != NULL);
    struct vma_struct *vma3 = find_vma(mm, i + 2);
    assert(vma3 == NULL);
    struct vma_struct *vma4 = find_vma(mm, i + 3);
    assert(vma4 == NULL);
    struct vma_struct *vma5 = find_vma(mm, i + 4);
    assert(vma5 == NULL);

    assert(vma1->vm_start == i && vma1->vm_end == i + 2);
    assert(vma2->vm_start == i && vma2->vm_end == i + 2);
  }

  for (i = 4; i >= 0; i--) {
    struct vma_struct *vma_below_5 = find_vma(mm, i);
    if (vma_below_5 != NULL) {
      printf("vma_below_5: i %x, start %x, end %x\n", i, vma_below_5->vm_start,
             vma_below_5->vm_end);
    }
    assert(vma_below_5 == NULL);
  }

  mm_destroy(mm);

  //    assert(nr_free_pages_store == nr_free_pages());

  printf("check_vma_struct() succeeded!\n");
}

struct mm_struct *check_mm_struct;

// check_pgfault - check correctness of pgfault handler
static void check_pgfault(void) {
  size_t nr_free_pages_store = nr_free_pages();

  check_mm_struct = mm_create();
  assert(check_mm_struct != NULL);

  struct mm_struct *mm = check_mm_struct;
  pde_t *pgdir = mm->pgdir = boot_pgdir;
  assert(pgdir[0] == 0);

  struct vma_struct *vma = vma_create(0, PDSIZE, VM_WRITE);
  assert(vma != NULL);

  insert_vma_struct(mm, vma);

  uintptr_t addr = 0x100;
  assert(find_vma(mm, addr) == vma);

  int i, sum = 0;
  for (i = 0; i < 100; i++) {
    *(char *)(addr + i) = i;
    sum += i;
  }
  for (i = 0; i < 100; i++) {
    sum -= *(char *)(addr + i);
  }
  assert(sum == 0);

  page_remove(pgdir, ROUNDDOWN(addr, PGSIZE));
  free_page(pde2page(pgdir[0]));
  pgdir[0] = 0;

  mm->pgdir = NULL;
  mm_destroy(mm);
  check_mm_struct = NULL;

  assert(nr_free_pages_store == nr_free_pages());

  printf("check_pgfault() succeeded!\n");
}

static inline void print_pgfault(struct trapframe *tf) {
  printf("page fault at 0x%08x: %c.\n", tf->cp0_badvaddr,
         ((tf->cp0_cause >> 2) & 0x01f) == 2 ? 'R' : 'W');
}

volatile unsigned int pgfault_num = 0;
int do_pgfault(struct mm_struct *mm, uint32_t cause, uintptr_t addr) {
  int ret = -E_INVAL;
  // try to find a vma which include addr
  struct vma_struct *vma = find_vma(mm, addr);

  pgfault_num++;

  // If the addr is in the range of a mm's vma?
  if (vma == NULL || vma->vm_start > addr) {
    printf("not valid addr %x, and  can not find it in vma\n", addr);
    goto failed;
  }

  if (cause == 3 && (!(vma->vm_flags & VM_WRITE))) {
    printf(
        "do_pgfault failed: error code flag = write AND not present, but the "
        "addr's vma cannot write\n");
    goto failed;
  }

  unsigned int perm = 0;
  if (vma->vm_flags & VM_WRITE) {
    perm |= PTE_D;
  }
  addr = ROUNDDOWN(addr, PGSIZE);

  ret = -E_NO_MEM;

  pte_t *ptep = NULL;

  // try to find a pte, if pte's PT(Page Table) isn't existed, then create a PT.
  // (notice the 3th parameter '1')
  if ((ptep = get_pte(mm->pgdir, addr, 1)) == NULL) {
    printf("get_pte in do_pgfault failed\n");
    goto failed;
  }

  if (*ptep == 0) {  // if the phy addr isn't exist, then alloc a page & map the
                     // phy addr with logical addr
    if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL) {
      printf("pgdir_alloc_page in do_pgfault failed\n");
      goto failed;
    }
  } else {
    struct Page *page = NULL;
    printf("do pgfault: ptep %x, pte %x\n", ptep, *ptep);

    // if this pte is a swap entry, then load data from disk to a page with phy
    // addr and call page_insert to map the phy addr with logical addr
    if (swap_init_ok) {
      if ((ret = swap_in(mm, addr, &page)) != 0) {
        printf("swap_in in do_pgfault failed\n");
        goto failed;
      }
      page_insert(mm->pgdir, page, addr, perm);
      swap_map_swappable(mm, addr, page, 1);
      page->pra_vaddr = addr;

    } else {
      printf("no swap_init_ok but ptep is %x, failed\n", *ptep);
      goto failed;
    }
  }
  ret = 0;
failed:
  return ret;
}

int pgfault_handler(struct trapframe *tf) {
  extern struct mm_struct *check_mm_struct;
  if (check_mm_struct != NULL) {  // used for test check_swap
    print_pgfault(tf);
  }
  struct mm_struct *mm;
  if (check_mm_struct != NULL) {
    mm = check_mm_struct;
  } else {
    // if (current == NULL) {
    //     print_pgfault(tf);
    //     panic("unhandled page fault.\n");
    // }
    // mm = current->mm;
  }
  return do_pgfault(mm, (tf->cp0_cause >> 2) & 0x01f, tf->cp0_badvaddr);
}
