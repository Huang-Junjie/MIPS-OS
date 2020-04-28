#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

LEAF(syscall)
    move    s0, a0      //syscall number
    move    s1, a1      //arg1
    move    s2, a2      //arg2
    move    s3, a3      //arg3
    move    s4, 16(sp)  //arg4
    move    s5, 20(sp)  //arg5
    syscall
    jr      ra
END(syscall)

