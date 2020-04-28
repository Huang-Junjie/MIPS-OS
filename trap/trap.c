#include <pmm.h>
#include <printf.h>
#include <proc.h>
#include <syscall_number.h>
#include <trap.h>
#include <vmm.h>

static void print_ticks() {
  static int ticks = 0;
  ticks++;
  if (ticks % 100 == 0) {
    printf("%d ticks\n", ticks);
  }
}

static void handle_int(struct trapframe *tf) {
  /* 判断是否是时钟中断 */
  if (tf->cp0_cause & tf->cp0_status & 0x1000) {
    *(char *)0xb5000110 = 1;  //响应时钟中断
    print_ticks();
  }
  // delete
  else {
    print_ticks("else int");
  }
}

static void handle_mod(struct trapframe *tf) { pgfault_handler(tf); }

extern void tlb_set(uintptr_t badvaddr, pte_t pte);
extern struct proc_struct *current;
static void handle_tlb(struct trapframe *tf) {
  pte_t *ptep;
  pde_t *cur_pgdir;
  cur_pgdir = current == NULL ? boot_pgdir : current->pgdir;
  while (1) {
    ptep = get_pte(cur_pgdir, tf->cp0_badvaddr, 0);
    if (ptep != NULL && *ptep & PTE_V) {
      tlb_set(tf->cp0_badvaddr, (*ptep) >> 6);
      return;
    } else {
      pgfault_handler(tf);
    }
  }
}

static void syscall(struct trapframe *tf) {
  int num = tf->regs[16];
  int ret = 0;
  switch (num) {
    case SYS_exit:
      ret = do_exit(tf->regs[17]);
      break;
    case SYS_fork:
      ret = do_fork(0, tf);
      break;
    case SYS_wait:
      ret = do_wait(tf->regs[17], tf->regs[18]);
      break;
    case SYS_sleep:
      //
      break;
    case SYS_putc:
      printf("%c", tf->regs[17]);
  }
  tf->regs[2] = ret;  //设置返回值
  tf->cp0_epc += 4;   //返回syscall下条指令
}

static void trap_dispatch(struct trapframe *tf) {
  switch ((tf->cp0_cause & 0x7c) >> 2) {
    case 0:  // Int
      handle_int(tf);
      break;
    case 1:  // TLB mod
      handle_mod(tf);
      break;
    case 2:  // TLBL
    case 3:  // TLBS
      handle_tlb(tf);
      break;
    case 8:  // Syscall
      syscall(tf);
  }
}

// static void print_tf(struct trapframe *tf)  {
//     printf("Enter print_tf\n");
//     printf("CP0_STATUS value:  \t0x%08x\n", tf->cp0_status);
//     printf("CP0_CAUSE value:   \t0x%08x\n", tf->cp0_cause);
//     printf("CP0_EPC value:     \t0x%08x\n", tf->cp0_epc);
//     printf("CP0_BADVADDR value:\t0x%08x\n", tf->cp0_badvaddr);
// }

void trap(struct trapframe *tf) {
  if (current == NULL) {
    trap_dispatch(tf);
  } else {
    // 将当前进程的tf指向当前tf
    struct trapframe *old_tf = current->tf;
    current->tf = tf;
    trap_dispatch(tf);
    current->tf = old_tf;
    if (current->need_resched) {
      schedule();
    }
  }
}
