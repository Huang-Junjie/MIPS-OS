#include <printf.h>
#include <console.h>
#include <stdarg.h>


void printStr(const char* s, int width, int padc) {
    char *s1 = s;
    int len = 0;
    while (*s1++) len++;
    if (width < len) width = len;
    while (width-- > len) putChar(padc);
    while (*s) putChar(*s++);
}

void printNum(unsigned int num, int base, int negFlag, int width, char padc) {
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

    if (width < len) width = len;
    while (width-- > len) putChar(padc);
    while (len != 0) {
        putChar(buf[--len]);
    }
}


void vprintf(const char* fmt, va_list ap) {
    long int num;
    int longFlag;
    int negFlag;
    char padc;
    int width;

    while (1) {
        while (*fmt != '%' && *fmt != '\0') {
            putChar(*fmt++);
        }
        if (*fmt == '\0') break;
        
        //跳过%
        fmt++;

        //检查副格式符
        if (*fmt == 'l') {
			longFlag = 1;
			fmt++;
		} else {
			longFlag = 0;
		}

        if (*fmt == '0') {
			padc = '0';
			fmt++;
		} else {
            padc = ' ';
        }

        width = 0;
		while (*fmt >= '0' && *fmt <= '9') {
			width = 10 * width + (*fmt - '0');
			fmt++;
		}
        negFlag = 0;

        switch (*fmt) {
        case 'c':
            if (width < 1) width = 1;
            while(width-- > 1)  putChar(padc);
            putChar((char)va_arg(ap, int));
            break;
        case 'd':
            num = longFlag ? va_arg(ap, long int) : va_arg(ap, int);
            if (num < 0) {
                num = -num;
                negFlag = 1;
            }
            printNum(num, 10, negFlag, width, padc);
            break;
        case 'o':
            num = longFlag ? va_arg(ap, long int) : va_arg(ap, int);
            printNum(num, 8, 0, width, padc);
            break;
        case 's':
            printStr(va_arg(ap, char *), width, padc);
            break;
        case 'u':
            num = longFlag ? va_arg(ap, long int) : va_arg(ap, int);
            printNum(num, 10, 0, width, padc);
            break;
        case 'x':
            num = longFlag ? va_arg(ap, long int) : va_arg(ap, int);
            printNum(num, 16, 0, width, padc);
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
