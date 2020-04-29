#include <error.h>
#include <pmm.h>
#include <pmm_firstfit.h>
#include <printf.h>
#include <string.h>
#include <swap.h>

size_t npage;
struct Page *pages;
extern pde_t _boot_pgdir;
pde_t *boot_pgdir = &_boot_pgdir;
const struct pmm_manager *pmm_manager;

pte_t *const vpt = (pte_t *)VPT;
pde_t *const vpd = (pde_t *)(PDX(VPT) << PDSHIFT | PDX(VPT) << PGSHIFT);

static void check_alloc_page(void);
static void check_pgdir(void);
static void check_boot_pgdir(void);

// 初始化pmm_manager实例
static void init_pmm_manager(void) {
  pmm_manager = &firstfit_pmm_manager;
  printf("memory manager: %s\n", pmm_manager->name);
  pmm_manager->init();
}

// 为空闲内存建立Page结构
static void init_memmap(struct Page *base, size_t n) {
  pmm_manager->init_memmap(base, n);
}

// 分配n个连续物理内存页
struct Page *alloc_pages(size_t n) {
  struct Page *page = NULL;

  while (1) {
    page = pmm_manager->alloc_pages(n);
    if (page != NULL || n > 1 || swap_init_ok == 0) break;

    extern struct mm_struct *check_mm_struct;
    swap_out(check_mm_struct, n, 0);
  }
  return page;
}

// 释放n个连续物理内存页
void free_pages(struct Page *base, size_t n) {
  pmm_manager->free_pages(base, n);
}

// 获得剩余空闲页个数
size_t nr_free_pages(void) { return pmm_manager->nr_free_pages(); }

static void page_init(void) {
  extern char end[];
  uintptr_t maxpa;
  int i;

  maxpa = 0x04000000;
  npage = maxpa / PGSIZE;
  pages = (struct Page *)ROUND((void *)end, PGSIZE);

  for (i = 0; i < npage; i++) {
    SetPageReserved(pages + i);
  }

  uintptr_t freemem = PADDR((uintptr_t)pages + sizeof(struct Page) * npage);

  freemem = ROUND(freemem, PGSIZE);
  maxpa = ROUNDDOWN(maxpa, PGSIZE);
  init_memmap(pa2page(freemem), (maxpa - freemem) / PGSIZE);
}

void pmm_init(void) {
  init_pmm_manager();
  page_init();

  check_alloc_page();
  check_pgdir();

  boot_pgdir[PDX(VPT)] = PADDR(boot_pgdir) | PTE_V;
  check_boot_pgdir();

  print_pgdir();
}

/* 根据虚拟地址va，从页目录中找到对应的页表，再从页表中找到对应的页表项，返回页表项内核虚拟地址。
如果在页目录发现对应的页表不存在，根据create参数决定是否创建页表并返回页表对应的页表项。*/
pte_t *get_pte(pde_t *pgdir, uintptr_t va, bool create) {
  pde_t *pdep = &pgdir[PDX(va)];
  if (!(*pdep & PTE_V)) {
    struct Page *page;
    if (!create || (page = alloc_page()) == NULL) {
      return NULL;
    }
    set_page_ref(page, 1);
    uintptr_t pa = page2pa(page);
    memset(KADDR(pa), 0, PGSIZE);
    *pdep = pa | PTE_D | PTE_V;
  }
  return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(va)];
}

// get_page - get revated Page struct for linear address va using PDT pgdir
struct Page *get_page(pde_t *pgdir, uintptr_t va, pte_t **ptep_store) {
  pte_t *ptep = get_pte(pgdir, va, 0);
  if (ptep_store != NULL) {
    *ptep_store = ptep;
  }
  if (ptep != NULL && *ptep & PTE_V) {
    return pte2page(*ptep);
  }
  return NULL;
}

/* 取消虚拟地址va对应的虚拟页和它目前映射的物理页之间的映射关系，并清除虚拟地址va的对应的TLB表项
 */
void page_remove(pde_t *pgdir, uintptr_t va) {
  pte_t *ptep = get_pte(pgdir, va, 0);
  if (ptep != NULL) {
    if (*ptep & PTE_V) {
      struct Page *page = pte2page(*ptep);
      if (page_ref_dec(page) == 0) {
        free_page(page);
      }
      *ptep = 0;
      tlb_invalidate(pgdir, va);
    }
  }
}

