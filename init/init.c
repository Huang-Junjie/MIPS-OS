#include <printf.h>
#include <clock.h>
#include <pmm.h>
#include <vmm.h>
#include <swap.h>

void init() {
    printf("init.c:\tEnter init.\n");


    pmm_init();

    vmm_init();
    swap_init();

    clock_init(100);

    while (1);
    panic("init.c:\tend of mips_init() reached!");
}
