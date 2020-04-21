#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>


.text
LEAF(clock_init)
    /* 中断使能； 取消对4号中断（时钟）的屏蔽 */
    mfc0    t0, CP0_STATUS
    or      t0, 0x1001
    mtc0    t0, CP0_STATUS

    /* 设置时钟频率 */
    sw      a0, 0xb5000100
    jr      ra

END(clock_init)
