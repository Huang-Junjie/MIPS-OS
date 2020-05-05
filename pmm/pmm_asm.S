#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

LEAF(tlb_out)
    mfc0    t0, CP0_ENTRYHI
    and     t1, t0, 0xff
    and     t2, a0, 0xffffefff
    or      t1, t2
    mtc0    t1, CP0_ENTRYHI
    tlbp

    mfc0    t1, CP0_INDEX
    bltz    t1, NOFOUND

    tlbr
    andi    t1, a0, 0x00001000
    beqz    t1, clear_Lo0
    mtc0    zero, CP0_ENTRYLO1
    j       clear_Lo0_b
clear_Lo0:
    mtc0    zero, CP0_ENTRYLO0
clear_Lo0_b:
    tlbwi

NOFOUND:
    mtc0    t0, CP0_ENTRYHI
    jr      ra
END(tlb_out)
