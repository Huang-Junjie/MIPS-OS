#include <printf.h>
#include <console.h>
#include <stdarg.h>


void printStr(const char* s) {
    while (*s) {
        putChar(*s++);
    }
}

void printNum(unsigned int num, int base, int negFlag) {
    char buf[32];
    int len = 0;

    if (negFlag) {
        putChar('-');
    }
    if (num == 0) {
        buf[len++] = '0';
    }
    while (num != 0) {
        if (num % base < 10) {
            buf[len++] = num % base + '0';
        } else {
            buf[len++] = num % base + 'a' - 10;
        }
        num = num / base;
    }

    while (len != 0) {
        putChar(buf[--len]);
    }
}


void vprintf(const char* fmt, va_list ap) {
    int num;
    int negFlag;
    while (1) {
        negFlag = 0;
        while (*fmt != '%' && *fmt != '\0') {
            putChar(*fmt++);
        }
        if (*fmt == '\0') break;

        switch (*(++fmt)) {
        case 'c':
            putChar((char)va_arg(ap, int));
            break;
        case 'd':
            num = va_arg(ap, int);
            if (num < 0) {
                num = -num;
                negFlag = 1;
            }
            printNum(num, 10, negFlag);
            break;
        case 'o':
            printNum(va_arg(ap, int), 8, 0);
            break;
        case 's':
            printStr(va_arg(ap, char *));
            break;
        case 'u':
            printNum(va_arg(ap, int), 10, 0);
            break;
        case 'x':
            printNum(va_arg(ap, int), 16, 0);
            break;
        case '\0':
            fmt--;
            break;
        default:
            putChar(*fmt);
        }
        fmt++;
    }
}

void printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}


void _panic(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("panic at %s:%d: ", file, line);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);

    for (;;);
}
