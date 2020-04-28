#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

LEAF(syscall)
    move    t0, a0      //syscall number
    move    t1, a1      //arg1
    move    t2, a2      //arg2
    move    t3, a3      //arg3
    lw      t4, 16(sp)  //arg4
    lw      t5, 20(sp)  //arg5
    syscall
    jr      ra
END(syscall)

