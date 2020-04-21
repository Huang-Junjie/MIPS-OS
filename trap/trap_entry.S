#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <stackframe.h>


/* 异常处理入口 */
.section .text.exc_vec
LEAF(except_vec)
    /* 保存trapframe */
    SAVE_ALL
    move    a0, sp
    jal      trap
    /* 恢复trapframe */
    RESTORE_ALL
    eret
END(except_vec)


/* tlb miss 异常处理入口 */
.section .text.exc_tlb
LEAF(except_tlb)
    j   except_vec
END(except_tlb)
