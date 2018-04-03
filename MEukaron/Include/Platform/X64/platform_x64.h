/******************************************************************************
Filename   : platform_x64.h
Author     : pry
Date       : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description: The header of "platform_x64.c".
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __PLATFORM_X64_H_DEFS__
#define __PLATFORM_X64_H_DEFS__
/*****************************************************************************/
/* Basic Types ***************************************************************/
#if(DEFINE_BASIC_TYPES==TRUE)

#ifndef __S64__
#define __S64__
typedef signed long long s64;
#endif

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

#ifndef __U64__
#define __U64__
typedef unsigned long long u64;
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
typedef s64 tid_t;
#endif

#ifndef __PTR_T__
#define __PTR_T__
/* The typedef for the pointers - This is the raw style. Pointers must be unsigned */
typedef u64 ptr_t;
#endif

#ifndef __CNT_T__
#define __CNT_T__
/* The typedef for the count variables */
typedef s64 cnt_t;
#endif

#ifndef __CID_T__
#define __CID_T__
/* The typedef for capability ID */
typedef s64 cid_t;
#endif

#ifndef __RET_T__
#define __RET_T__
/* The type for process return value */
typedef s64 ret_t;
#endif
/* End Extended Types ********************************************************/

/* System macros *************************************************************/
/* Compiler "extern" keyword setting */
#define EXTERN                  extern
/* Compiler "inline" keyword setting */
#define INLINE                  inline
/* Number of CPUs in the system - max. 16384 ones are supported */
#define RME_CPU_NUM             16384
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER          6
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA           (RME_FALSE)
/* Quiescence timeslice value - always 10 slices, roughly equivalent to 100ms */
#define RME_QUIE_TIME           10
/* Normal page directory size calculation macro */
#define RME_PGTBL_SIZE_NOM(NUM_ORDER)   ((1<<(NUM_ORDER))*sizeof(ptr_t))
/* Top-level page directory size calculation macro */
#define RME_PGTBL_SIZE_TOP(NUM_ORDER)   RME_PGTBL_SIZE_NOM(NUM_ORDER)
/* Kernel stack size and address */
#define RME_KMEM_STACK_ADDR     0xFFFFFFFF80010000ULL

/* The CPU and application specific macros are here */
#include "platform_x64_conf.h"
/* End System macros *********************************************************/

/* X64 specific macros *******************************************************/
/* Initial boot capabilities */
/* The capability table of the init process */
#define RME_BOOT_CAPTBL                      0
/* The top-level page table of the init process - always 4GB full range split into 8 pages */
#define RME_BOOT_PGTBL                       1
/* The init process */
#define RME_BOOT_INIT_PROC                   2
/* The init thread - this is a pointer to a per-core captbl array */
#define RME_BOOT_INIT_THD                    3
/* The initial kernel function capability */
#define RME_BOOT_INIT_KERN                   4
/* The initial kernel memory capability */
#define RME_BOOT_INIT_KMEM                   5
/* The initial timer endpoint - this is a pointer to a per-core array */
#define RME_BOOT_INIT_TIMER                  6
/* The initial fault endpoint - this is a pointer to a per-core array */
#define RME_BOOT_INIT_FAULT                  7
/* The initial default endpoint for all other interrupts - this is a pointer to a per-core array */
#define RME_BOOT_INIT_INT                    8

/* Booting capability layout */
#define RME_X64_CPT              ((struct RME_Cap_Captbl*)(RME_KMEM_VA_START))
/* SRAM base */
#define RME_X64_SRAM_BASE        0x20000000
/* For x64:
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
#define RME_X64_PGTBL_TBL_NOM(X)        ((X)+(sizeof(struct __RME_X64_Pgtbl_Meta)))
#define RME_X64_PGTBL_TBL_TOP(X)        RME_X64_PGTBL_TBL_NOM(X)

/* Page entry bit definitions */
/* No execution */
#define RME_X64_MMU_NX                  (((ptr_t)1)<<63)
/* Present */
#define RME_X64_MMU_P                   (((ptr_t)1)<<0)
/* Is it read-only or writable? */
#define RME_X64_MMU_RW                  (((ptr_t)1)<<1)
/* Is it user or system? */
#define RME_X64_MMU_US                  (((ptr_t)1)<<2)
/* Is it write-through? */
#define RME_X64_MMU_PWT                 (((ptr_t)1)<<3)
/* Can we cache it? */
#define RME_X64_MMU_PCD                 (((ptr_t)1)<<4)
/* Is this accessed? */
#define RME_X64_MMU_A                   (((ptr_t)1)<<5)
/* Is this page dirty? */
#define RME_X64_MMU_D                   (((ptr_t)1)<<6)
/* The page-attribute table bit for 4k pages */
#define RME_X64_MMU_PTE_PAT             (((ptr_t)1)<<7)
/* The super-page bit */
#define RME_X64_MMU_PDE_SUP             (((ptr_t)1)<<7)
/* The global page bit - use this for all kernel memories */
#define RME_X64_MMU_G                   (((ptr_t)1)<<8)
/* The page-attribute table bit for other page sizes */
#define RME_X64_MMU_PDE_PAT             (((ptr_t)1)<<12)
/* The generic address mask */
#define RME_X64_MMU_ADDR(X)             ((X)&0x8FFFFFFFFF000)

