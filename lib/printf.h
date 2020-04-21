#ifndef _PRINTF_H_
#define _PRINTF_H_

void printf(const char* fmt, ...);

void _panic(const char *, int, const char *, ...);

#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#endif /* _PRINTF_H_ */
