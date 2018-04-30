/******************************************************************************
Filename   : platform_cmx.h
Author     : pry
Date       : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description: The header of "platform_cmx.h".
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __PLATFORM_CMX_H_DEFS__
#define __PLATFORM_CMX_H_DEFS__
/*****************************************************************************/
/* Basic Types ***************************************************************/
#if(DEFINE_BASIC_TYPES==TRUE)

#ifndef __S32__
#define __S32__
typedef signed int  s32;
#endif

#ifndef __S16__
#define __S16__
typedef signed short s16;
#endif

#ifndef __S8__
#define __S8__
typedef signed char  s8;
#endif

#ifndef __U32__
#define __U32__
typedef unsigned int  u32;
#endif

#ifndef __U16__
#define __U16__
typedef unsigned short u16;
#endif

#ifndef __U8__
#define __U8__
typedef unsigned char  u8;
#endif

#endif
/* End Basic Types ***********************************************************/

/* Begin Extended Types ******************************************************/
#ifndef __TID_T__
#define __TID_T__
/* The typedef for the Thread ID */
typedef s32 tid_t;
#endif

#ifndef __PTR_T__
#define __PTR_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef u32 ptr_t;
#endif

#ifndef __CNT_T__
#define __CNT_T__
/* The typedef for the count variables */
typedef s32 cnt_t;
#endif

#ifndef __CID_T__
#define __CID_T__
/* The typedef for capability ID */
typedef s32 cid_t;
#endif

#ifndef __RET_T__
#define __RET_T__
/* The type for process return value */
typedef s32 ret_t;
#endif
/* End Extended Types ********************************************************/

/* System macros *************************************************************/
/* Compiler "extern" keyword setting */
#define EXTERN                  extern
/* Compiler "inline" keyword setting */
#define INLINE                  __forceinline
/* Number of CPUs in the system */
#define RME_CPU_NUM             1
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER          5
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA           (RME_TRUE)
/* Quiescence timeslice value */
#define RME_QUIE_TIME           0
/* Normal page directory size calculation macro */
#define RME_PGTBL_SIZE_NOM(NUM_ORDER)   ((1<<(NUM_ORDER))*sizeof(ptr_t)+sizeof(struct __RME_CMX_Pgtbl_Meta))
/* Top-level page directory size calculation macro */
#define RME_PGTBL_SIZE_TOP(NUM_ORDER)   (RME_PGTBL_SIZE_NOM(NUM_ORDER)+sizeof(struct __RME_CMX_MPU_Data))
/* The kernel object allocation table address - original */
#define RME_KOTBL               RME_Kotbl

/* The CPU and application specific macros are here */
#include "platform_cmx_conf.h"
/* End System macros *********************************************************/

/* Cortex-M specific macros **************************************************/
/* Initial boot capabilities */
/* The capability table of the init process */
#define RME_BOOT_CAPTBL                      0
/* The top-level page table of the init process - always 4GB full range split into 8 pages */
#define RME_BOOT_PGTBL                       1
/* The init process */
#define RME_BOOT_INIT_PROC                   2
/* The init thread */
#define RME_BOOT_INIT_THD                    3
/* The initial kernel function capability */
#define RME_BOOT_INIT_KERN                   4
/* The initial kernel memory capability */
#define RME_BOOT_INIT_KMEM                   5
/* The initial timer endpoint */
#define RME_BOOT_INIT_TIMER                  6
/* The initial fault endpoint */
#define RME_BOOT_INIT_FAULT                  7
/* The initial default endpoint for all other interrupts - this will directly go to the INTD. */
#define RME_BOOT_INIT_INT                    8

/* Booting capability layout */
#define RME_CMX_CPT              ((struct RME_Cap_Captbl*)(RME_KMEM_VA_START))
/* SRAM base */
#define RME_CMX_SRAM_BASE        0x20000000
/* For Cortex-M:
 * The layout of the page entry is:
 * [31:7] Paddr - The physical address to map this page to, or the physical
 *                address of the next layer of page table. This address is
 *                always aligned to 128 bytes. 
 * [6] Static - Is this page a static page?
 * [5] Bufferable - Is this page write-bufferable?
 * [4] Cacheable - Is this page cacheable?
 * [3] Execute - Do we allow execution(instruction fetch) on this page?
 * [2] Readonly - Is this page user read-only?
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 *
 * The layout of a directory entry is:
 * [31:2] Paddr - The in-kernel physical address of the lower page directory.
 * [1] Terminal - Is this page a terminal page, or points to another page table?
 * [0] Present - Is this entry present?
 */
