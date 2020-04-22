#ifndef _MMU_H_
#define _MMU_H_

/**
 *  虚拟地址 va 的结构:
 *
 *   +--------10------+-------10-------+---------12----------+
 *   | Page Directory |   Page Table   | Offset within Page  |
 *   |      Index     |     Index      |                     |
 *   +----------------+----------------+---------------------+
 *    \--- PDX(va) --/ \--- PTX(va) --/ \---- PGOFF(va) ----/
 *    \----------- PPN(va) -----------/
 *
 */

#define NPDEENTRY       1024                    // page directory entries per page directory
#define NPTEENTRY       1024                    // page table entries per page table

#define PGSIZE      4096                // 一个页映射的字节数
#define PGSHIFT     12                  // log2(PGSIZE)
#define PDSIZE      (PGSIZE * NPTEENTRY)     // 一个页目录项映射的字节数
#define PDSHIFT     22                  // log2(PTSIZE)

#define PDX(va)     ((((unsigned int)(va)) >> PDSHIFT) & 0x3FF)      // page directory index
#define PTX(va)     ((((unsigned int)(va)) >> PGSHIFT) & 0x3FF)      // page table index
#define PTE_ADDR(pte)   ((unsigned int)(pte) & ~0xFFF)               // 页表项中的基地址
#define PDE_ADDR(pde)   PTE_ADDR(pde)                                // 页目录项中的基地址


#define PPN(pa)     (((unsigned int)(pa)) >> PGSHIFT)               // 地址对应的页/帧号
#define VPN(va)     PPN(va)
#define PGOFF(va)   (((unsigned int)(va)) & 0xFFF)                  // 页内偏移


/* 页表项/页目录项 标志位 */
#define PTE_G       0x040     // 全局位
#define PTE_V       0x080     // 有效位
#define PTE_D       0x100     // 脏位（写使能位）

typedef unsigned int  pde_t;
typedef unsigned int  pte_t;
typedef pte_t swap_entry_t;

/**
 *                                 内存布局图
 *
 *     4G ----------->  +----------------------------+------------0xffff ffff
 *                      |                            |  kseg3
 *                      +----------------------------+------------0xe000 0000
 *                      |                            |  kseg2
 *                      +----------------------------+------------0xc000 0000-------
 *                      |                            |                          /|\
 *                      +----------------------------+------------0xb600 0000    |
 *                      |            MMIO            |                          kseg1
 *                      +----------------------------+------------0xb000 0000    |
 *                      |                            |                          \|/
 *                      +----------------------------+------------0xa000 0000-------
 *                      |      Invalid memory        |    /|\                      
 *                      +----------------------------+-----|------Physics Memory Max                          
 *                      |                            |     |                       
 *                      +----------------------------+-----|------0x8040 0000--------end
 *                      |       Kernel Stack         |     |                     
 *                      +----------------------------+   kseg0                      
 *                      |       Kernel Data          |     |                      
 *                      +----------------------------+     |                      
 *                      |       Kernel Text          |     |                      
 *                      +----------------------------+-----|------0x8001 0000    
 *                      |   Interrupts & Exception   |    \|/                     
 *                      +----------------------------+------------0x8000 0000-------
 *                      |            VPT             |                          /|\
 *     VPT,USTACKTOP--> +----------------------------+------------0x7fc0 0000    |
 *                      |        USER STACK          |                           |
 *                      +----------------------------+------------0x7fbf e000    |
 *                      |                            |                           |
 *                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~|                           |
 *                      .                            .                           |
 *                      .                            .                         kuseg
 *                      .                            .                           |
 *                      |~~~~~~~~~~~~~~~~~~~~~~~~~~~~|                           |
 *                      |                            |                           |
 *       UTEXT   -----> +----------------------------+                           |
 *                      |                            |                          \|/
 *     0 ------------>  +----------------------------+ -----------------------------
 *
 */


#define ULIM 0x80000000
#define VPT  0x7fc00000

#define KSTACKPAGE          2                           // # of pages in kernel stack
#define KSTACKSIZE          (KSTACKPAGE * PGSIZE)       // sizeof kernel stack

#ifndef __ASSEMBLER__


#define PADDR(kva)  ({                                          \
            unsigned int a = (unsigned int) (kva);              \
            if (a < ULIM)  {                                    \
                panic("PADDR called with invalid kva %x", a);   \
            }                                                   \
            a - ULIM;                                           \
        })


#define KADDR(pa)   ({                                                          \
            unsigned int ppn = PPN(pa);                                         \
            if (ppn >= npage) {                                                 \
                panic("KADDR called with invalid pa %x", (unsigned int)pa);     \
            }                                                                   \
            (unsigned int)(pa) + ULIM;                                          \
        })


#define assert(x)                                   \
    do {                                            \
        if (!(x)) {                                 \
            panic("assertion failed: %s", #x);      \
        }                                           \
    } while (0)


#endif /* __ASSEMBLER__ */
#endif /* _MMU_H_ */
