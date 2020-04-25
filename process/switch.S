#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <trap.h>


LEAF(switch_to)
    # save from's registers
    sw  $1,TF_REG1(a0)
    sw  $2,TF_REG2(a0)
    sw  $3,TF_REG3(a0)
    sw  $4,TF_REG4(a0)
    sw  $5,TF_REG5(a0)
    sw  $6,TF_REG6(a0)
    sw  $7,TF_REG7(a0)
    sw  $8,TF_REG8(a0)
    sw  $9,TF_REG9(a0)
    sw  $10,TF_REG10(a0)
    sw  $11,TF_REG11(a0)
    sw  $12,TF_REG12(a0)
    sw  $13,TF_REG13(a0)
    sw  $14,TF_REG14(a0)
    sw  $15,TF_REG15(a0)
    sw  $16,TF_REG16(a0)
    sw  $17,TF_REG17(a0)
    sw  $18,TF_REG18(a0)
    sw  $19,TF_REG19(a0)
    sw  $20,TF_REG20(a0)
    sw  $21,TF_REG21(a0)
    sw  $22,TF_REG22(a0)
    sw  $23,TF_REG23(a0)
    sw  $24,TF_REG24(a0)
    sw  $25,TF_REG25(a0)
    sw  $26,TF_REG26(a0)
    sw  $27,TF_REG27(a0)
    sw  $28,TF_REG28(a0)
    sw  $29,TF_REG29(a0)
    sw  $30,TF_REG30(a0)
    sw  $31,TF_REG31(a0)

    mfhi    t0
    sw      t0, (TF_REG31+4)(a0)
    mflo    t0
    sw      t0, (TF_REG31+8)(a0)

    # restore to's registers
    lw      t0, (TF_REG31+4)(a1)
    mthi    t0
    lw      t0, (TF_REG31+8)(a1)
    mtlo    t0

    lw  $1,TF_REG1(a1)
    lw  $2,TF_REG2(a1)
    lw  $3,TF_REG3(a1)
    lw  $4,TF_REG4(a1)
    lw  $6,TF_REG6(a1)
    lw  $7,TF_REG7(a1)
    lw  $8,TF_REG8(a1)
    lw  $9,TF_REG9(a1)
    lw  $10,TF_REG10(a1)
    lw  $11,TF_REG11(a1)
    lw  $12,TF_REG12(a1)
    lw  $13,TF_REG13(a1)
    lw  $14,TF_REG14(a1)
    lw  $15,TF_REG15(a1)
    lw  $16,TF_REG16(a1)
    lw  $17,TF_REG17(a1)
    lw  $18,TF_REG18(a1)
    lw  $19,TF_REG19(a1)
    lw  $20,TF_REG20(a1)
    lw  $21,TF_REG21(a1)
    lw  $22,TF_REG22(a1)
    lw  $23,TF_REG23(a1)
    lw  $24,TF_REG24(a1)
    lw  $25,TF_REG25(a1)
    lw  $26,TF_REG26(a1)
    lw  $27,TF_REG27(a1)
    lw  $28,TF_REG28(a1)
    lw  $29,TF_REG29(a1)
    lw  $30,TF_REG30(a1)
    lw  $31,TF_REG31(a1)
    lw  $5,TF_REG5(a1)
    jr  ra
END(switch_to)


LEAF(set_asid)
    mfc0    t0, CP0_ENTRYHI
    and     t0, ~0xff
    or      t0, a0
    mtc0    t0, CP0_ENTRYHI
    jr      ra
END(set_asid)