#include <printf.h>

int main() {
    int pid = fork();
    if (pid == 0) {
        printf("user: child\n");
    }else {
        printf("user: parent\n");
    }
    return 0;
}