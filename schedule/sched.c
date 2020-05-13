#include <list.h>
#include <printf.h>
#include <proc.h>
#include <sched.h>
#include <sched_rr.h>
#include <sync.h>

static list_entry_t timer_list;

static struct sched_class *sched;

static struct run_queue *rq;

static inline void sched_enqueue(struct proc_struct *proc) {
  if (proc != idleproc) {
    sched->enqueue(rq, proc);
  }
}

static inline void sched_dequeue(struct proc_struct *proc) {
  sched->dequeue(rq, proc);
}

static inline struct proc_struct *sched_pick_next(void) {
  return sched->pick_next(rq);
}


static void scheduler_tick() {
  if (current != idleproc) {
    sched->proc_tick(rq, current);
  } else {
    current->need_resched = 1;
  }
}

static struct run_queue __rq;

void sched_init(void) {
  list_init(&timer_list);

  sched = &RR_sched;

  rq = &__rq;
  rq->max_time_slice = 5;
  sched->init(rq);

  printf("sched class: %s\n", sched->name);
}

void wakeup_proc(struct proc_struct *proc) {
  assert(proc->state != PROC_ZOMBIE);
  bool intr_flag;
  local_intr_save(intr_flag);
  {
    if (proc->state != PROC_RUNNABLE) {
      proc->state = PROC_RUNNABLE;
      proc->wait_state = 0;
      if (proc != current) {
        sched_enqueue(proc);
      }
    } else {
      printf("wakeup runnable process.\n");
    }
  }
  local_intr_restore(intr_flag);
}

void schedule(void) {
  bool intr_flag;
  struct proc_struct *next;
  local_intr_save(intr_flag);
  {
    current->need_resched = 0;
    if (current->state == PROC_RUNNABLE) {
      sched_enqueue(current);
    }
    if ((next = sched_pick_next()) != NULL) {
      sched_dequeue(next);
    }
    if (next == NULL) {
      next = idleproc;
    }
    next->runs++;
    if (next != current) {
      proc_run(next);
    }
  }
  local_intr_restore(intr_flag);
}

void add_timer(timer_t *timer) {
  bool intr_flag;
  local_intr_save(intr_flag);
  {
    assert(timer->expires > 0 && timer->proc != NULL);
    assert(list_empty(&(timer->timer_link)));
    list_entry_t *le = list_next(&timer_list);
    while (le != &timer_list) {
      timer_t *next = le2timer(le, timer_link);
      if (timer->expires < next->expires) {
        next->expires -= timer->expires;
        break;
      }
      timer->expires -= next->expires;
      le = list_next(le);
    }
    list_add_before(le, &(timer->timer_link));
  }
  local_intr_restore(intr_flag);
}

void del_timer(timer_t *timer) {
  bool intr_flag;
  local_intr_save(intr_flag);
  {
    if (!list_empty(&(timer->timer_link))) {
      if (timer->expires != 0) {
        list_entry_t *le = list_next(&(timer->timer_link));
        if (le != &timer_list) {
          timer_t *next = le2timer(le, timer_link);
          next->expires += timer->expires;
        }
      }
      list_del_init(&(timer->timer_link));
    }
  }
  local_intr_restore(intr_flag);
}

void run_timer_list(void) {
  bool intr_flag;
  local_intr_save(intr_flag);
  {
    list_entry_t *le = list_next(&timer_list);
    if (le != &timer_list) {
      timer_t *timer = le2timer(le, timer_link);
      assert(timer->expires != 0);
      timer->expires--;
      while (timer->expires == 0) {
        le = list_next(le);
        struct proc_struct *proc = timer->proc;

        assert(proc->wait_state == WT_TIMER);

        wakeup_proc(proc);
        del_timer(timer);
        if (le == &timer_list) {
          break;
        }
        timer = le2timer(le, timer_link);
      }
    }
    scheduler_tick();
  }
  local_intr_restore(intr_flag);
}
