#include <syscall.h>
#include <sysnum.h>

extern int syscall(int num, ...);

int sys_exit(int error_code) { return syscall(SYS_exit, error_code); }

int sys_fork(void) { return syscall(SYS_fork); }

int sys_wait(int pid, int *store) { return syscall(SYS_wait, pid, store); }

int sys_putc(int c) { return syscall(SYS_putc, c); }

int sys_sleep(unsigned int time) { return syscall(SYS_sleep, time); }
