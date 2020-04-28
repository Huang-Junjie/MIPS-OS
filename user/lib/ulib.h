#ifndef _ULIB_H_
#define _ULIB_H_

#define assert(x)                        \
  do {                                   \
    if (!(x)) {                          \
      panic("assertion failed: %s", #x); \
    }                                    \
  } while (0)


void exit(int error_code);
int fork(void);
int wait(void);
int waitpid(int pid, int *store);
int sleep(unsigned int time);

#endif /* _ULIB_H_ */
