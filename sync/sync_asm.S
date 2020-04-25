#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

LEAF(cli)
    mfc0	t0, CP0_STATUS
	and     t0, ~0x1
	mtc0	t0, CP0_STATUS
    jr      ra
END(cli)

LEAF(sti)
    mfc0	t0, CP0_STATUS
	or      t0, 0x1
	mtc0	t0, CP0_STATUS
    jr      ra
END(sti)