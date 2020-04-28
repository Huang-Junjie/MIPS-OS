#include <printf.h>
#include <syscall.h>

void exit(int error_code) {
  sys_exit(error_code);
  panic("exit will not return! %d.\n");
}

int fork(void) { return sys_fork(); }

int wait(void) { return sys_wait(0, NULL); }

int waitpid(int pid, int *store) { return sys_wait(pid, store); }

int sleep(unsigned int time) { return sys_sleep(time); }
