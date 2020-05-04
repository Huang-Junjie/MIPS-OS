#ifndef _SEM_H_
#define _SEM_H_

#include <types.h>
#include <wait.h>

typedef struct {
    int value;
    wait_queue_t wait_queue;
} semaphore_t;

void sem_init(semaphore_t *sem, int value);
void wait(semaphore_t *sem);
void signal(semaphore_t *sem);

#endif /* _SEM_H_ */