/* MMU definitions */
/* Write info to MMU */
#define RME_X64_CR3_PCD                 (1<<4)
#define RME_X64_CR3_PWT                 (1<<3)

/* Cortex-M (ARMv8) EXC_RETURN values */
#define RME_X64_EXC_RET_BASE            (0xFFFFFF80)
/* Whether we are returning to secure stack. 1 means yes, 0 means no */
#define RME_X64_EXC_RET_SECURE_STACK    (1<<6)
/* Whether the callee registers are automatically pushed to user stack. 1 means yes, 0 means no */
#define RME_X64_EXC_RET_CALLEE_SAVE     (1<<5)
/* Whether the stack frame is standard(contains no FPU data). 1 means yes, 0 means no */
#define RME_X64_EXC_RET_STD_FRAME       (1<<4)
/* Are we returning to user mode? 1 means yes, 0 means no */
#define RME_X64_EXC_RET_RET_USER        (1<<3)
/* Are we returning to PSP? 1 means yes, 0 means no */
#define RME_X64_EXC_RET_RET_PSP         (1<<2)
/* Is this interrupt taken to a secured domain? 1 means yes, 0 means no */
#define RME_X64_EXC_INT_SECURE_DOMAIN   (1<<0)
/* FPU type definitions */
#define RME_X64_FPU_NONE                (0)
#define RME_X64_FPU_VFPV4               (1)
#define RME_X64_FPU_FPV5_SP             (2)
#define RME_X64_FPU_FPV5_DP             (3)

/* Some useful SCB definitions */
#define RME_X64_SHCSR_USGFAULTENA       (1<<18)
#define RME_X64_SHCSR_BUSFAULTENA       (1<<17)
#define RME_X64_SHCSR_MEMFAULTENA       (1<<16)
/* MPU definitions */
#define RME_X64_MPU_PRIVDEF             0x00000004
/* NVIC definitions */
#define RME_X64_NVIC_GROUPING_P7S1      0
#define RME_X64_NVIC_GROUPING_P6S2      1
#define RME_X64_NVIC_GROUPING_P5S3      2
#define RME_X64_NVIC_GROUPING_P4S4      3
#define RME_X64_NVIC_GROUPING_P3S5      4
#define RME_X64_NVIC_GROUPING_P2S6      5
#define RME_X64_NVIC_GROUPING_P1S7      6
#define RME_X64_NVIC_GROUPING_P0S8      7
/* Fault definitions */
/* The NMI is active */
#define RME_X64_ICSR_NMIPENDSET         (((ptr_t)1)<<31)
/* Debug event has occurred. The Debug Fault Status Register has been updated */
#define RME_X64_HFSR_DEBUGEVT           (((ptr_t)1)<<31)
/* Processor has escalated a configurable-priority exception to HardFault */
#define RME_X64_HFSR_FORCED             (1<<30)
/* Vector table read fault has occurred */
#define RME_X64_HFSR_VECTTBL            (1<<1)
/* Divide by zero */
#define RME_X64_UFSR_DIVBYZERO          (1<<25)
/* Unaligned load/store access */
#define RME_X64_UFSR_UNALIGNED          (1<<24)
/* No such coprocessor */
#define RME_X64_UFSR_NOCP               (1<<19)
/* Invalid vector return LR or PC value */
#define RME_X64_UFSR_INVPC              (1<<18)
/* Invalid IT instruction or related instructions */
#define RME_X64_UFSR_INVSTATE           (1<<17)
/* Invalid IT instruction or related instructions */
#define RME_X64_UFSR_UNDEFINSTR         (1<<16)
/* The Bus Fault Address Register is valid */
#define RME_X64_BFSR_BFARVALID          (1<<15)
/* The bus fault happened during FP lazy stacking */
#define RME_X64_BFSR_LSPERR             (1<<13)
/* A derived bus fault has occurred on exception entry */
#define RME_X64_BFSR_STKERR             (1<<12)
/* A derived bus fault has occurred on exception return */
#define RME_X64_BFSR_UNSTKERR           (1<<11)
/* Imprecise data access error has occurred */
#define RME_X64_BFSR_IMPRECISERR        (1<<10)
/* A precise data access error has occurred, and the processor 
 * has written the faulting address to the BFAR */
