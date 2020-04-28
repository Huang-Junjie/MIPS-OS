#include <elf.h>
#include <error.h>
#include <pmm.h>
#include <printf.h>
#include <proc.h>
#include <string.h>
#include <vmm.h>
#include <sysnum.h>

static struct proc_struct *procs = NULL;  // 所有的进程控制块
struct proc_struct *current = NULL;
struct proc_struct *idleproc = NULL;
static struct proc_struct *initproc = NULL;

static list_entry_t proc_free_list;  // 未分配的进程控制块链表
static int nr_process = 0;

void kernel_thread_entry(void);
void forkret();
void switch_to(struct context *from, struct context *to);
void set_asid(uint32_t asid);

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

static struct proc_struct *alloc_proc() {
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
  list_init(&(proc->run_link));
  return proc;
}

void proc_run(struct proc_struct *proc) {
  if (proc != current) {
    struct proc_struct *prev = current, *next = proc;
    current = proc;
    set_asid(GET_PROC_ASID(current->pid));
    switch_to(&(prev->context), &(next->context));
  }
}

static int copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
  struct mm_struct *mm, *oldmm = current->mm;
  //当前是内核线程
  if (oldmm == NULL) {
    return 0;
  }
  //共享mm
  if (clone_flags & CLONE_VM) {
    mm = oldmm;
  } else {
    mm = mm_create();
    //建立页目录
    struct Page *page = alloc_page();
    pde_t *pgdir = page2kva(page);
    pgdir[PDX(VPT)] = PADDR(pgdir) | PTE_V;
    mm->pgdir = pgdir;
    //复制mm
    dup_mmap(mm, oldmm);
  }
  mm_count_inc(mm);
  proc->mm = mm;
  proc->pgdir = mm->pgdir;
  return 0;
}

int do_fork(uint32_t clone_flags, struct trapframe *tf) {
  if (nr_process >= MAX_PROCESS) {
    return -E_NO_FREE_PROC;
  }
  //分配进程控制块
  struct proc_struct *proc = alloc_proc();

  //建立内核栈
  struct Page *page = alloc_pages(KSTACKPAGE);
  proc->kstack = (uintptr_t)page2kva(page);

  //复制mm
  copy_mm(clone_flags, proc);

  //设置tf和context
  proc->tf = (struct trapframe *)(proc->kstack + KSTACKSIZE) - 1;
  *(proc->tf) = *tf;
  proc->tf->regs[2] = 0;  // v0=0
  if (proc->tf->regs[29] == 0) {
    proc->tf->regs[29] = proc->kstack + KSTACKSIZE;
  }
  proc->context.regs[31] = (uintptr_t)forkret;
  proc->context.regs[29] = (uintptr_t)(proc->tf);

  //设置父子兄弟进程关系
  proc->parent = current;
  if ((proc->optr = proc->parent->cptr) != NULL) {
    proc->optr->yptr = proc;
  }
  proc->parent->cptr = proc;
  nr_process++;

  //将新进程加入就绪队列
  wakeup_proc(proc);
  //返回新进程pid
  return proc->pid;
}

int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags) {
  struct trapframe tf;
  memset(&tf, 0, sizeof(struct trapframe));
  tf.regs[8] = (uint32_t)fn;
  tf.regs[9] = (uint32_t)arg;
  tf.cp0_epc = (uint32_t)kernel_thread_entry;
  tf.cp0_status = 0x1001;
  return do_fork(clone_flags | CLONE_VM, &tf);
}

void cpu_idle(void) {
  while (1) {
    if (current->need_resched) {
      schedule();
    }
  }
}