/* Get the actual table positions */
#define RME_CMX_PGTBL_TBL_NOM(X)        ((X)+(sizeof(struct __RME_CMX_Pgtbl_Meta)/sizeof(ptr_t)))
#define RME_CMX_PGTBL_TBL_TOP(X)        ((X)+(sizeof(struct __RME_CMX_Pgtbl_Meta)+sizeof(struct __RME_CMX_MPU_Data))/sizeof(ptr_t))

/* Page entry bit definitions */
#define RME_CMX_PGTBL_PRESENT           (1<<0)
#define RME_CMX_PGTBL_TERMINAL          (1<<1)
#define RME_CMX_PGTBL_READONLY          (1<<2)
#define RME_CMX_PGTBL_READWRITE         (1<<3)
#define RME_CMX_PGTBL_EXECUTE           (1<<4)
#define RME_CMX_PGTBL_CACHEABLE         (1<<5)
#define RME_CMX_PGTBL_BUFFERABLE        (1<<6)
#define RME_CMX_PGTBL_STATIC            (1<<7)
/* The address mask for the actual page address */
#define RME_CMX_PGTBL_PTE_ADDR(X)       ((X)&0xFFFFFFFC)
/* The address mask for the next level page table address */
#define RME_CMX_PGTBL_PGD_ADDR(X)       ((X)&0xFFFFFFFC)
#define RME_CMX_PGTBL_FLAGMASK(X)       ((X)&0x00000078)
/* Page table metadata definitions */
#define RME_CMX_PGTBL_START(X)          ((X)&0xFFFFFFFE)
#define RME_CMX_PGTBL_SIZEORD(X)        ((X)>>16)
#define RME_CMX_PGTBL_NUMORD(X)         ((X)&0x0000FFFF)
#define RME_CMX_PGTBL_DIRNUM(X)         ((X)>>16)
#define RME_CMX_PGTBL_PAGENUM(X)        ((X)&0x0000FFFF)
#define RME_CMX_PGTBL_DIRNUM(X)         ((X)>>16)
#define RME_CMX_PGTBL_INC_PAGENUM(X)    ((X)+=0x00000001)
#define RME_CMX_PGTBL_DEC_PAGENUM(X)    ((X)-=0x00000001)
#define RME_CMX_PGTBL_INC_DIRNUM(X)     ((X)+=0x00010000)
#define RME_CMX_PGTBL_DEC_DIRNUM(X)     ((X)-=0x00010000)
/* MPU operation flag */
#define RME_CMX_MPU_CLR                 (0)
#define RME_CMX_MPU_UPD                 (1)
/* MPU definitions */
/* Extract address for/from MPU */
#define RME_CMX_MPU_ADDR(X)             ((X)&0xFFFFFFE0)
/* Get info from MPU */
#define RME_CMX_MPU_SZORD(X)            ((((X)&0x3F)>>1)-2)
/* Write info to MPU */
#define RME_CMX_MPU_VALID               (1<<4)
#define RME_CMX_MPU_SRDCLR              (0x0000FF00)
#define RME_CMX_MPU_XN                  (1<<28)
#define RME_CMX_MPU_RO                  (2<<24)
#define RME_CMX_MPU_RW                  (3<<24)
#define RME_CMX_MPU_CACHEABLE           (1<<17)
#define RME_CMX_MPU_BUFFERABLE          (1<<16)
#define RME_CMX_MPU_REGIONSIZE(X)       ((X+2)<<1)
#define RME_CMX_MPU_SZENABLE            (1)
/* Cortex-M (ARMv8) EXC_RETURN values */
#define RME_CMX_EXC_RET_BASE            (0xFFFFFF80)
/* Whether we are returning to secure stack. 1 means yes, 0 means no */
#define RME_CMX_EXC_RET_SECURE_STACK    (1<<6)
/* Whether the callee registers are automatically pushed to user stack. 1 means yes, 0 means no */
#define RME_CMX_EXC_RET_CALLEE_SAVE     (1<<5)
/* Whether the stack frame is standard(contains no FPU data). 1 means yes, 0 means no */
#define RME_CMX_EXC_RET_STD_FRAME       (1<<4)
/* Are we returning to user mode? 1 means yes, 0 means no */
#define RME_CMX_EXC_RET_RET_USER        (1<<3)
/* Are we returning to PSP? 1 means yes, 0 means no */
#define RME_CMX_EXC_RET_RET_PSP         (1<<2)
/* Is this interrupt taken to a secured domain? 1 means yes, 0 means no */
#define RME_CMX_EXC_INT_SECURE_DOMAIN   (1<<0)
/* FPU type definitions */
#define RME_CMX_FPU_NONE                (0)
#define RME_CMX_FPU_VFPV4               (1)
#define RME_CMX_FPU_FPV5_SP             (2)
#define RME_CMX_FPU_FPV5_DP             (3)

