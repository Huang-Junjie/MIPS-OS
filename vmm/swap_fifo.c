#include <list.h>
#include <printf.h>
#include <string.h>
#include <swap.h>
#include <swap_fifo.h>
#include <types.h>



list_entry_t pra_list_head;


static int fifo_init_mm(struct mm_struct *mm) {
  list_init(&pra_list_head);
  mm->sm_priv = &pra_list_head;
  return 0;
}


static int fifo_map_swappable(struct mm_struct *mm, uintptr_t addr,
                               struct Page *page, int swap_in) {
  list_entry_t *head = (list_entry_t *)mm->sm_priv;
  list_entry_t *entry = &(page->pra_page_link);

  assert(entry != NULL && head != NULL);
  list_add_after(head, entry);
  return 0;
}


static int fifo_swap_out_victim(struct mm_struct *mm, struct Page **ptr_page,
                                 int in_tick) {
  list_entry_t *head = (list_entry_t *)mm->sm_priv;
  assert(head != NULL);
  assert(in_tick == 0);
  list_entry_t *le = head->prev;
  assert(head != le);
  struct Page *p = le2page(le, pra_page_link);
  list_del(le);
  assert(p != NULL);
  *ptr_page = p;
  return 0;
}

static int fifo_check_swap(void) {
  printf("write Virt Page c in fifo_check_swap\n");
  *(unsigned char *)0x3000 = 0x0c;
  assert(pgfault_num == 4);
  printf("write Virt Page a in fifo_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num == 4);
  printf("write Virt Page d in fifo_check_swap\n");
  *(unsigned char *)0x4000 = 0x0d;
  assert(pgfault_num == 4);
  printf("write Virt Page b in fifo_check_swap\n");
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num == 4);
  printf("write Virt Page e in fifo_check_swap\n");
  *(unsigned char *)0x5000 = 0x0e;
  assert(pgfault_num == 5);
  printf("write Virt Page b in fifo_check_swap\n");
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num == 5);
  printf("write Virt Page a in fifo_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num == 6);
  printf("write Virt Page b in fifo_check_swap\n");
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num == 7);
  printf("write Virt Page c in fifo_check_swap\n");
  *(unsigned char *)0x3000 = 0x0c;
  assert(pgfault_num == 8);
  printf("write Virt Page d in fifo_check_swap\n");
  *(unsigned char *)0x4000 = 0x0d;
  assert(pgfault_num == 9);
  printf("write Virt Page e in fifo_check_swap\n");
  *(unsigned char *)0x5000 = 0x0e;
  assert(pgfault_num == 10);
  printf("write Virt Page a in fifo_check_swap\n");
  assert(*(unsigned char *)0x1000 == 0x0a);
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num == 11);
  return 0;
}

static int fifo_init(void) { return 0; }

static int fifo_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
  return 0;
}

static int fifo_tick_event(struct mm_struct *mm) { return 0; }

struct swap_manager fifo_swap_manager = {
    .name = "fifo swap manager",
    .init = fifo_init,
    .init_mm = fifo_init_mm,
    .tick_event = fifo_tick_event,
    .map_swappable = fifo_map_swappable,
    .set_unswappable = fifo_set_unswappable,
    .swap_out_victim = fifo_swap_out_victim,
    .check_swap = fifo_check_swap,
};
