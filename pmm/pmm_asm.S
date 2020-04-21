#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

LEAF(tlb_out)
    mfc0    k1, CP0_ENTRYHI
    and     k0, a0, 0xffffefff
    mtc0    k0, CP0_ENTRYHI
    tlbp

    mfc0    k0, CP0_INDEX
    bltz    k0, NOFOUND

    andi    k0, a0, 0x00001000
    beqz    k0, clear_Lo0
    mtc0    zero, CP0_ENTRYLO1
    j       clear_Lo0_b
clear_Lo0:
    mtc0    zero, CP0_ENTRYLO0
clear_Lo0_b:
    tlbwi

NOFOUND:
    mtc0    k1,CP0_ENTRYHI
    jr      ra
END(tlb_out)
