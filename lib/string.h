#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t len);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t len);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

void *memset(void *s, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);

#endif /* _STRING_H_ */