/* 将虚拟地址va对应的虚拟页与page变量对应的物理页建立映射 */
int page_insert(pde_t *pgdir, struct Page *page, uintptr_t va, uint32_t perm) {
  pte_t *ptep = get_pte(pgdir, va, 1);
  if (ptep == NULL) {
    return -E_NO_MEM;
  }
  page_ref_inc(page);
  if (*ptep & PTE_V) {
    struct Page *p = pte2page(*ptep);
    if (p == page) {
      page_ref_dec(page);
    } else {
      page_remove_pte(pgdir, va, ptep);
    }
  }
  *ptep = page2pa(page) | PTE_V | perm;
  tlb_invalidate(pgdir, va);
  return 0;
}

/* 清除虚拟地址va对应的TLB表项 */
void tlb_invalidate(pde_t *pgdir, uintptr_t va) { tlb_out(PTE_ADDR(va)); }


/* 调用alloc_page和page_insert函数为虚拟地址va分配一个物理页并建立映射关系 */
struct Page *pgdir_alloc_page(pde_t *pgdir, uintptr_t va, uint32_t perm) {
  struct Page *page = alloc_page();
  if (page != NULL) {
    if (page_insert(pgdir, page, va, perm) != 0) {
      free_page(page);
      return NULL;
    }
    if (swap_init_ok) {
      if (check_mm_struct != NULL) {
        swap_map_swappable(check_mm_struct, va, page, 0);
        page->pra_vaddr = va;
        assert(page_ref(page) == 1);
      }
    }
  }

  return page;
}

void unmap_range(pde_t *pgdir, uintptr_t start, uintptr_t end) {
  assert(start % PGSIZE == 0 && end % PGSIZE == 0);

  do {
    pte_t *ptep = get_pte(pgdir, start, 0);
    if (ptep == NULL) {
      start = ROUNDDOWN(start + PDSIZE, PDSIZE);
      continue;
    }
    if (*ptep != 0) {
      page_remove_pte(pgdir, start, ptep);
    }
    start += PGSIZE;
  } while (start != 0 && start < end);
}

void exit_range(pde_t *pgdir, uintptr_t start, uintptr_t end) {
  assert(start % PGSIZE == 0 && end % PGSIZE == 0);

  start = ROUNDDOWN(start, PDSIZE);
  do {
    int pde_idx = PDX(start);
    if (pgdir[pde_idx] & PTE_V) {
      free_page(pde2page(pgdir[pde_idx]));
      pgdir[pde_idx] = 0;
    }
    start += PDSIZE;
  } while (start != 0 && start < end);
}

int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end,
               bool share) {
  assert(start % PGSIZE == 0 && end % PGSIZE == 0);

  // copy content by page unit.
  do {
    // call get_pte to find process A's pte according to the addr start
    pte_t *ptep = get_pte(from, start, 0), *nptep;
    if (ptep == NULL) {
      start = ROUNDDOWN(start + PDSIZE, PDSIZE);
      continue;
    }
    // call get_pte to find process B's pte according to the addr start. If pte
    // is NULL, just alloc a PT
    if (*ptep & PTE_V) {
      if ((nptep = get_pte(to, start, 1)) == NULL) {
        return -E_NO_MEM;
      }
      uint32_t perm = (*ptep & (PTE_V | PTE_D));
      // get page from ptep
      struct Page *page = pte2page(*ptep);
      // alloc a page for process B
      struct Page *npage = alloc_page();
      assert(page != NULL);
      assert(npage != NULL);
      int ret = 0;

      void *kva_src = page2kva(page);
      void *kva_dst = page2kva(npage);

      memcpy(kva_dst, kva_src, PGSIZE);

      ret = page_insert(to, npage, start, perm);
      assert(ret == 0);
    }
    start += PGSIZE;
  } while (start != 0 && start < end);
  return 0;
}

static void check_alloc_page(void) {
  pmm_manager->check();
  printf("check_alloc_page() succeeded!\n");
}

static void check_pgdir(void) {
  assert(boot_pgdir != NULL && (uint32_t)PGOFF(boot_pgdir) == 0);
  assert(get_page(boot_pgdir, 0x0, NULL) == NULL);

  struct Page *p1, *p2;
  p1 = alloc_page();
  assert(page_insert(boot_pgdir, p1, 0x0, 0) == 0);

  pte_t *ptep;
  assert((ptep = get_pte(boot_pgdir, 0x0, 0)) != NULL);
  assert(pte2page(*ptep) == p1);
  assert(page_ref(p1) == 1);

  ptep = &((pte_t *)KADDR(PDE_ADDR(boot_pgdir[0])))[1];
  assert(get_pte(boot_pgdir, PGSIZE, 0) == ptep);

  p2 = alloc_page();
  assert(page_insert(boot_pgdir, p2, PGSIZE, PTE_D) == 0);
  assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
  assert(*ptep & PTE_D);
  assert(page_ref(p2) == 1);

  assert(page_insert(boot_pgdir, p1, PGSIZE, 0) == 0);
  assert(page_ref(p1) == 2);
  assert(page_ref(p2) == 0);
  assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
  assert(pte2page(*ptep) == p1);

  page_remove(boot_pgdir, 0x0);
  assert(page_ref(p1) == 1);
  assert(page_ref(p2) == 0);

  page_remove(boot_pgdir, PGSIZE);
  assert(page_ref(p1) == 0);
  assert(page_ref(p2) == 0);

  assert(page_ref(pde2page(boot_pgdir[0])) == 1);
  free_page(pde2page(boot_pgdir[0]));
  boot_pgdir[0] = 0;

  printf("check_pgdir() succeeded!\n");
}

