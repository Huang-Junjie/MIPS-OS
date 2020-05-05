#ifndef _ULIB_H_
#define _ULIB_H_

void exit(int error_code);
int fork(void);
int wait(void);
int waitpid(int pid, int *store);
int sleep(unsigned int time);

#endif /* _ULIB_H_ */
