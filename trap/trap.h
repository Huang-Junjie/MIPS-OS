#ifndef _TRAP_H_
#define _TRAP_H_

#ifndef __ASSEMBLER__
struct trapframe {
    /* 保存通用寄存器 */
    unsigned int regs[32];
    /* 保存特殊功能寄存器 */
    unsigned int hi;
    unsigned int lo;
    /* 保存cp0寄存器 */
    unsigned int cp0_status;
    unsigned int cp0_cause;
    unsigned int cp0_epc;
    unsigned int cp0_badvaddr;
};
#endif /* __ASSEMBLER__ */

#define TF_REG0     0
#define TF_REG1     ((TF_REG0) + 4)
#define TF_REG2     ((TF_REG1) + 4)
#define TF_REG3     ((TF_REG2) + 4)
#define TF_REG4     ((TF_REG3) + 4)
#define TF_REG5     ((TF_REG4) + 4)
#define TF_REG6     ((TF_REG5) + 4)
#define TF_REG7     ((TF_REG6) + 4)
#define TF_REG8     ((TF_REG7) + 4)
#define TF_REG9     ((TF_REG8) + 4)
#define TF_REG10    ((TF_REG9) + 4)
#define TF_REG11    ((TF_REG10) + 4)
#define TF_REG12    ((TF_REG11) + 4)
#define TF_REG13    ((TF_REG12) + 4)
#define TF_REG14    ((TF_REG13) + 4)
#define TF_REG15    ((TF_REG14) + 4)
#define TF_REG16    ((TF_REG15) + 4)
#define TF_REG17    ((TF_REG16) + 4)
#define TF_REG18    ((TF_REG17) + 4)
#define TF_REG19    ((TF_REG18) + 4)
#define TF_REG20    ((TF_REG19) + 4)
#define TF_REG21    ((TF_REG20) + 4)
#define TF_REG22    ((TF_REG21) + 4)
#define TF_REG23    ((TF_REG22) + 4)
#define TF_REG24    ((TF_REG23) + 4)
#define TF_REG25    ((TF_REG24) + 4)
/* $26(k0)和$27(k1)可以不用保存 */
#define TF_REG26    ((TF_REG25) + 4)
#define TF_REG27    ((TF_REG26) + 4)
#define TF_REG28    ((TF_REG27) + 4)
#define TF_REG29    ((TF_REG28) + 4)
#define TF_REG30    ((TF_REG29) + 4)
#define TF_REG31    ((TF_REG30) + 4)

#define TF_HI       ((TF_REG31) + 4)
#define TF_LO       ((TF_HI) + 4)

#define TF_STATUS   ((TF_LO)+4)
#define TF_CAUSE    ((TF_STATUS) + 4)
#define TF_EPC      ((TF_CAUSE) + 4)
#define TF_BADVADDR ((TF_EPC) + 4)

/* stackframe的大小 */
#define TF_SIZE     ((TF_BADVADDR)+4)

/* 进程控制块中的kstack偏移 */	
#define KSTACK_OFFSET    20	
#define KSTACK_SIZE  8192	


#endif /* _TRAP_H_ */
