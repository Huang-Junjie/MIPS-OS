#ifndef _PMM_H_
#define _PMM_H_

#include <list.h>
#include <types.h>
#include <mmu.h>
#include <printf.h>

struct Page {
    int ref;                        //物理页引用次数
    uint32_t flags;                 //物理页状态
    unsigned int property;          //从此页开始的连续空闲物理页个数
    list_entry_t page_link;         //链接空闲物理页的链表节点
    list_entry_t pra_page_link;     // used for pra (page replace algorithm)
    uintptr_t pra_vaddr;            // used for pra (page replace algorithm)
};

typedef struct {
    list_entry_t free_list;         // 空闲块链表头节点
    unsigned int nr_free;           // 空闲块链表空闲页的数量
} free_area_t;


/* Page flag bit */
#define PG_reserved                 0       // 物理页是否保留
#define PG_property                 1       // 物理页是否空闲
#define SetPageReserved(page)       ((page)->flags |= 0x1)
#define ClearPageReserved(page)     ((page)->flags &= ~0x1)
#define PageReserved(page)          ((page)->flags & 0x1)
#define SetPageProperty(page)       ((page)->flags |= 0x2)
#define ClearPageProperty(page)     ((page)->flags &= ~0x2)
#define PageProperty(page)          ((page)->flags & 0x2)


#define le2page(le, member)                 \
    to_struct((le), struct Page, member)



struct pmm_manager {
    const char *name; //物理内存页管理器的名字
    void (*init)(void); //初始化内存管理器数据结构
    void (*init_memmap)(struct Page *base, size_t n); //根据空闲内存空间建立数据结构
    struct Page *(*alloc_pages)(size_t n); //分配n个连续物理内存页
    void (*free_pages)(struct Page *base, size_t n); //释放n个连续物理内存页
    size_t (*nr_free_pages)(void); //返回当前剩余的空闲页数
    void (*check)(void); //检测分配/释放实现是否正确
};



extern struct Page *pages;
extern size_t npage;

static inline size_t
page2ppn(struct Page *page) {
    return page - pages;
}

static inline size_t
page2pa(struct Page *page) {
    return page2ppn(page) << PGSHIFT;
}

static inline struct Page *
pa2page(size_t pa) {
    if (PPN(pa) >= npage) {
        panic("pa2page called with invalid pa: %x", pa);
    }
    return &pages[PPN(pa)];
}

static inline size_t
page2kva(struct Page *page) {
    return KADDR(page2pa(page));
}

static inline struct Page *
kva2page(void *kva) {
    return pa2page(PADDR(kva));
}


static inline struct Page *
pte2page(pte_t pte) {
    if (!(pte & PTE_V)) {
        panic("pte2page called with invalid pte");
    }
    return pa2page(PTE_ADDR(pte));
}

static inline struct Page *
pde2page(pde_t pde) {
    return pa2page(PDE_ADDR(pde));
}

static inline int
page_ref(struct Page *page) {
    return page->ref;
}

static inline void
set_page_ref(struct Page *page, int val) {
    page->ref = val;
}

static inline int
page_ref_inc(struct Page *page) {
    page->ref += 1;
    return page->ref;
}

static inline int
page_ref_dec(struct Page *page) {
    page->ref -= 1;
    return page->ref;
}



extern const struct pmm_manager *pmm_manager;
extern pde_t *boot_pgdir;
extern void tlb_out(uintptr_t entryhi);

void pmm_init(void);
struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);

#define alloc_page() alloc_pages(1)
#define free_page(page) free_pages(page, 1)


pte_t *get_pte(pde_t *pgdir, uintptr_t va, bool create);
void page_remove(pde_t *pgdir, uintptr_t va);
int page_insert(pde_t *pgdir, struct Page *page, uintptr_t va, uint32_t perm);
void tlb_invalidate(pde_t *pgdir, uintptr_t va);
struct Page *pgdir_alloc_page(pde_t *pgdir, uintptr_t va, uint32_t perm);

void unmap_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
void exit_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end, bool share);

void print_pgdir(void);



#endif /* _PMM_H_ */
