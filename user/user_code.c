#include <printf.h>

int main() {
  int n, pid;
  for (n = 0; n < 5; n++) {
    if ((pid = fork()) == 0) {
      printf("I am child %d\n", n);
    }
    assert(pid > 0);
  }

  for (; n > 0; n--) {
    assert(wait() == 0);
  }

  assert(wait() != 0);
  printf("I am parent %d\n", n);
  return 0;
}