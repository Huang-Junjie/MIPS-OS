#include <printf.h>
#include <proc.h>
#include <sem.h>

#define N 5                  /* 哲学家数目 */
#define LEFT (i - 1 + N) % N /* i的左邻号码 */
#define RIGHT (i + 1) % N    /* i的右邻号码 */
#define TIMES 4              /* 吃4次饭 */
#define SLEEP_TIME 10

enum { THINKING, HUNGRY, EATING } state[N]; /* 记录每个人状态的数组 */
semaphore_t mutex;                          /* 临界区互斥 */
semaphore_t self[N];                        /* 每个哲学家一个信号量 */

struct proc_struct *philosopher_proc[N];

static void test(i) {
  if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
    state[i] = EATING;
    signal(&self[i]);
  }
}

static void pickup(int i) {
  wait(&mutex);      /* 进入临界区 */
  state[i] = HUNGRY; /* 记录下哲学家i饥饿的事实 */
  test(i);           /* 试图得到两只筷子 */
  signal(&mutex);    /* 离开临界区 */
  wait(&self[i]);    /* 如果得不到筷子就阻塞 */
}

static void putdown(int i) {
  wait(&mutex);        /* 进入临界区 */
  state[i] = THINKING; /* 哲学家进餐结束 */
  test(LEFT);          /* 看一下左邻居现在是否能进餐 */
  test(RIGHT);         /* 看一下右邻居现在是否能进餐 */
  signal(&mutex);      /* 离开临界区 */
}

//哲学家线程函数
static int philosopher(void *arg) {
  int i, iter = 0;
  i = (int)arg;
  printf("I am No.%d philosopher\n", i);
  while (iter++ < TIMES) {
    //思考
    printf("Iter %d, No.%d philosopher is thinking\n", iter, i);
    do_sleep(SLEEP_TIME);
    //拿起两只筷子或阻塞
    pickup(i);
    //进餐
    printf("Iter %d, No.%d philosopher is eating\n", iter, i);
    do_sleep(SLEEP_TIME);
    //放下两只筷子
    putdown(i);
  }
  printf("No.%d philosopher quit\n", i);
  return 0;
}

void check_sync(void) {
  int i;
  sem_init(&mutex, 1);
  for (i = 0; i < N; i++) {
    sem_init(&self[i], 0);
    int pid = kernel_thread(philosopher, (void *)i, 0);
    if (pid <= 0) {
      panic("create No.%d philosopher failed.\n");
    }
  }
}
