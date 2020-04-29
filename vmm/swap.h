#ifndef _SWAP_H_
#define _SWAP_H_

#include <types.h>
#include <mmu.h>
#include <pmm.h>
#include <vmm.h>



#define max_swap_offset (128 * 2^20 / PGSIZE)

/* *
 * swap_offset - takes a swap_entry (saved in pte), and returns
 * the corresponding offset in swap mem_map.
 * */
#define swap_offset(entry) ({                                       \
               size_t __offset = (entry >> 8);                        \
               if (!(__offset > 0 && __offset < max_swap_offset)) {    \
                    panic("invalid swap_entry_t = %08x.\n", entry);    \
               }                                                    \
               __offset;                                            \
          })

struct swap_manager  
{  
    const char *name;  
    int (*init) (void);  //初始化页替换管理器
    int (*init_mm) (struct mm_struct *mm); //初始化mm中页替换管理器的数据
    int (*tick_event) (struct mm_struct *mm); //当时钟中断发生时被调用 
    int (*map_swappable) (struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in);   //将物理页设为可替换
    int (*set_unswappable) (struct mm_struct *mm, uintptr_t addr); //将物理页设为不可替换 
    int (*swap_out_victim) (struct mm_struct *mm, struct Page *ptr_page, int in_tick);  //选择要换出的页
    int (*check_swap)(void);   //检查页替换算法
};


extern volatile int swap_init_ok;
int swap_init(void);
int swap_init_mm(struct mm_struct *mm);
int swap_tick_event(struct mm_struct *mm);
int swap_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in);
int swap_set_unswappable(struct mm_struct *mm, uintptr_t addr);
int swap_out(struct mm_struct *mm, int n, int in_tick);
int swap_in(struct mm_struct *mm, uintptr_t addr, struct Page **ptr_result);


#endif /* _SWAP_H_ */
