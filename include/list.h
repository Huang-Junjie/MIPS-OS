#ifndef _LIST_H_
#define _LIST_H_
#include <types.h>


struct list_entry {
    struct list_entry *prev, *next;
};

typedef struct list_entry list_entry_t;

static inline void list_init(list_entry_t *elm) __attribute__((always_inline));
static inline void list_add_before(list_entry_t *listelm, list_entry_t *elm) __attribute__((always_inline));
static inline void list_add_after(list_entry_t *listelm, list_entry_t *elm) __attribute__((always_inline));
static inline void list_del(list_entry_t *listelm) __attribute__((always_inline));
static inline bool list_empty(list_entry_t *list) __attribute__((always_inline));
static inline list_entry_t *list_next(list_entry_t *listelm) __attribute__((always_inline));
static inline list_entry_t *list_prev(list_entry_t *listelm) __attribute__((always_inline));



static inline void
list_init(list_entry_t *elm) {
    elm->prev = elm->next = elm;
}

static inline void
list_add_before(list_entry_t *listelm, list_entry_t *elm) {
    elm->prev = listelm->prev;
    listelm->prev->next = elm;
    elm->next = listelm;
    listelm->prev = elm;
}

static inline void
list_add_after(list_entry_t *listelm, list_entry_t *elm) {
    elm->next = listelm->next;
    listelm->next->prev = elm;
    elm->prev = listelm;
    listelm->next = elm;
}

static inline void
list_del(list_entry_t *listelm) {
    listelm->prev->next = listelm->next;
    listelm->next->prev = listelm->prev;
}

static inline bool
list_empty(list_entry_t *listhead)  {
    return listhead->next == listhead;
}

static inline list_entry_t *
list_next(list_entry_t *listelm) {
    return listelm->next;
}

static inline list_entry_t *
list_prev(list_entry_t *listelm) {
    return listelm->prev;
}


#endif /* _LIST_H_ */
