#ifndef __KERN_DRIVER_IDE_H__
#define __KERN_DRIVER_IDE_H__

int ide_read_secs(unsigned int ideno, unsigned int secno, void *dst, unsigned int nsecs);
int ide_write_secs(unsigned int ideno, unsigned int secno, void *src, unsigned int nsecs);

#endif /* !__KERN_DRIVER_IDE_H__ */

