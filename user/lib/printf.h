#ifndef _USER_PRINTF_H_
#define _USER_PRINTF_H_

void printf(const char* fmt, ...);

void _panic(const char *, int, const char *, ...);

#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#endif /* _USER_PRINTF_H_ */
