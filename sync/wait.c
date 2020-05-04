#include <wait.h>
#include <sched.h>
#include <printf.h>

void wait_init(wait_t *wait, struct proc_struct *proc) {
  wait->proc = proc;
  wait->wakeup_flags = 0;
  list_init(&(wait->wait_link));
}

void wait_queue_init(wait_queue_t *queue) { list_init(&(queue->wait_head)); }

void wait_queue_add(wait_queue_t *queue, wait_t *wait) {
  assert(list_empty(&(wait->wait_link)) && wait->proc != NULL);
  wait->wait_queue = queue;
  list_add_before(&(queue->wait_head), &(wait->wait_link));
}

void wait_queue_del(wait_queue_t *queue, wait_t *wait) {
  assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
  list_del_init(&(wait->wait_link));
}

wait_t *wait_queue_first(wait_queue_t *queue) {
  list_entry_t *le = list_next(&(queue->wait_head));
  if (le != &(queue->wait_head)) {
    return le2wait(le, wait_link);
  }
  return NULL;
}

void wakeup_wait(wait_queue_t *queue, wait_t *wait, uint32_t wakeup_flags,
                 bool del) {
  if (del) {
    wait_queue_del(queue, wait);
  }
  wait->wakeup_flags = wakeup_flags;
  wakeup_proc(wait->proc);
}