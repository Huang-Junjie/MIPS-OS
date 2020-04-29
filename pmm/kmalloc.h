#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#include <types.h>


void *kmalloc(size_t n);
void kfree(void *objp);


#endif /* !_KMALLOC_H_ */