#define RME_X64_BFSR_PRECISERR          (1<<9)
/* A bus fault on an instruction prefetch has occurred. The 
 * fault is signaled only if the instruction is issued */
#define RME_X64_BFSR_IBUSERR            (1<<8)
/* The Memory Mnagement Fault Address Register have valid contents */
#define RME_X64_MFSR_MMARVALID          (1<<7)
/* A MemManage fault occurred during FP lazy state preservation */
#define RME_X64_MFSR_MLSPERR            (1<<5)
/* A derived MemManage fault occurred on exception entry */
#define RME_X64_MFSR_MSTKERR            (1<<4)
/* A derived MemManage fault occurred on exception return */
#define RME_X64_MFSR_MUNSTKERR          (1<<3)
/* Data access violation. The MMFAR shows the data address that
 * the load or store tried to access */
#define RME_X64_MFSR_DACCVIOL           (1<<1)
/* MPU or Execute Never (XN) default memory map access violation on an
 * instruction fetch has occurred. The fault is signalled only if the
 * instruction is issued */
#define RME_X64_MFSR_IACCVIOL           (1<<0)

/* These faults cannot be recovered and will lead to termination immediately */
#define RME_X64_FAULT_FATAL             (RME_X64_UFSR_DIVBYZERO|RME_X64_UFSR_UNALIGNED| \
                                         RME_X64_UFSR_NOCP|RME_X64_UFSR_INVPC| \
                                         RME_X64_UFSR_INVSTATE|RME_X64_UFSR_UNDEFINSTR| \
                                         RME_X64_BFSR_LSPERR|RME_X64_BFSR_STKERR| \
                                         RME_X64_BFSR_UNSTKERR|RME_X64_BFSR_IMPRECISERR| \
                                         RME_X64_BFSR_PRECISERR|RME_X64_BFSR_IBUSERR)

/* Hardware definitions */
#define RME_X64_COM1                    0x3F8
/*****************************************************************************/
/* __PLATFORM_X64_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __PLATFORM_X64_H_STRUCTS__
#define __PLATFORM_X64_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/
/* The 6 registers that are used to pass arguments are RDI, RSI, RDX, RCX, R8, R9.
 * Note that this is different from Micro$oft: M$ use RCX, RDX, R8, R9. The return
 * value is always located at RAX. */
struct RME_Reg_Struct
{
    ptr_t RAX;
    ptr_t RBX;
    ptr_t RCX;
    ptr_t RDX;
    ptr_t RSI;
    ptr_t RDI;
    ptr_t RBP;
    ptr_t RSP;
    ptr_t R8;
    ptr_t R9;
    ptr_t R10;
    ptr_t R11;
    ptr_t R12;
    ptr_t R13;
    ptr_t R14;
    ptr_t R15;
    ptr_t RFLAGS;
};