int do_exit(int error_code) {
  if (current == idleproc) {
    panic("idleproc exit.\n");
  }
  if (current == initproc) {
    panic("initproc exit.\n");
  }
  //回收mm的资源
  struct mm_struct *mm = current->mm;
  if (mm != NULL) {
    if (mm_count_dec(mm) == 0) {
      exit_mmap(mm);
      free_page(kva2page(mm->pgdir));
      mm_destroy(mm);
    }
    current->mm = NULL;
  }
  current->state = PROC_ZOMBIE;
  current->exit_code = error_code;

  struct proc_struct *proc;
  //唤醒等待子进程的父进程
  proc = current->parent;
  if (proc->wait_state == WT_CHILD) {
    wakeup_proc(proc);
  }

  //将当前进程的子进程全部作为initproc的子进程
  while (current->cptr != NULL) {
    //取出当前进程的最新子进程
    proc = current->cptr;
    proc->yptr = NULL;
    current->cptr = proc->optr;

    //将该子进程的父进程设置为initproc
    if ((proc->optr = initproc->cptr) != NULL) {
      initproc->cptr->yptr = proc;
    }
    proc->parent = initproc;
    initproc->cptr = proc;

    //若该子进程已经退出,则唤醒initproc将其回收
    if (proc->state == PROC_ZOMBIE) {
      if (initproc->wait_state == WT_CHILD) {
        wakeup_proc(initproc);
      }
    }
  }

  schedule();
  panic("do_exit will not return! %d.\n", current->pid);
}

int do_wait(int pid, int *code_store) {
  struct proc_struct *proc;
  bool haskid, found;
  while (1) {
    haskid = found = 0;
    if (pid != 0) {
      proc = find_proc(pid);
      if (proc != NULL && proc->parent == current) {
        haskid = 1;
        if (proc->state == PROC_ZOMBIE) {
          found = 1;
        }
      }
    } else {
      proc = current->cptr;
      for (; proc != NULL; proc = proc->optr) {
        haskid = 1;
        if (proc->state == PROC_ZOMBIE) {
          found = 1;
          break;
        }
      }
    }
    //等待的子进程已经退出，回收其资源
    if (found) {
      if (proc == idleproc || proc == initproc) {
        panic("wait idleproc or initproc.\n");
      }
      if (code_store != NULL) {
        *code_store = proc->exit_code;
      }
      //回收进程控制块
      list_add_before(&proc_free_list, &proc->free_list_link);
      //设置父进程和兄弟进程关系
      if (proc->optr != NULL) {
        proc->optr->yptr = proc->yptr;
      }
      if (proc->yptr != NULL) {
        proc->yptr->optr = proc->optr;
      } else {
        proc->parent->cptr = proc->optr;
      }
      nr_process--;
      //回收内核栈
      free_pages(kva2page((void *)(proc->kstack)), KSTACKPAGE);
      return 0;
    }
    //如果等待的子进程未退出，则进入阻塞态，等待子进程唤醒
    if (haskid) {
      current->state = PROC_SLEEPING;
      current->wait_state = WT_CHILD;
      schedule();
    } else {
      //没有找到等待的子进程，返回错误码
      return -E_BAD_PROC;
    }
  }
}

