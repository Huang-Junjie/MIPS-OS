#ifndef _VMM_H_
#define _VMM_H_

#include <list.h>
#include <trap.h>
#include <types.h>

// pre define
struct mm_struct;

// the virtual continuous memory area(vma)
struct vma_struct {
  struct mm_struct *vm_mm;  // the set of vma using the same PDT
  uintptr_t vm_start;       //    start addr of vma
  uintptr_t vm_end;         // end addr of vma
  uint32_t vm_flags;        // flags of vma
  list_entry_t list_link;  // linear list link which sorted by start addr of vma
};

#define le2vma(le, member) to_struct((le), struct vma_struct, member)

#define VM_READ 0x00000001
#define VM_WRITE 0x00000002
#define VM_EXEC 0x00000004

// the control struct for a set of vma using the same PDT
struct mm_struct {
  list_entry_t mmap_list;  // linear list link which sorted by start addr of vma
  struct vma_struct
      *mmap_cache;  // current accessed vma, used for speed purpose
  pde_t *pgdir;     // the PDT of these vma
  int map_count;    // the count of these vma
  int mm_count;                  // the number ofprocess which shared the mm
  void *sm_priv;    // the private data for swap manager
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