/* Some useful SCB definitions */
#define RME_CMX_SHCSR_USGFAULTENA       (1<<18)
#define RME_CMX_SHCSR_BUSFAULTENA       (1<<17)
#define RME_CMX_SHCSR_MEMFAULTENA       (1<<16)
/* MPU definitions */
#define RME_CMX_MPU_PRIVDEF             0x00000004
/* NVIC definitions */
#define RME_CMX_NVIC_GROUPING_P7S1      0
#define RME_CMX_NVIC_GROUPING_P6S2      1
#define RME_CMX_NVIC_GROUPING_P5S3      2
#define RME_CMX_NVIC_GROUPING_P4S4      3
#define RME_CMX_NVIC_GROUPING_P3S5      4
#define RME_CMX_NVIC_GROUPING_P2S6      5
#define RME_CMX_NVIC_GROUPING_P1S7      6
#define RME_CMX_NVIC_GROUPING_P0S8      7
/* Fault definitions */
/* The NMI is active */
#define RME_CMX_ICSR_NMIPENDSET         (((ptr_t)1)<<31)
/* Debug event has occurred. The Debug Fault Status Register has been updated */
#define RME_CMX_HFSR_DEBUGEVT           (((ptr_t)1)<<31)
/* Processor has escalated a configurable-priority exception to HardFault */
#define RME_CMX_HFSR_FORCED             (1<<30)
/* Vector table read fault has occurred */
#define RME_CMX_HFSR_VECTTBL            (1<<1)
/* Divide by zero */
#define RME_CMX_UFSR_DIVBYZERO          (1<<25)
/* Unaligned load/store access */
#define RME_CMX_UFSR_UNALIGNED          (1<<24)
/* No such coprocessor */
#define RME_CMX_UFSR_NOCP               (1<<19)
/* Invalid vector return LR or PC value */
#define RME_CMX_UFSR_INVPC              (1<<18)
/* Invalid IT instruction or related instructions */
#define RME_CMX_UFSR_INVSTATE           (1<<17)
/* Invalid IT instruction or related instructions */
#define RME_CMX_UFSR_UNDEFINSTR         (1<<16)
/* The Bus Fault Address Register is valid */
#define RME_CMX_BFSR_BFARVALID          (1<<15)
/* The bus fault happened during FP lazy stacking */
#define RME_CMX_BFSR_LSPERR             (1<<13)
/* A derived bus fault has occurred on exception entry */
#define RME_CMX_BFSR_STKERR             (1<<12)
/* A derived bus fault has occurred on exception return */
#define RME_CMX_BFSR_UNSTKERR           (1<<11)
/* Imprecise data access error has occurred */
#define RME_CMX_BFSR_IMPRECISERR        (1<<10)
/* A precise data access error has occurred, and the processor 
 * has written the faulting address to the BFAR */
#define RME_CMX_BFSR_PRECISERR          (1<<9)
/* A bus fault on an instruction prefetch has occurred. The 
 * fault is signaled only if the instruction is issued */
#define RME_CMX_BFSR_IBUSERR            (1<<8)
/* The Memory Mnagement Fault Address Register have valid contents */
#define RME_CMX_MFSR_MMARVALID          (1<<7)
/* A MemManage fault occurred during FP lazy state preservation */
#define RME_CMX_MFSR_MLSPERR            (1<<5)
/* A derived MemManage fault occurred on exception entry */
#define RME_CMX_MFSR_MSTKERR            (1<<4)
/* A derived MemManage fault occurred on exception return */
#define RME_CMX_MFSR_MUNSTKERR          (1<<3)
/* Data access violation. The MMFAR shows the data address that
 * the load or store tried to access */
#define RME_CMX_MFSR_DACCVIOL           (1<<1)
/* MPU or Execute Never (XN) default memory map access violation on an
 * instruction fetch has occurred. The fault is signalled only if the
 * instruction is issued */
#define RME_CMX_MFSR_IACCVIOL           (1<<0)

