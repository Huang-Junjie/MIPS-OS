#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_

int sys_exit(int error_code);
int sys_fork(void);
int sys_wait(int pid, int *store);
int sys_putc(int c);
int sys_sleep(unsigned int time);


#endif /* _USER_SYSCALL_H_ */

