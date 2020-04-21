#include <string.h>


size_t strlen(const char *s) {
    size_t cnt = 0;
    while (*s++ != '\0') {
        cnt++;
    }
    return cnt;
}


size_t strnlen(const char *s, size_t len) {
    size_t cnt = 0;
    while (cnt < len && *s++ != '\0') {
        cnt++;
    }
    return cnt;
}


char *strcpy(char *dst, const char *src) {
    char *p = dst;
    while ((*p++ = *src++) != '\0');
    return dst;
}


char *strncpy(char *dst, const char *src, size_t len) {
    char *p = dst;
    while (len > 0) {
        if ((*p = *src) != '\0') {
            src++;
        }
        p++;
        len--;
    }
    return dst;
}


int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (int)((unsigned char)*s1 - (unsigned char)*s2);
}


int strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 != '\0' && *s1 == *s2) {
        n--;
        s1++;
        s2++;
    }
    return n == 0 ? 0 : (int)((unsigned char)*s1 - (unsigned char)*s2);
}




void *memset(void *s, char c, size_t n) {
    char *p = s;
    while (n-- > 0) {
        *p++ = c;
    }
    return s;
}


void *memcpy(void *dst, const void *src, size_t n) {
    const char *s = src;
    char *d = dst;
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dst;
}