/* These faults cannot be recovered and will lead to termination immediately */
#define RME_CMX_FAULT_FATAL             (RME_CMX_UFSR_DIVBYZERO|RME_CMX_UFSR_UNALIGNED| \
                                         RME_CMX_UFSR_NOCP|RME_CMX_UFSR_INVPC| \
                                         RME_CMX_UFSR_INVSTATE|RME_CMX_UFSR_UNDEFINSTR| \
                                         RME_CMX_BFSR_LSPERR|RME_CMX_BFSR_STKERR| \
                                         RME_CMX_BFSR_UNSTKERR|RME_CMX_BFSR_IMPRECISERR| \
                                         RME_CMX_BFSR_PRECISERR|RME_CMX_BFSR_IBUSERR)
/*****************************************************************************/
/* __PLATFORM_CMX_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __PLATFORM_CMX_H_STRUCTS__
#define __PLATFORM_CMX_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* The register set struct - R0-R3, R12, PC, LR, xPSR is automatically pushed.
 * Here we need LR to decide EXC_RETURN, that's why it is here */
struct RME_Reg_Struct
{
    ptr_t SP;
    ptr_t R4;
    ptr_t R5;
    ptr_t R6;
    ptr_t R7;
    ptr_t R8;
    ptr_t R9;
    ptr_t R10;
    ptr_t R11;
    ptr_t LR;
};

/* The coprocessor register set structure. In Cortex-M, if there is a 
 * single-precision FPU, then the FPU S0-S15 is automatically pushed */
struct RME_Cop_Struct
{
    ptr_t S16;
    ptr_t S17;
    ptr_t S18;
    ptr_t S19;
    ptr_t S20;
    ptr_t S21;
    ptr_t S22;
    ptr_t S23;
    ptr_t S24;
    ptr_t S25;
    ptr_t S26;
    ptr_t S27;
    ptr_t S28;
    ptr_t S29;
    ptr_t S30;
    ptr_t S31;
};

struct __RME_CMX_MPU_Entry
{
    ptr_t MPU_RBAR;
    ptr_t MPU_RASR;
};

struct __RME_CMX_Pgtbl_Meta
{
    /* The MPU setting is always in the top level. This is a pointer to the top level */
    ptr_t Toplevel;
    /* The start mapping address of this page table */
    ptr_t Start_Addr;
    /* The size/num order of this level */
    ptr_t Size_Num_Order;
    /* The child directory/page number in this level */
    ptr_t Dir_Page_Count;
    /* The page flags at this level. If any pages are mapped in, it must conform
     * to the same attributes as the older pages */
    ptr_t Page_Flags;
};

struct __RME_CMX_MPU_Data
{
    /* [31:16] Static [15:0] Present */
    ptr_t State;
    struct __RME_CMX_MPU_Entry Data[8];
};

/* Interrupt flags - this type of flags will only appear on MPU-based systems */
struct __RME_CMX_Flag_Set
{
    ptr_t Lock;
    ptr_t Group;
    ptr_t Flags[32];
};

struct __RME_CMX_Flags
{
    struct __RME_CMX_Flag_Set Set0;
    struct __RME_CMX_Flag_Set Set1;
};
/*****************************************************************************/
/* __PLATFORM_CMX_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __PLATFORM_CMX_MEMBERS__
#define __PLATFORM_CMX_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/

/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/*****************************************************************************/
static ptr_t ___RME_Pgtbl_MPU_Gen_RASR(ptr_t* Table, ptr_t Flags, ptr_t Entry_Size_Order);
static ptr_t ___RME_Pgtbl_MPU_Clear(struct __RME_CMX_MPU_Data* Top_MPU, 
                                    ptr_t Start_Addr, ptr_t Size_Order);
static ptr_t ___RME_Pgtbl_MPU_Add(struct __RME_CMX_MPU_Data* Top_MPU, 
                                  ptr_t Start_Addr, ptr_t Size_Order,
                                  ptr_t MPU_RASR, ptr_t Static_Flag);
static ptr_t ___RME_Pgtbl_MPU_Update(struct __RME_CMX_Pgtbl_Meta* Meta, ptr_t Op_Flag);
/*****************************************************************************/
#define __EXTERN__
/* End Private C Function Prototypes *****************************************/

/* Public Global Variables ***************************************************/
/* __HDR_PUBLIC_MEMBERS__ */
#else
#define __EXTERN__ EXTERN 
/* __HDR_PUBLIC_MEMBERS__ */
#endif

/*****************************************************************************/

