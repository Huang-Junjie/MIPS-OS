#include <proc.h>
#include <sem.h>
#include <sync.h>
#include <printf.h>
#include <sched.h>

void sem_init(semaphore_t *sem, int value) {
  sem->value = value;
  wait_queue_init(&(sem->wait_queue));
}

void wait(semaphore_t *sem) {
  bool intr_flag;
  local_intr_save(intr_flag);
  if (sem->value > 0) {
    sem->value--;
    local_intr_restore(intr_flag);
    return;
  }

  //将当前进程加入等待队列
  wait_t __wait, *wait = &__wait;
  assert(current != NULL);
  wait_init(wait, current);
  current->state = PROC_SLEEPING;
  current->wait_state = WT_SEM;
  wait_queue_add(&(sem->wait_queue), wait);
  local_intr_restore(intr_flag);

  schedule();

  assert(wait->wakeup_flags == WT_SEM);
}

void signal(semaphore_t *sem) {
  bool intr_flag;
  local_intr_save(intr_flag);
  {
    wait_t *wait;
    if ((wait = wait_queue_first(&(sem->wait_queue))) == NULL) {
      sem->value++;
    } else {
      assert(wait->proc->wait_state == WT_SEM);
      //唤醒等待队列中第一个进程，并将其从队列中删除
      wakeup_wait(&(sem->wait_queue), wait, WT_SEM, 1);
    }
  }
  local_intr_restore(intr_flag);
}