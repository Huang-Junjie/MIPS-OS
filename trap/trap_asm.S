#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <stackframe.h>

LEAF(tlb_set)
    tlbp
    mfc0    t0, CP0_INDEX
    bltz    t0, clear_lo
    tlbr
    j       clear_lo_b
clear_lo:
    mtc0    zero,CP0_ENTRYLO0
    mtc0    zero,CP0_ENTRYLO1
clear_lo_b:
    andi    a0, 0x00001000
    beqz    a0, set_lo0
    mtc0    a1,CP0_ENTRYLO1
    j       set_lo0_b
set_lo0:
    mtc0    a1,CP0_ENTRYLO0
set_lo0_b:
    bltz    t0, random_write
    tlbwi
    j       random_write_b
random_write:
    tlbwr
random_write_b:
    jr      ra
END(tlb_set)


LEAF(forkret)
    /* 恢复trapframe */
    RESTORE_ALL
    eret
END(forkret)