#include <printf.h>
#include <trap.h>
#include <proc.h>
#include <pmm.h>
#include <vmm.h>


static void print_ticks() {
    static int ticks = 0;
    ticks++;
    if (ticks % 100 == 0) {
        printf("%d ticks\n", ticks);
    }
}


static void handle_int(struct trapframe *tf) {
    /* 判断是否是时钟中断 */
    if (tf->cp0_cause & tf->cp0_status & 0x1000) {
        *(char *)0xb5000110 = 1; //响应时钟中断
        print_ticks();
    }
    //delete
    else {
        print_ticks("else int");
    }
}

static void handle_mod(struct trapframe *tf) {
    panic("write a non-writable addr: 0x%08x", tf->cp0_badvaddr);
}


extern void tlb_set(uintptr_t badvaddr, pte_t pte);
static void handle_tlb(struct trapframe *tf) {
    pte_t *ptep = get_pte(current->pgdir, tf->cp0_badvaddr, 0);
    while(1) {
        if (ptep != NULL && *ptep & PTE_V) {
            tlb_set(tf->cp0_badvaddr, (*ptep) >> 6);
            return;
        } 
        else {
            pgfault_handler(tf);
        }
    }
}


static void trap_dispatch(struct trapframe *tf) {
    switch ((tf->cp0_cause | 0x7c) >> 2) {
        case 0:  //Int
            handle_int(tf);
            break;
        case 1: //TLB mod
            handle_mod(tf);
            break;
        case 2: //TLBL
        case 3: //TLBS
            handle_tlb(tf);
            break;
        case 8: //Syscall
            ;

    }
}

void trap(struct trapframe *tf) {
    printf("enter trap--------\n");
    if (current == NULL) {
        trap_dispatch(tf);
    }
    else {
        // 将当前进程的tf指向当前tf
        struct trapframe *old_tf = current->tf;
        current->tf = tf;
        trap_dispatch(tf);
        current->tf = old_tf;
    }
}

