/******************************************************************************
Filename   : rme_platform_ch32v307vct6.h
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The configuration file for STM32F405RG.
******************************************************************************/

/* Defines *******************************************************************/
/* Debugging *****************************************************************/
#define RME_ASSERT_CORRECT                              (0U)
#define RME_DEBUG_PRINT                                 (1U)
/* Generator *****************************************************************/
/* Are we using the generator? */
#define RME_RVM_GEN_ENABLE                              (0U)
/* Modifiable ****************************************************************/
/* The virtual memory start address for the kernel objects */
#define RME_KOM_VA_BASE                                 (0x20003000)
/* The size of the kernel object virtual memory */
#define RME_KOM_VA_SIZE                                 (0xD000)
/* The virtual memory start address for the virtual machines - If no VM is used, set to 0 */
#define RME_HYP_VA_BASE                                 (0x0U)
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_VA_SIZE                                 (0xFFFFFFFFU)
/* The granularity of kernel memory allocation, in bytes */
#define RME_KOM_SLOT_ORDER                              (4U)
/* Kernel stack size and address */
#define RME_KSTK_VA_BASE                                (0x20000FF0U)
#define RME_KSTK_VA_SIZE                                (0x800U)
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define RME_PREEMPT_PRIO_NUM                            (32U)

/* Physical vector number, flag area base and its size */
#define RME_RVM_PHYS_VCT_NUM                            (104U)
#define RME_RVM_PHYS_VCTF_BASE                          (0x20002FC0U)
#define RME_RVM_PHYS_VCTF_SIZE                          (0x40U)
/* Virtual event number, flag area base and its size */
#define RME_RVM_VIRT_EVT_NUM                            (32U)
#define RME_RVM_VIRT_EVTF_BASE                          (0x20002FA0U)
#define RME_RVM_VIRT_EVTF_SIZE                          (0x20U)
/* Size of initial capability table */
#define RME_RVM_INIT_CPT_SIZE                           (54U)
/* Initial kernel object frontier limit */
#define RME_RVM_CPT_BOOT_FRONT                          (9U)
#define RME_RVM_KOM_BOOT_FRONT                          (0x1000U)
/* Post-boot kernel object frontier limit */
#define RME_RVM_CPT_DONE_FRONT                          (22U)
#define RME_RVM_KOM_DONE_FRONT                          (0x1C10U)

/* Init process's first thread's entry point address */
#define RME_RV32GP_INIT_ENTRY                           (0x8010021U)
/* Init process's first thread's stack address */
#define RME_RV32GP_INIT_STACK                           (0x2001FFF0)
/* What is the Systick value? - 10ms per tick*/
#define RME_RV32GP_OSTIM_VAL                            (96000)
/* Number of MPU regions available */
#define RME_RV32GP_REGION_NUM                           (4U)
/* What is the FPU type? */
#define RME_COP_NUM                                     (1U)
#define RME_RV32GP_COP_RVF                              (1U)
#define RME_RV32GP_COP_RVD                              (0U)

/* Timer interrupt MCAUSE value */
#define RME_RV32GP_MCAUSE_TIM                           (0x8000000CU)

/* CSR - GCC specific */
#define RME_RV32GP_CSR_SET(X,V) \
do \
{ \
    __asm__ __volatile__("LI    t0, %1  \n\t" \
                         "CSRW  %0, t0  \n\t" \
                         ::"i"(X),"i"(V):"t0"); \
} \
while(0)

/* PFIC */
#define RME_RV32GP_PFIC_ISR(X)                          RME_RV32GP_REG(0xE000E000U+(X))
#define RME_RV32GP_PFIC_IENR(X)                         RME_RV32GP_REG(0xE000E100U+(X))
#define RME_RV32GP_PFIC_IRER(X)                         RME_RV32GP_REG(0xE000E180U+(X))
#define RME_RV32GP_PFIC_CFGR                            RME_RV32GP_REG(0xE000E048U)
#define RME_RV32GP_PFIC_IPRIOR(X)                       RME_RV32GP_REGB(0xE000E400U+(X))

