/* Notes written by Qian Liu <qianlxc@outlook.com>
  If you find any bug, please contact with me.*/

#include <printf.h>
#include <proc.h>
#include <error.h>
#include <list.h>
#include <types.h>
#include <string.h>
// #include <mmu.h>
// #include <sched.h>

struct proc_struct *procs = NULL;        // All environments
struct proc_struct *current = NULL;
struct proc_struct *idleproc = NULL;
struct proc_struct *initproc = NULL;

static list_entry_t proc_free_list;     // Free list
list_entry_t proc_list;




// uint32_t mkprocpid(struct proc_struct *proc) {
//     static uint32_t next_pid = 0;
//     uint32_t idx = proc - procs;
//     return (++next_pid << 8) | idx;
// }

// struct proc_struct * pid2proc(uint32_t pid, struct proc_struct *proc, int checkperm) {
//     struct proc_struct *proc;

//     proc = procs + PROCX(pid);

//     if (proc->state == PROC_FREE || proc->pid != pid) {
//         return NULL;
//     }

//     if (checkperm) {
//         if (proc != current && proc->parent_pid != current->pid) {
//             return NULL;
//         }

//     }
//     return proc;
// }


// void
// proc_init(void) {
//     int i;

//     procs = kmalloc(sizeof(struct proc_struct) * MAX_PROCESS);

//     list_init(&proc_free_list);
//     list_init(&proc_list);

//     for (i = 0; i < MAX_PROCESS; i++) {
//         (procs[i]).state = PROC_FREE;
//         list_add_before(&proc_free_list, &(procs[i].free_list_link));
//     }
// }



// static struct proc_struct *
// proc_alloc(uint32_t parent_pid) {
//     if (list_empty(&proc_free_list)) {
//         return NULL;
//     }

//     struct proc_struct *proc;
//     proc = le2proc(list_next(&proc_free_list), free_list_link);


//     proc->tf = NULL;
//     memset(&(proc->context), 0, sizeof(struct context));
//     proc->pid = mkprocid(proc);
//     proc->parent_pid = parent_pid;
//     proc->mm = NULL;
//     proc->state = PROC_UNINIT;
//     proc->pgdir = boot_pgdir;
//     proc->kstack = 0;
//     proc->runs = 0;

//     list_del(&proc->free_list_link);
//     return proc;
// }


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



// // kernel_thread - create a kernel thread using "fn" function
// // NOTE: the contents of temp trapframe tf will be copied to
// //       proc->tf in do_fork-->copy_thread function
// int
// kernel_thread(int (*fn)(void *), void *arg) {
//     struct trapframe tf;
//     memset(&tf, 0, sizeof(struct trapframe));
//     tf.tf_cs = KERNEL_CS;
//     tf.tf_ds = tf.tf_es = tf.tf_ss = KERNEL_DS;
//     tf.tf_regs.reg_ebx = (uint32_t)fn;
//     tf.tf_regs.reg_edx = (uint32_t)arg;
//     tf.cp0_epc = (uint32_t)kernel_thread_entry;
//     return do_fork(clone_flags | CLONE_VM, 0, &tf);
// }
