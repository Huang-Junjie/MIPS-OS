#ifndef _VMM_H_
#define _VMM_H_

#include <list.h>
#include <trap.h>
#include <types.h>


struct mm_struct;

struct vma_struct {
    struct mm_struct *vm_mm; //同一进程的vma指向该进程的mm
    uintptr_t vm_start; //vma的起始地址
    uintptr_t vm_end;  // vma的结束地址
    uint32_t vm_flags; // 此vma表示的虚拟内存空间的属性
    list_entry_t list_link;  // 链接其他vma
};

#define le2vma(le, member) to_struct((le), struct vma_struct, member)

#define VM_READ 0x00000001
#define VM_WRITE 0x00000002
#define VM_EXEC 0x00000004

struct mm_struct {
    list_entry_t mmap_list;// 链接vma
    struct vma_struct *mmap_cache;// 当前访问的vma，用于加速vma查询 
    pde_t *pgdir; //  vma的页目录
    int map_count; // vma的数量
    int mm_count;   // mm共享计数
    void *sm_priv; // 用于页替换管理器的私有数据
};

struct vma_struct *find_vma(struct mm_struct *mm, uintptr_t addr);
struct vma_struct *vma_create(uintptr_t vm_start, uintptr_t vm_end,
                              uint32_t vm_flags);
void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma);

struct mm_struct *mm_create(void);
void mm_destroy(struct mm_struct *mm);

void vmm_init(void);
int pgfault_handler(struct trapframe *tf);


int mm_map(struct mm_struct *mm, uintptr_t addr, size_t len, uint32_t vm_flags);
int dup_mmap(struct mm_struct *to, struct mm_struct *from);
void exit_mmap(struct mm_struct *mm);


extern volatile unsigned int pgfault_num;
extern struct mm_struct *check_mm_struct;


static inline int
mm_count(struct mm_struct *mm) {
    return mm->mm_count;
}

static inline void
set_mm_count(struct mm_struct *mm, int val) {
    mm->mm_count = val;
}

static inline int
mm_count_inc(struct mm_struct *mm) {
    mm->mm_count += 1;
    return mm->mm_count;
}

static inline int
mm_count_dec(struct mm_struct *mm) {
    mm->mm_count -= 1;
    return mm->mm_count;
}
#endif /* _VMM_H_ */
