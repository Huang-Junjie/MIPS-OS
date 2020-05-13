#include <printf.h>
#include <ulib.h>

int main() {
  int i, pid[5], ecode;
  for (i = 0; i < 5; i++) {
    if ((pid[i] = fork()) == 0) {
      printf("I am child %d\n", i);
      return i;
    }
  }

  for (i = 0; i < 5; i++) {
    waitpid(pid[i], &ecode);
    assert(ecode == i);
  }

  assert(wait() != 0);

  printf("I am parent\n");
  return 0;
}