static int load_icode(unsigned char *binary, size_t size) {
  assert(current->mm == NULL);
  struct mm_struct *mm;
  struct Page *page;
  uint32_t vm_flags, perm;

  /* 为当前进程创建mm,建立页目录 */
  mm = mm_create();
  page = alloc_page();
  pde_t *pgdir = page2kva(page);
  pgdir[PDX(VPT)] = PADDR(pgdir) | PTE_V;
  mm->pgdir = pgdir;
  mm_count_inc(mm);
  current->mm = mm;
  current->pgdir = mm->pgdir;

  /* 解析elf程序 */
  // elf header
  Elf32_Ehdr *elf = (Elf32_Ehdr *)binary;
  //判断是否为elf格式
  if (size < 4 || elf->e_ident[0] != ELFMAG0 || elf->e_ident[1] != ELFMAG1 ||
      elf->e_ident[2] != ELFMAG2 || elf->e_ident[3] != ELFMAG3) {
    return -1;
  }
  //遍历每个程序段
  // program header table
  Elf32_Phdr *ph = (Elf32_Phdr *)(binary + elf->e_phoff);
  size_t ph_entry_count = elf->e_phnum;
  while (ph_entry_count--) {
    //判断是否为可加载的段
    if (ph->p_type != PT_LOAD) {
      continue;
    }

    //建立vma
    vm_flags = 0, perm = 0;
    if (ph->p_flags & PF_R) vm_flags |= VM_READ;
    if (ph->p_flags & PF_X) vm_flags |= VM_EXEC;
    if (ph->p_flags & PF_W) {
      vm_flags |= VM_WRITE;
      perm |= PTE_D;
    }
    mm_map(mm, ph->p_vaddr, ph->p_memsz, vm_flags);

    //分配物理内存，复制程序段的内容到内存中
    unsigned char *from = binary + ph->p_offset;
    size_t off, size;
    uintptr_t start = ph->p_vaddr, end, va = ROUNDDOWN(start, PGSIZE);
    end = ph->p_vaddr + ph->p_filesz;
    //复制程序内容
    while (start < end) {
      page = pgdir_alloc_page(mm->pgdir, va, perm);
      off = start - va;
      size = PGSIZE - off;
      va += PGSIZE;
      if (end < va) {
        size -= va - end;
      }
      memcpy(page2kva(page) + off, from, size);
      start += size;
      from += size;
    }

    // bss段内存内容设为0
    if (ph->p_memsz > ph->p_filesz) {
      end = ph->p_vaddr + ph->p_memsz;
      if (start < va) {
        //复制程序内容时分配的最后一个页的剩余内存
        off = start + PGSIZE - va;
        size = PGSIZE - off;
        if (end < va) {
          size -= va - end;
        }
        memset(page2kva(page) + off, 0, size);
        start += size;
        assert((end < va && start == end) || (end >= va && start == va));
      }
      while (start < end) {
        page = pgdir_alloc_page(mm->pgdir, va, perm);
        off = start - va;
        size = PGSIZE - off;
        va += PGSIZE;
        if (end < va) {
          size -= va - end;
        }
        memset(page2kva(page) + off, 0, size);
        start += size;
      }
    }
  }

  //建立用户栈内存空间
  vm_flags = VM_READ | VM_WRITE;
  mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags);
  // pgdir_alloc_page(mm->pgdir, USTACKTOP - PGSIZE, PTE_D) != NULL;

  //设置当前进程中断帧,使其异常返回后,回到用户态
  memset(current->tf, 0, sizeof(struct trapframe));
  current->tf->cp0_status = 0x1013;
  current->tf->regs[29] = USTACKTOP;
  current->tf->cp0_epc = elf->e_entry;

  return 0;
}

int do_execve(unsigned char *binary, size_t size) {
  //回收原来mm的资源
  struct mm_struct *mm = current->mm;
  if (mm != NULL) {
    if (mm_count_dec(mm) == 0) {
      exit_mmap(mm);
      free_page(kva2page(mm->pgdir));
      mm_destroy(mm);
    }
    current->mm = NULL;
  }

  int ret;
  if ((ret = load_icode(binary, size)) != 0) {
    do_exit(ret);
  }
  return 0;
}

extern int syscall(int num, ...);
static int kernel_execve(unsigned char *binary, size_t size) {
  return syscall(SYS_exec, binary, size);
}

static int user_main(void *arg) {
  extern unsigned char USERSTART[], USERSIZE[];
  kernel_execve(USERSTART, (size_t)USERSIZE);
}

static int init_main(void *arg) {
  int pid = kernel_thread(user_main, NULL, 0);
  if (pid <= 0) {
    panic("create user_main failed.\n");
  }
  // extern void check_sync(void);
  // check_sync();  // check philosopher sync problem

  while (do_wait(0, NULL) == 0) {
    schedule();
  }

  printf("all user-mode processes have quit.\n");
  assert(initproc->cptr == NULL && initproc->yptr == NULL &&
         initproc->optr == NULL);
  assert(nr_process == 2);
  return 0;
}

void proc_init(void) {
  //初始化进程控制块和进程控制块链表
  int i;
  procs = kmalloc(sizeof(struct proc_struct) * MAX_PROCESS);
  list_init(&proc_free_list);
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
  assert(idleproc != NULL && PROCX(idleproc->pid) == 0);
  assert(initproc != NULL && PROCX(initproc->pid) == 1);
}
