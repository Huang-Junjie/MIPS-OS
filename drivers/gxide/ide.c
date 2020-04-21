#include <string.h>

extern int read_sector(unsigned int ideno, unsigned int offset);
extern int write_sector(unsigned int ideno, unsigned int offset);

int
ide_read_secs(unsigned int ideno, unsigned int secno, void *dst, unsigned int nsecs) {
    int i, ret = 0;
    for (i = 0; i < nsecs; i++) {
        if (read_sector(ideno, (secno + i) * 512)) {
            strncpy(dst + i * 512, 0xB3004000, 512);
        } else {
            ret = -1;
            break;
        }
    }
    return ret;
}

int
ide_write_secs(unsigned int ideno, unsigned int secno, void *src, unsigned int nsecs) {
    int i, ret = 0;
    for (i = 0; i < nsecs; i++) {
        strncpy(0xB3004000, src + i * 512, 512);
        if (!write_sector(ideno, (secno + i) * 512)) {
            ret = -1;
            break;
        }
    }
    return ret;
}

