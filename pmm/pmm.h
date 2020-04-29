#ifndef _PMM_H_
#define _PMM_H_

#include <list.h>
#include <types.h>
#include <mmu.h>
#include <printf.h>

struct Page {
    int ref;                        // page frame's reference counter
    uint32_t flags;                 // array of flags that describe the status of the page frame
    unsigned int property;          // used in buddy system, stores the order (the X in 2^X) of the continuous memory block
    list_entry_t page_link;         // free list link
    list_entry_t pra_page_link;     // used for pra (page replace algorithm)
    uintptr_t pra_vaddr;            // used for pra (page replace algorithm)
};

/* Flags describing the status of a page frame */
#define PG_reserved                 0       // the page descriptor is reserved for kernel or unusable
#define PG_property                 1       // the member 'property' is valid

#define SetPageReserved(page)       ((page)->flags |= 0x1)
#define ClearPageReserved(page)     ((page)->flags &= ~0x1)
#define PageReserved(page)          ((page)->flags & 0x1)
#define SetPageProperty(page)       ((page)->flags |= 0x2)
#define ClearPageProperty(page)     ((page)->flags &= ~0x2)
#define PageProperty(page)          ((page)->flags & 0x2)


#define le2page(le, member)                 \
    to_struct((le), struct Page, member)

/* free_area_t - maintains a doubly linked list to record free (unused) pages */
typedef struct {
    list_entry_t free_list;         // the list header
    unsigned int nr_free;           // # of free pages in this free list
} free_area_t;


struct pmm_manager {
    const char *name;                                 // XXX_pmm_manager's name
    void (*init)(void);                               // initialize internal description&management data structure
                                                      // (free block list, number of free block) of XXX_pmm_manager
    void (*init_memmap)(struct Page *base, size_t n); // setup description&management data structcure according to
                                                      // the initial free physical memory space
    struct Page *(*alloc_pages)(size_t n);            // allocate >=n pages, depend on the allocation algorithm
    void (*free_pages)(struct Page *base, size_t n);  // free >=n pages with "base" addr of Page descriptor structures(memlayout.h)
    size_t (*nr_free_pages)(void);                    // return the number of free pages
    void (*check)(void);                              // check the correctness of XXX_pmm_manager
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

void pmm_init(void);

struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);

#define alloc_page() alloc_pages(1)
#define free_page(page) free_pages(page, 1)

pte_t *get_pte(pde_t *pgdir, uintptr_t la, bool create);
struct Page *get_page(pde_t *pgdir, uintptr_t la, pte_t **ptep_store);
void page_remove(pde_t *pgdir, uintptr_t la);
int page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm);

extern void tlb_out(uintptr_t entryhi);
void tlb_invalidate(pde_t *pgdir, uintptr_t la);
struct Page *pgdir_alloc_page(pde_t *pgdir, uintptr_t la, uint32_t perm);

void unmap_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
void exit_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end, bool share);

void print_pgdir(void);



#endif /* _PMM_H_ */