static void check_boot_pgdir(void) {
  pte_t *ptep;

  assert(PDE_ADDR(boot_pgdir[PDX(VPT)]) == PADDR(boot_pgdir));

  assert(boot_pgdir[0] == 0);

  struct Page *p;
  p = alloc_page();
  assert(page_insert(boot_pgdir, p, 0x100, PTE_D) == 0);
  assert(page_ref(p) == 1);
  assert(page_insert(boot_pgdir, p, 0x100 + PGSIZE, PTE_D) == 0);
  assert(page_ref(p) == 2);

  const char *str = "ucore: Hello world!!";
  strcpy((void *)0x100, str);
  assert(strcmp((void *)0x100, (void *)(0x100 + PGSIZE)) == 0);

  *(char *)(page2kva(p) + 0x100) = '\0';
  assert(strlen((const char *)0x100) == 0);

  page_remove(boot_pgdir, 0x100);
  page_remove(boot_pgdir, 0x100 + PGSIZE);
  free_page(pde2page(boot_pgdir[0]));
  boot_pgdir[0] = 0;

  printf("check_boot_pgdir() succeeded!\n");
}

// perm2str - use string 'u,r,w,-' to present the permission
static const char *perm2str(int perm) {
  static char str[4];
  str[0] = 'r';
  str[1] = (perm & PTE_D) ? 'w' : '-';
  str[2] = (perm & PTE_G) ? 'g' : '-';
  str[3] = '\0';
  return str;
}

// get_pgtable_items - In [left, right] range of PDT or PT, find a continuous
// linear addr space
//                  - (left_store*X_SIZE~right_store*X_SIZE) for PDT or PT
//                  - X_SIZE=PDSIZE=4M, if PDT; X_SIZE=PGSIZE=4K, if PT
// paramemters:
//  left:        no use ???
//  right:       the high side of table's range
//  start:       the low side of table's range
//  table:       the beginning addr of table
//  left_store:  the pointer of the high side of table's next range
//  right_store: the pointer of the low side of table's next range
// return value: 0 - not a invalid item range, perm - a valid item range with
// perm permission
static int get_pgtable_items(size_t left, size_t right, size_t start,
                             uintptr_t *table, size_t *left_store,
                             size_t *right_store) {
  if (start >= right) {
    return 0;
  }
  while (start < right && !(table[start] & PTE_V)) {
    start++;
  }
  if (start < right) {
    if (left_store != NULL) {
      *left_store = start;
    }
    int perm = (table[start++] & (PTE_G | PTE_V | PTE_D));
    while (start < right && (table[start] & (PTE_G | PTE_V | PTE_D)) == perm) {
      start++;
    }
    if (right_store != NULL) {
      *right_store = start;
    }
    return perm;
  }
  return 0;
}

 /* 按层级打印当前进程的页目录和页表 */
void print_pgdir(void) {
  printf("-------------------- BEGIN --------------------\n");
  size_t left, right = 0, perm;
  while ((perm = get_pgtable_items(0, NPDEENTRY, right, vpd, &left, &right)) !=
         0) {
    printf("PDE(%x) %x-%x %x %s\n", right - left, left * PDSIZE, right * PDSIZE,
           (right - left) * PDSIZE, perm2str(perm));
    size_t l, r = left * NPTEENTRY;
    while ((perm = get_pgtable_items(left * NPTEENTRY, right * NPTEENTRY, r,
                                     vpt, &l, &r)) != 0) {
      printf("  |-- PTE(%x) %x-%x %x %s\n", r - l, l * PGSIZE, r * PGSIZE,
             (r - l) * PGSIZE, perm2str(perm));
    }
  }
  printf("--------------------- END ---------------------\n");
}

void pageout(uintptr_t va, uintptr_t context) {
  struct Page *p = NULL;

  assert(context >= 0x80000000);

  if (p = alloc_page()) {
    p->ref++;
    page_insert((pde_t *)context, p, va, PTE_D);
    printf("pageout:\t@@@___0x%x___@@@  ins a page \n", va);
  }
}