/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
/* Interrupts */
EXTERN void __RME_Disable_Int(void);
EXTERN void __RME_Enable_Int(void);
EXTERN void __RME_CMX_WFI(void);
/* Atomics */
__EXTERN__ ptr_t __RME_Comp_Swap(ptr_t* Ptr, ptr_t* Old, ptr_t New);
__EXTERN__ ptr_t __RME_Fetch_Add(ptr_t* Ptr, cnt_t Addend);
__EXTERN__ ptr_t __RME_Fetch_And(ptr_t* Ptr, ptr_t Operand);
/* MSB counting */
EXTERN ptr_t __RME_MSB_Get(ptr_t Val);
/* Debugging */
__EXTERN__ ptr_t __RME_Putchar(char Char);
/* Coprocessor */
EXTERN void ___RME_CMX_Thd_Cop_Save(struct RME_Cop_Struct* Cop_Reg);
EXTERN void ___RME_CMX_Thd_Cop_Restore(struct RME_Cop_Struct* Cop_Reg);
/* Booting */
EXTERN void _RME_Kmain(ptr_t Stack);
EXTERN void __RME_Enter_User_Mode(ptr_t Entry_Addr, ptr_t Stack_Addr);
__EXTERN__ ptr_t __RME_Low_Level_Init(void);
__EXTERN__ ptr_t __RME_Boot(void);
__EXTERN__ void __RME_Reboot(void);
__EXTERN__ void __RME_Shutdown(void);
/* Syscall & invocation */
__EXTERN__ ptr_t __RME_CPUID_Get(void);
__EXTERN__ ptr_t __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, ptr_t* Svc,
                                         ptr_t* Capid, ptr_t* Param);
__EXTERN__ ptr_t __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, ret_t Retval);
__EXTERN__ ptr_t __RME_Get_Inv_Retval(struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, ret_t Retval);
/* Thread register sets */
__EXTERN__ ptr_t __RME_Thd_Reg_Init(ptr_t Entry, ptr_t Stack, struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src);
__EXTERN__ ptr_t __RME_Thd_Cop_Init(ptr_t Entry, ptr_t Stack, struct RME_Cop_Struct* Cop_Reg);
__EXTERN__ ptr_t __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
__EXTERN__ ptr_t __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg);
/* Invocation register sets */
__EXTERN__ ptr_t __RME_Inv_Reg_Init(ptr_t Param, struct RME_Reg_Struct* Reg);
__EXTERN__ ptr_t __RME_Inv_Cop_Init(ptr_t Param, struct RME_Cop_Struct* Cop_Reg);
/* Kernel function handler */
__EXTERN__ ptr_t __RME_Kern_Func_Handler(struct RME_Reg_Struct* Reg, ptr_t Func_ID, 
                                         ptr_t Param1, ptr_t Param2);
/* Fault handler */
__EXTERN__ void __RME_CMX_Fault_Handler(struct RME_Reg_Struct* Reg);
/* Generic interrupt handler */
__EXTERN__ void __RME_CMX_Generic_Handler(struct RME_Reg_Struct* Reg, ptr_t Int_Num);
/* Page table operations */
EXTERN void ___RME_CMX_MPU_Set(ptr_t MPU_Meta);
__EXTERN__ void __RME_Pgtbl_Set(ptr_t Pgtbl);
__EXTERN__ ptr_t __RME_Pgtbl_Kmem_Init(void);
__EXTERN__ ptr_t __RME_Pgtbl_Check(ptr_t Start_Addr, ptr_t Top_Flag, ptr_t Size_Order, ptr_t Num_Order);
__EXTERN__ ptr_t __RME_Pgtbl_Init(struct RME_Cap_Pgtbl* Pgtbl_Op);
__EXTERN__ ptr_t __RME_Pgtbl_Del_Check(struct RME_Cap_Pgtbl* Pgtbl_Op);
__EXTERN__ ptr_t __RME_Pgtbl_Page_Map(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Paddr, ptr_t Pos, ptr_t Flags);
__EXTERN__ ptr_t __RME_Pgtbl_Page_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos);
__EXTERN__ ptr_t __RME_Pgtbl_Pgdir_Map(struct RME_Cap_Pgtbl* Pgtbl_Parent, ptr_t Pos, 
                                       struct RME_Cap_Pgtbl* Pgtbl_Child);
__EXTERN__ ptr_t __RME_Pgtbl_Pgdir_Unmap(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos);
__EXTERN__ ptr_t __RME_Pgtbl_Lookup(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Pos, ptr_t* Paddr, ptr_t* Flags);
__EXTERN__ ptr_t __RME_Pgtbl_Walk(struct RME_Cap_Pgtbl* Pgtbl_Op, ptr_t Vaddr, ptr_t* Pgtbl,
                                  ptr_t* Map_Vaddr, ptr_t* Paddr, ptr_t* Size_Order, ptr_t* Num_Order, ptr_t* Flags);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __PLATFORM_CMX_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
