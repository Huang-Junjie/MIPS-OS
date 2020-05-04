#ifndef _SYNC_H_
#define _SYNC_H_

#include <types.h>

void sti(void);
void cli(void);

static inline bool
__intr_save(void) {
    cli();
}

static inline void
__intr_restore(bool flag) {
    if (flag) {
        sti();
    }
}

#define local_intr_save(x)      do { x = __intr_save(); } while (0)
#define local_intr_restore(x)   __intr_restore(x);

#endif /* _SYNC_H_ */