/* The coprocessor register set structure. MMX and SSE */
struct RME_Cop_Struct
{
	/* MMX registers first */
	ptr_t FPR_MMX0[2];
	ptr_t FPR_MMX1[2];
	ptr_t FPR_MMX2[2];
	ptr_t FPR_MMX3[2];
	ptr_t FPR_MMX4[2];
	ptr_t FPR_MMX5[2];
	ptr_t FPR_MMX6[2];
	ptr_t FPR_MMX7[2];
	/* SSE registers follow */
#if(RME_X64_AVX==RME_FALSE)
	ptr_t XMM0[2];
	ptr_t XMM1[2];
	ptr_t XMM2[2];
	ptr_t XMM3[2];
	ptr_t XMM4[2];
	ptr_t XMM5[2];
	ptr_t XMM6[2];
	ptr_t XMM7[2];
	ptr_t XMM8[2];
	ptr_t XMM9[2];
	ptr_t XMM10[2];
	ptr_t XMM11[2];
	ptr_t XMM12[2];
	ptr_t XMM13[2];
	ptr_t XMM14[2];
	ptr_t XMM15[2];
#elif(RME_X64_AVX==RME_AVX)
	ptr_t XYMM0[4];
	ptr_t XYMM1[4];
	ptr_t XYMM2[4];
	ptr_t XYMM3[4];
	ptr_t XYMM4[4];
	ptr_t XYMM5[4];
	ptr_t XYMM6[4];
	ptr_t XYMM7[4];
	ptr_t XYMM8[4];
	ptr_t XYMM9[4];
	ptr_t XYMM10[4];
	ptr_t XYMM11[4];
	ptr_t XYMM12[4];
	ptr_t XYMM13[4];
	ptr_t XYMM14[4];
	ptr_t XYMM15[4];
#elif(RME_X64_AVX==RME_AVX512)
	ptr_t XYZMM0[8];
	ptr_t XYZMM1[8];
	ptr_t XYZMM2[8];
	ptr_t XYZMM3[8];
	ptr_t XYZMM4[8];
	ptr_t XYZMM5[8];
	ptr_t XYZMM6[8];
	ptr_t XYZMM7[8];
	ptr_t XYZMM8[8];
	ptr_t XYZMM9[8];
	ptr_t XYZMM10[8];
	ptr_t XYZMM11[8];
	ptr_t XYZMM12[8];
	ptr_t XYZMM13[8];
	ptr_t XYZMM14[8];
	ptr_t XYZMM15[8];
	ptr_t XYZMM16[8];
	ptr_t XYZMM17[8];
	ptr_t XYZMM18[8];
	ptr_t XYZMM19[8];
	ptr_t XYZMM20[8];
	ptr_t XYZMM21[8];
	ptr_t XYZMM22[8];
	ptr_t XYZMM23[8];
	ptr_t XYZMM24[8];
	ptr_t XYZMM25[8];
	ptr_t XYZMM26[8];
	ptr_t XYZMM27[8];
	ptr_t XYZMM28[8];
	ptr_t XYZMM29[8];
	ptr_t XYZMM30[8];
	ptr_t XYZMM31[8];
#endif
};

/* Interrupt flags - this type of flags will only appear on MPU-based systems */
struct __RME_X64_Flag_Set
{
    ptr_t Lock;
    ptr_t Group;
    ptr_t Flags[32];
};

struct __RME_X64_Flags
{
    struct __RME_X64_Flag_Set Set0;
    struct __RME_X64_Flag_Set Set1;
};
/*****************************************************************************/
/* __PLATFORM_X64_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __PLATFORM_X64_MEMBERS__
#define __PLATFORM_X64_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/
static ptr_t RME_X64_UART_Present;
/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/*****************************************************************************/
static ptr_t ___RME_Pgtbl_MPU_Gen_RASR(ptr_t* Table, ptr_t Flags, ptr_t Entry_Size_Order);
static ptr_t ___RME_Pgtbl_MPU_Clear(struct __RME_X64_MPU_Data* Top_MPU,
                                    ptr_t Start_Addr, ptr_t Size_Order);
static ptr_t ___RME_Pgtbl_MPU_Add(struct __RME_X64_MPU_Data* Top_MPU,
                                  ptr_t Start_Addr, ptr_t Size_Order,
                                  ptr_t MPU_RASR, ptr_t Static_Flag);

static ptr_t ___RME_Pgtbl_MPU_Update(struct __RME_X64_Pgtbl_Meta* Meta, ptr_t Op_Flag);
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
EXTERN void __RME_X64_WFI(void);
/* Atomics */
__EXTERN__ ptr_t __RME_Comp_Swap(ptr_t* Ptr, ptr_t* Old, ptr_t New);
__EXTERN__ ptr_t __RME_Fetch_Add(ptr_t* Ptr, cnt_t Addend);
__EXTERN__ ptr_t __RME_Fetch_And(ptr_t* Ptr, ptr_t Operand);
/* MSB counting */
EXTERN ptr_t __RME_MSB_Get(ptr_t Val);
/* Debugging */
__EXTERN__ ptr_t __RME_Putchar(char Char);
/* Coprocessor */
EXTERN void ___RME_X64_Thd_Cop_Save(struct RME_Cop_Struct* Cop_Reg);
EXTERN void ___RME_X64_Thd_Cop_Restore(struct RME_Cop_Struct* Cop_Reg);
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
__EXTERN__ void __RME_X64_Fault_Handler(struct RME_Reg_Struct* Reg);
/* Generic interrupt handler */
__EXTERN__ void __RME_X64_Generic_Handler(struct RME_Reg_Struct* Reg, ptr_t Int_Num);
/* Page table operations */
EXTERN void ___RME_X64_MPU_Set(ptr_t MPU_Meta);
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

/* X64 specific */
EXTERN ptr_t __RME_X64_In(ptr_t Port);
EXTERN void __RME_X64_Out(ptr_t Port, ptr_t Data);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __PLATFORM_X64_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
