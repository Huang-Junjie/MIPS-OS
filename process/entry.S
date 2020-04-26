#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/cp0regdef.h>

NESTED(kernel_thread_entry, 0, ra)
    move    a0, t0  #arg  
    jal     t1      #fn
    move    a0, v0  
    #jal     do_exit
END(kernel_thread_entry)
