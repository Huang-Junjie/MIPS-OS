#include <error.h>
#include <pmm.h>
#include <printf.h>
#include <proc.h>
#include <string.h>

static struct proc_struct *procs = NULL;  // 所有的进程控制块
struct proc_struct *current = NULL;
struct proc_struct *idleproc = NULL;
static struct proc_struct *initproc = NULL;

static list_entry_t proc_free_list;  // 未分配的进程控制块链表
list_entry_t proc_list;              // 已分配的进程控制块链表
static int nr_process = 0;

// void
// proc_run(struct proc_struct *proc) {
//     if (proc != current) {
//         // bool intr_flag;
//         struct proc_struct *prev = current, *next = proc;
//         // local_intr_save(intr_flag);
//         // {
//         current = proc;
//         load_kernel_sp(next->kstack + KSTACKSIZE);
//         load_pgdir(next->pgdir);
//         switch_to(&(prev->context), &(next->context));
//         // }
//         // local_intr_restore(intr_flag);
//     }
// }



static uint32_t make_pid(struct proc_struct *proc) {
  static uint32_t next_pid = 0;
  uint32_t idx = proc - procs;
  return (next_pid++ << 8) | idx;
}

struct proc_struct *find_proc(uint32_t pid) {
  struct proc_struct *proc;
  proc = procs + PROCX(pid);
  if (proc->state == PROC_FREE || proc->pid != pid) {
    return NULL;
  }
  return proc;
}

static struct proc_struct *proc_alloc() {
  if (list_empty(&proc_free_list)) {
    return NULL;
  }
  //分配进程控制块
  struct proc_struct *proc;
  proc = le2proc(list_next(&proc_free_list), free_list_link);
  list_del(&proc->free_list_link);
  //赋初始值
  proc->pid = make_pid(proc);
  proc->state = PROC_UNINIT;
  proc->runs = 0;
  proc->need_resched = 0;
  proc->time_slice = 0;
  proc->wait_state = 0;
  proc->exit_code = 0;
  proc->kstack = 0;
  proc->pgdir = boot_pgdir;
  proc->mm = NULL;
  memset(&(proc->context), 0, sizeof(struct context));
  proc->tf = NULL;
  proc->parent = NULL;
  proc->cptr = proc->yptr = proc->optr = NULL;

  return proc;
}



int
kernel_thread(int (*fn)(void *), void *arg) {
    struct trapframe tf;
    memset(&tf, 0, sizeof(struct trapframe));
    tf.tf_cs = KERNEL_CS;
    tf.tf_ds = tf.tf_es = tf.tf_ss = KERNEL_DS;
    tf.tf_regs.reg_ebx = (uint32_t)fn;
    tf.tf_regs.reg_edx = (uint32_t)arg;
    tf.cp0_epc = (uint32_t)kernel_thread_entry;
    return do_fork(clone_flags | CLONE_VM, 0, &tf);
}


void proc_init(void) {
  //初始化进程控制块和进程控制块链表
  int i;
  procs = kmalloc(sizeof(struct proc_struct) * MAX_PROCESS);
  list_init(&proc_free_list);
  list_init(&proc_list);
  for (i = 0; i < MAX_PROCESS; i++) {
    (procs[i]).state = PROC_FREE;
    list_add_before(&proc_free_list, &(procs[i].free_list_link));
  }

  //将当前上下文打造为idleproc
  if ((idleproc = alloc_proc()) == NULL) {
    panic("alloc idleproc failed.\n");
  }
  idleproc->state = PROC_RUNNABLE;
  idleproc->kstack = (uintptr_t)0x80400000;
  idleproc->need_resched = 1;
  nr_process++;
  current = idleproc;

  //创建initproc内核线程
  int pid = kernel_thread(init_main, NULL, 0);
  if (pid <= 0) {
    panic("create init_main failed.\n");
  }
  initproc = find_proc(pid);
  assert(idleproc != NULL && idleproc->pid == 0);
  assert(initproc != NULL && initproc->pid == 1);
}