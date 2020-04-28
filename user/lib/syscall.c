#include <syscall.h>
#include <sysnum.h>

extern int syscall(int num, int arg1, int arg2, int arg3, int arg4, int arg5);

int sys_exit(int error_code) {
  return syscall(SYS_exit, error_code, 0, 0, 0, 0);
}

int sys_fork(void) { return syscall(SYS_fork, 0, 0, 0, 0, 0); }

int sys_wait(int pid, int *store) {
  return syscall(SYS_wait, pid, store, 0, 0, 0);
}

int sys_putc(int c) { return syscall(SYS_putc, c, 0, 0, 0, 0); }

int sys_sleep(unsigned int time) {
  return syscall(SYS_sleep, time, 0, 0, 0, 0);
}
