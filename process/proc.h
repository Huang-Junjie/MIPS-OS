#ifndef _ENV_H_
#define _ENV_H_

#include <types.h>
#include <list.h>
#include <trap.h>
// #include <mmu.h>

// process's state in his life cycle
enum proc_state {
    PROC_FREE = 0,
    PROC_UNINIT,  // uninitialized
    PROC_SLEEPING,    // sleeping
    PROC_RUNNABLE,    // runnable(maybe running)
    PROC_ZOMBIE,      // almost dead, and wait parent proc to reclaim his resource
};

struct context {
    uint32_t regs[32];
    uint32_t hi;
    uint32_t lo;
    // uint32_t pc;
};

struct proc_struct {
    uint32_t pid;   //进程id
    enum proc_state state;  //进程状态
    uint32_t runs;         //进程被运行次数
    volatile bool need_resched; //进程是否需要被调度来释放CPU
    int time_slice;  //进程剩余时间片
    uint32_t wait_state;    //进程等待态的原因
    int exit_code;          //进程退出状态码
    uintptr_t kstack; //进程的内核栈
    pde_t *pgdir; //进程页目录指针
    struct mm_struct *mm; //进程虚拟内存空间管理信息
    struct context context;  //进程切换时的上下文
    struct trapframe *tf;   //当前中断的中断帧指针
    struct proc_struct *parent; //父进程
    struct proc_struct *cptr, *yptr, *optr; //子进程、弟进程、兄进程
    list_entry_t list_link;  //已分配的进程控制块链表指针
    list_entry_t free_list_link;  //未分配的进程控制块链表指针
    list_entry_t run_link;  //就绪队列的进程控制块链表指针
};



#define MAX_PROCESS                 256
#define PROCX(pid) ((pid) & 0x0ff)
#define GET_PROC_ASID(pid) PROCX(pid)
#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)

#define CLONE_VM 0x1

#define WT_CHILD 0x1   //等待子进程
#define WT_SEM   0x2    //等待信号量
#define WT_TIMER 0x3   //等待计时器

extern list_entry_t proc_list;
extern struct proc_struct *idleproc, *current;



void proc_init(void);
void proc_run(struct proc_struct *proc);
struct proc_struct *find_proc(uint32_t pid);
int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags);
int do_fork(uint32_t clone_flags, struct trapframe *tf)

// int proc_alloc(struct proc_struct **proc_p, uint32_t parent_pid);
// void proc_free(struct proc_struct *);
// void proc_create(u_char *binary, int size);
// void proc_destroy(struct proc_struct *proc);
// void proc_run(struct proc_struct *proc);

// struct proc_struct * pid2proc(uint32_t pid, struct proc_struct *proc, int checkperm);


// int kernel_thread(int (*fn)(void *), void *arg);
// void cpu_idle(void) __attribute__((noreturn));

int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf);





#define ENV_CREATE(x) \
{ \
    extern u_char _binary_user_##x##_start[];\
    extern u_char _binary_user_##x##_size[]; \
    env_create(binary_##x##_start, \
        (size_t)binary_##x##_size); \
}

#endif // !_ENV_H_



