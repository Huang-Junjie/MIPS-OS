#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>


.data
.align 12
    .globl _boot_pgdir
_boot_pgdir:
    .space 4096

    .globl cur_pgdir
cur_pgdir:
    .word _boot_pgdir

    .globl cur_kernel_sp
cur_kernel_sp:
    .word 0


/* 操作系统入口*/
.text
LEAF(_start)
    /* 关闭中断 */
    mtc0    zero, CP0_STATUS

    /* 关闭观测异常 */
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI


    /* 设置栈指针 */
    la  sp, 0x80400000


    /* 进入init函数 */
    jal    init

loop:
    j    loop
END(_start)

