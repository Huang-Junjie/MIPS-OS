#include <printf.h>
#include <clock.h>
#include <pmm.h>
#include <vmm.h>
#include <swap.h>
#include <proc.h>
#include <sched.h>

void init() {
    printf("init.c:\tEnter init.\n");


    pmm_init();

    vmm_init();
    swap_init();

    sched_init();
    proc_init();
    clock_init(100);

    cpu_idle(); 
    panic("init.c:\tend of mips_init() reached!");
}
