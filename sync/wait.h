#ifndef _WAIT_H_
#define _WAIT_H_

#include <list.h>
#include <proc.h>
#include <types.h>

typedef struct {
    list_entry_t wait_head;
} wait_queue_t;


typedef struct {
    struct proc_struct *proc;
    uint32_t wakeup_flags;
    wait_queue_t *wait_queue;
    list_entry_t wait_link;
} wait_t;

#define le2wait(le, member)         \
    to_struct((le), wait_t, member)


void wait_init(wait_t *wait, struct proc_struct *proc);
void wait_queue_init(wait_queue_t *queue);
void wait_queue_add(wait_queue_t *queue, wait_t *wait);
void wait_queue_del(wait_queue_t *queue, wait_t *wait);
wait_t *wait_queue_first(wait_queue_t *queue);


#endif /* _WAIT_H_ */

