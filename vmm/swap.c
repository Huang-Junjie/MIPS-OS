#include <ide.h>
#include <mmu.h>
#include <pmm.h>
#include <printf.h>
#include <string.h>
#include <swap.h>
#include <swap_fifo.h>

// 页替换检查时的虚拟页数量
#define CHECK_VALID_VIR_PAGE_NUM 5
#define BEING_CHECK_VALID_VADDR 0X1000
#define CHECK_VALID_VADDR (CHECK_VALID_VIR_PAGE_NUM + 1) * 0x1000
// 页替换检查时的物理页数量
#define CHECK_VALID_PHY_PAGE_NUM 4
#define SWAP_DEV_NO 1
#define PAGE_NSECT (PGSIZE / 512)

static struct swap_manager *sm;

volatile int swap_init_ok = 0;

static void check_swap(void);

int swapfs_read(swap_entry_t entry, struct Page *page) {
  return ide_read_secs(SWAP_DEV_NO, swap_offset(entry) * PAGE_NSECT,
                       page2kva(page), PAGE_NSECT);
}

int swapfs_write(swap_entry_t entry, struct Page *page) {
  return ide_write_secs(SWAP_DEV_NO, swap_offset(entry) * PAGE_NSECT,
                        page2kva(page), PAGE_NSECT);
}

int swap_init(void) {
  sm = &fifo_swap_manager;
  int r = sm->init();

  if (r == 0) {
    swap_init_ok = 1;
    printf("swap manager: %s\n", sm->name);
    check_swap();
  }

  return r;
}

int swap_init_mm(struct mm_struct *mm) { return sm->init_mm(mm); }

int swap_tick_event(struct mm_struct *mm) { return sm->tick_event(mm); }

int swap_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page,
                       int swap_in) {
  return sm->map_swappable(mm, addr, page, swap_in);
}

int swap_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
  return sm->set_unswappable(mm, addr);
}

volatile unsigned int swap_out_num = 0;

int swap_out(struct mm_struct *mm, int n, int in_tick) {
  int i;
  for (i = 0; i != n; ++i) {
    uintptr_t v;
    struct Page *page;
    int r = sm->swap_out_victim(mm, &page, in_tick);
    if (r != 0) {
      printf("i %d, swap_out: call swap_out_victim failed\n", i);
      break;
    }

    v = page->pra_vaddr;
    pte_t *ptep = get_pte(mm->pgdir, v, 0);
    assert((*ptep & PTE_V) != 0);

    if (swapfs_write((page->pra_vaddr / PGSIZE + 1) << 8, page) != 0) {
      printf("SWAP: failed to save\n");
      sm->map_swappable(mm, v, page, 0);
      continue;
    } else {
      printf("swap_out: i %d, store page in vaddr 0x%x to disk swap entry %d\n",
             i, v, page->pra_vaddr / PGSIZE + 1);
      *ptep = (page->pra_vaddr / PGSIZE + 1) << 8;
      free_page(page);
    }

    tlb_invalidate(mm->pgdir, v);
  }
  return i;
}

int swap_in(struct mm_struct *mm, uintptr_t addr, struct Page **ptr_result) {
  struct Page *result = alloc_page();
  assert(result != NULL);

  pte_t *ptep = get_pte(mm->pgdir, addr, 0);

  int r;
  if ((r = swapfs_read((*ptep), result)) != 0) {
    assert(r != 0);
  }
  printf("swap_in: load disk swap entry %d with swap_page in vadr 0x%x\n",
         (*ptep) >> 8, addr);
  *ptr_result = result;
  return 0;
}

static inline void check_content_set(void) {
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num == 1);
  *(unsigned char *)0x1010 = 0x0a;
  assert(pgfault_num == 1);
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num == 2);
  *(unsigned char *)0x2010 = 0x0b;
  assert(pgfault_num == 2);
  *(unsigned char *)0x3000 = 0x0c;
  assert(pgfault_num == 3);
  *(unsigned char *)0x3010 = 0x0c;
  assert(pgfault_num == 3);
  *(unsigned char *)0x4000 = 0x0d;
  assert(pgfault_num == 4);
  *(unsigned char *)0x4010 = 0x0d;
  assert(pgfault_num == 4);
}

static inline int check_content_access(void) {
  int ret = sm->check_swap();
  return ret;
}

struct Page *check_rp[CHECK_VALID_PHY_PAGE_NUM];
pte_t *check_ptep[CHECK_VALID_PHY_PAGE_NUM];
unsigned int check_swap_addr[CHECK_VALID_VIR_PAGE_NUM];

extern free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static void check_swap(void) {
  int ret, i;

  // 建立好mm和vma
  struct mm_struct *mm = mm_create();
  assert(mm != NULL);
  extern struct mm_struct *check_mm_struct;
  assert(check_mm_struct == NULL);
  check_mm_struct = mm;
  pde_t *pgdir = mm->pgdir = boot_pgdir;
  assert(pgdir[0] == 0);
  struct vma_struct *vma = vma_create(BEING_CHECK_VALID_VADDR,
                                      CHECK_VALID_VADDR, VM_WRITE | VM_READ);
  assert(vma != NULL);
  insert_vma_struct(mm, vma);

  // 为0~4MB虚拟地址空间建立临时页表
  pte_t *temp_ptep = get_pte(mm->pgdir, BEING_CHECK_VALID_VADDR, 1);
  assert(temp_ptep != NULL);

  //建立指定数量物理页的内存环境
  for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
    check_rp[i] = alloc_page();
    assert(check_rp[i] != NULL);
    assert(!PageProperty(check_rp[i]));
  }
  list_entry_t free_list_store = free_list;
  list_init(&free_list);
  assert(list_empty(&free_list));

  unsigned int nr_free_store = nr_free;
  nr_free = 0;
  for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
    free_pages(check_rp[i], 1);
  }
  assert(nr_free == CHECK_VALID_PHY_PAGE_NUM);

  //建立初始的虚拟页到物理页的映射关系，为页替换做准备
  printf("set up init env for check_swap begin!\n");
  pgfault_num = 0;
  check_content_set();
  assert(nr_free == 0);
  for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
    check_ptep[i] = 0;
    check_ptep[i] = get_pte(pgdir, (i + 1) * 0x1000, 0);
    assert(check_ptep[i] != NULL);
    assert(pte2page(*check_ptep[i]) == check_rp[i]);
    assert((*check_ptep[i] & PTE_V));
  }
  printf("set up init env for check_swap over!\n");


  //页替换检测
  ret = check_content_access();
  assert(ret == 0);

  //恢复内存环境
  for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
    free_pages(check_rp[i], 1);
  }
  free_page(pde2page(pgdir[0]));
  pgdir[0] = 0;
  mm->pgdir = NULL;
  mm_destroy(mm);
  check_mm_struct = NULL;
  nr_free = nr_free_store;
  free_list = free_list_store;

  printf("check_swap() succeeded!\n");
}