/* Wait for interrupt and place processor in low-power mode */
#define RME_RV32GP_WAIT_INT()

/* Get interrupt state */
#define RME_RV32GP_INT_GET_STATE(INT_NUM) \
do \
{ \
    if((RME_RV32GP_PFIC_ISR((INT_NUM)>>5)&RME_POW2((INT_NUM)&0x1FU))!=0U) \
        return 1; \
} \
while(0)

/* Enable interrupt */
#define RME_RV32GP_INT_SET_ENABLE(INT_NUM) \
do \
{ \
    RME_RV32GP_PFIC_IENR((INT_NUM)>>5)|=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Disable interrupt */
#define RME_RV32GP_INT_SET_DISABLE(INT_NUM) \
do \
{ \
    RME_RV32GP_PFIC_IRER((INT_NUM)>>5)|=RME_POW2((INT_NUM)&0x1FU); \
} \
while(0)

/* Get interrupt priority */
#define RME_RV32GP_INT_GET_PRIO(INT_NUM) \
do \
{ \
    return RME_RV32GP_PFIC_IPRIOR(INT_NUM); \
} \
while(0)

/* Set interrupt priority */
#define RME_RV32GP_INT_SET_PRIO(INT_NUM, PRIO) \
do \
{ \
    RME_RV32GP_PFIC_IPRIOR(INT_NUM)=(PRIO); \
} \
while(0)

/* Local interrupt triggering */
#define RME_RV32GP_INT_LOCAL_TRIG(INT_NUM)

/* Cache mode configuration */
#define RME_RV32GP_CACHE_MOD(CACHE_ID, OPERATION, PARAM)
/* Cache maintenance */
#define RME_RV32GP_CACHE_MAINT(CACHE_ID, OPERATION, PARAM)
/* Prefetcher mode ocnfiguration */
#define RME_RV32GP_PRFTH_MOD(PRFTH_ID, OPERATION, PARAM)

/* Performance monitor configuration */
#define RME_RV32GP_PERF_MON_MOD(PERF_ID, OPERATION, PARAM)
/* Performance monitor cycle counter read or write */
#define RME_RV32GP_PERF_CYCLE_MOD(CYCLE_ID, OPERATION, VALUE)

/* Reboot system */
#define RME_RV32GP_REBOOT()

/* Preinitialization of critical hardware */
#define RME_RV32GP_LOWLVL_PREINIT() \
do \
{ \
    /* INTSYSCR(0x0804): turn off hardware stacking, 3 levels of nesting */ \
    RME_RV32GP_CSR_SET(0x0804U, 0x0000E00EU); \
    /* CORECFGR(0x0BC0): default value 0x1F */ \
    RME_RV32GP_CSR_SET(0x0BC0U, 0x0000001FU); \
} \
while(0)

#define RME_RV32GP_LOWLVL_INIT_COMMON() \
do \
{ \
} \
while(0)

#if(RME_DEBUG_PRINT==1U)
/* Other low-level initialization stuff - clock and serial */
#define RME_RV32GP_LOWLVL_INIT() \
do \
{ \
    RME_RV32GP_LOWLVL_INIT_COMMON(); \
} \
while(0)

/* This is for debugging output */
#define RME_RV32GP_PUTCHAR(CHAR) \
do \
{ \
} \
while(0)

#else

/* Other low-level initialization stuff - clock and serial */
#define RME_RV32GP_LOWLVL_INIT() \
do \
{ \
    RME_RV32GP_LOWLVL_INIT_COMMON(); \
} \
while(0)
#endif

/* Prefetcher state set and get */
#define RME_RV32GP_PRFTH_STATE_SET(STATE) \
do \
{ \
    FLASH_Enhance_Mode(STATE);\
} \
while(0)
#define RME_RV32GP_PRFTH_STATE_GET()  ((FLASH->CTLR & 1U<<24) == 0U)
/* End Defines ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

