/******************************************************************************
Filename   : platform_x64.h
Author     : pry
Date       : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description: The header of "platform_x64.c".
******************************************************************************/

/* Defines *******************************************************************/
#include "multiboot.h"
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
#define EXTERN                               extern
/* Compiler "inline" keyword setting */
#define INLINE                               inline
/* Number of CPUs in the system - max. 4096 ones are supported */
#define RME_CPU_NUM                          256
/* The order of bits in one CPU machine word */
#define RME_WORD_ORDER                       6
/* Forcing VA=PA in user memory segments */
#define RME_VA_EQU_PA                        (RME_FALSE)
/* Quiescence timeslice value - always 10 slices, roughly equivalent to 100ms */
#define RME_QUIE_TIME                        10
/* Normal page directory size calculation macro */
#define RME_PGTBL_SIZE_NOM(NUM_ORDER)        ((1<<(NUM_ORDER))*sizeof(ptr_t))
/* Top-level page directory size calculation macro */
#define RME_PGTBL_SIZE_TOP(NUM_ORDER)        RME_PGTBL_SIZE_NOM(NUM_ORDER)
/* Initial stack size and address */
#define RME_KMEM_STACK_ADDR                  ((ptr_t)__RME_X64_Kern_Boot_Stack)
/* The virtual memory start address for the kernel objects */
#define RME_KMEM_VA_START                    0xFFFF800000000000ULL
/* The size of the kernel object virtual memory - dummy, we will detect the actual values */
#define RME_KMEM_SIZE                        0x1000
/* The virtual memory start address for the virtual machines - If no virtual machines is used, set to 0 */
#define RME_HYP_VA_START                     0
/* The size of the hypervisor reserved virtual memory */
#define RME_HYP_SIZE                         0
/* The kernel object allocation table address - relocated */
#define RME_KOTBL                            ((ptr_t*)0xFFFF800001000000)

/* FPU type definitions */
#define RME_X64_FPU_AVX                      (1)
#define RME_X64_FPU_AVX512                   (2)
/* The CPU and application specific macros are here */
#include "platform_x64_conf.h"
/* End System macros *********************************************************/

/* X64 specific macros *******************************************************/
/* Make 2-level capability */
#define RME_CAPID(X,Y)                       (((X)<<(sizeof(ptr_t)*2))|(Y)|RME_CAPID_2L)
/* Initial boot capabilities */
/* The capability table of the init process */
#define RME_BOOT_CAPTBL                      0
/* The top-level page table of the init process - an array */
#define RME_BOOT_TBL_PGTBL                   1
/* The init process */
#define RME_BOOT_INIT_PROC                   2
/* The init thread - this is a per-core array */
#define RME_BOOT_TBL_THD                     3
/* The initial kernel function capability */
#define RME_BOOT_INIT_KERN                   4
/* The initial kernel memory capability - this is a per-NUMA node array */
#define RME_BOOT_TBL_KMEM                    5
/* The initial timer endpoint - this is a per-core array */
#define RME_BOOT_TBL_TIMER                   6
/* The initial fault endpoint - this is a per-core array */
#define RME_BOOT_TBL_FAULT                   7
/* The initial default endpoint for all other interrupts - this is a per-core array */
#define RME_BOOT_TBL_INT                     8

/* The initial page table indices in the RME_BOOT_TBL_PGTBL */
#define RME_BOOT_PML4                        0
#define RME_BOOT_PDP(X)                      (RME_BOOT_PML4+1+(X))
#define RME_BOOT_PDE(X)                      (RME_BOOT_PDP(16)+(X))

/* Booting capability layout */
#define RME_X64_CPT                          ((struct RME_Cap_Captbl*)(RME_X64_Layout.Kmem1_Start))
/* Kernel VA mapping base address - PML5 currently unsupported */
#define RME_X64_VA_BASE                      (0xFFFF800000000000ULL)
#define RME_X64_TEXT_VA_BASE                 (0xFFFFFFFF80000000ULL)
/* The offset of kernel object table */
#define RME_X64_KOTBL_OFFSET                 (0x1000000ULL)
/* The offset of device hole */
#define RME_X64_DEVICE_OFFSET                (0xFE000000ULL)
/* Convert PA-VA and VA-PA in the first block of memory (16MB - 3.25GB)*/
#define RME_X64_PA2VA(PA)                    (((ptr_t)(PA))+RME_X64_VA_BASE)
#define RME_X64_VA2PA(VA)                    (((ptr_t)(VA))-RME_X64_VA_BASE)
/* Convert PA-VA and VA-PA in the text memory (16MB - 2GB )*/
#define RME_X64_TEXT_PA2VA(PA)               (((ptr_t)(PA))+RME_X64_TEXT_VA_BASE)
#define RME_X64_TEXT_VA2PA(VA)               (((ptr_t)(VA))-RME_X64_TEXT_VA_BASE)

/* Kernel stact size per CPU - currently set to 1MB */
#define RME_X64_KSTACK_ORDER                 (20)
/* Get the actual table positions */
#define RME_X64_PGTBL_TBL_NOM(X)             (X)
#define RME_X64_PGTBL_TBL_TOP(X)             (X)       

/* Device types */
#define RME_X64_MADT_LAPIC                   0
#define RME_X64_MADT_IOAPIC                  1
#define RME_X64_MADT_INT_SRC_OVERRIDE        2
#define RME_X64_MADT_NMI_INT_SRC             3
#define RME_X64_MADT_LAPIC_NMI               4

#define RME_X64_APIC_LAPIC_ENABLED           1

/* Page entry bit definitions */
/* No execution */
#define RME_X64_MMU_NX                       (((ptr_t)1)<<63)
/* Present */
#define RME_X64_MMU_P                        (((ptr_t)1)<<0)
/* Is it read-only or writable? */
#define RME_X64_MMU_RW                       (((ptr_t)1)<<1)
/* Is it user or system? */
#define RME_X64_MMU_US                       (((ptr_t)1)<<2)
/* Is it write-through? */
#define RME_X64_MMU_PWT                      (((ptr_t)1)<<3)
/* Can we cache it? */
#define RME_X64_MMU_PCD                      (((ptr_t)1)<<4)
/* Is this accessed? */
#define RME_X64_MMU_A                        (((ptr_t)1)<<5)
/* Is this page dirty? */
#define RME_X64_MMU_D                        (((ptr_t)1)<<6)
/* The page-attribute table bit for 4k pages */
#define RME_X64_MMU_PTE_PAT                  (((ptr_t)1)<<7)
/* The super-page bit */
#define RME_X64_MMU_PDE_SUP                  (((ptr_t)1)<<7)
/* The global page bit - use this for all kernel memories */
#define RME_X64_MMU_G                        (((ptr_t)1)<<8)
/* The page-attribute table bit for other page sizes */
#define RME_X64_MMU_PDE_PAT                  (((ptr_t)1)<<12)
/* The generic address mask */
#define RME_X64_MMU_ADDR(X)                  ((X)&0x000FFFFFFFFFF000)
/* Initial PML4 entries */
#define RME_X64_MMU_KERN_PML4                (RME_X64_MMU_P|RME_X64_MMU_RW|RME_X64_MMU_G)
/* Initial PDP entries - note that the P bit is not set */
#define RME_X64_MMU_KERN_PDP                 (RME_X64_MMU_RW|RME_X64_MMU_G)
/* Initial PDE entries */
#define RME_X64_MMU_KERN_PDE                 (RME_X64_MMU_P|RME_X64_MMU_PDE_SUP|RME_X64_MMU_RW|RME_X64_MMU_G)

/* MMU definitions */
/* Write info to MMU */
#define RME_X64_CR3_PCD                      (1<<4)
#define RME_X64_CR3_PWT                      (1<<3)

#define RME_X64_PGREG_POS(TABLE)             (((struct __RME_X64_Pgreg*)RME_X64_Layout.Pgreg_Start)[RME_X64_VA2PA(TABLE)>>RME_PGTBL_SIZE_4K])

/* Aggregate the X64 flags and prepare for translation - NX, PCD, PWT, RW */
#define RME_X64_PGFLG_RME2NAT(FLAGS)         (RME_X64_Pgflg_RME2NAT[(FLAGS)&(~RME_PGTBL_STATIC)])
#define RME_X64_PGFLG_NAT2RME(FLAGS)         (RME_X64_Pgflg_NAT2RME[(((FLAGS)>>63)<<3)|(((FLAGS)&0x18)>>2)|(((FLAGS)&0x02)>>1)])

/* Hardware port definitions */
#define RME_X64_COM1                         (0x3F8)
#define RME_X64_PIT_CH0                      (0x40)
#define RME_X64_PIT_CH1                      (0x41)
#define RME_X64_PIT_CH2                      (0x42)
#define RME_X64_PIT_CMD                      (0x43)
#define RME_X64_RTC_CMD                      (0x70)
#define RME_X64_RTC_DATA                     (0x71)
#define RME_X64_PIC1                         (0x20)
#define RME_X64_PIC2                         (0xA0)

/* CPUID feature tables */
/* Vendor ID/highest function parameter */
#define RME_X64_CPUID_0_VENDOR_ID            (0x0)
/* Processor info and feature bits */
#define RME_X64_CPUID_1_INFO_FEATURE         (0x1)
/* Cache and TLB descriptor information */
#define RME_X64_CPUID_2_CACHE_TLB            (0x2)
/* Processor serial number */
#define RME_X64_CPUID_3_SERIAL_NUM           (0x3)
/* Intel thread/core and cache topology 1 */
#define RME_X64_CPUID_4_INTEL_TOPO1          (0x4)
/* ECX=0, returns Intel extended features */
#define RME_X64_CPUID_7_ECX0_INTEL_EXT       (0x7)
/* Intel thread/core and cache topology 2 */
#define RME_X64_CPUID_B_INTEL_TOPO2          (0xB)

/* Get highest extenbded function supported */
#define RME_X64_CPUID_E0_EXT_MAX             (0x80000000)
/* Extended processor info and feature bits */
#define RME_X64_CPUID_E1_INFO_FEATURE        (0x80000001)
#define RME_X64_E1_EDX_FPU                   (1<<0)
#define RME_X64_E1_EDX_VME                   (1<<1)
#define RME_X64_E1_EDX_DE                    (1<<2)
#define RME_X64_E1_EDX_PSE                   (1<<3)
#define RME_X64_E1_EDX_TSC                   (1<<4)
#define RME_X64_E1_EDX_MSR                   (1<<5)
#define RME_X64_E1_EDX_PAE                   (1<<6)
#define RME_X64_E1_EDX_MCE                   (1<<7)
#define RME_X64_E1_EDX_CX8                   (1<<8)
#define RME_X64_E1_EDX_APIC                  (1<<9)
#define RME_X64_E1_EDX_SYSCALL               (1<<11)
#define RME_X64_E1_EDX_MTRR                  (1<<12)
#define RME_X64_E1_EDX_PGE                   (1<<13)
#define RME_X64_E1_EDX_MCA                   (1<<14)
#define RME_X64_E1_EDX_CMOV                  (1<<15)
#define RME_X64_E1_EDX_PAT                   (1<<16)
#define RME_X64_E1_EDX_PSE36                 (1<<17)
#define RME_X64_E1_EDX_MP                    (1<<19)
#define RME_X64_E1_EDX_NX                    (1<<20)
#define RME_X64_E1_EDX_MMXEXT                (1<<22)
#define RME_X64_E1_EDX_MMX                   (1<<23)
#define RME_X64_E1_EDX_FXSR                  (1<<24)
#define RME_X64_E1_EDX_FXSR_OPT              (1<<25)
#define RME_X64_E1_EDX_PDPE1GB               (1<<26)
#define RME_X64_E1_EDX_RDTSCP                (1<<27)
#define RME_X64_E1_EDX_LM                    (1<<29)
#define RME_X64_E1_EDX_3DNOWEXT              (1<<30)
#define RME_X64_E1_EDX_3DNOW                 (1<<31)

/* Processor brand string 1 */
#define RME_X64_CPUID_E2_BRAND1              (0x80000002)
/* Processor brand string 2 */
#define RME_X64_CPUID_E3_BRAND2              (0x80000003)
/* Processor brand string 3 */
#define RME_X64_CPUID_E4_BRAND3              (0x80000004)
/* L1 cache and TLB identifiers */
#define RME_X64_CPUID_E5_L1_TLB              (0x80000005)
/* Extended L2 cache features */
#define RME_X64_CPUID_E6_L2                  (0x80000006)
/* Advanced power management information */
#define RME_X64_CPUID_E7_APMI                (0x80000007)
/* Virtual and physical address sizes */
#define RME_X64_CPUID_E8_VA_PA_SIZE          (0x80000008)
/* AMD Easter egg - IT'S HAMMER TIME */
#define RME_X64_CPUID_EX_AMD_EASTER          (0x8FFFFFFF)

/* Feature detection macros */
#define RME_X64_FUNC(FUNC,REG)               (RME_X64_Feature.Func[FUNC][REG])
#define RME_X64_EXT(EXT,REG)                 (RME_X64_Feature.Ext[(EXT)-RME_X64_CPUID_E0_EXT_MAX][REG])

/* Vector and trap types - for trap we use vector type but allow DPL3 */
#define RME_X64_IDT_VECT                     (0x8E)
#define RME_X64_IDT_TRAP                     (0xEE)

/* Set up a normal interrupt/trap gate descriptor. 8 is the GDT CS offset */
#define RME_X64_SET_IDT(TABLE, VECT, TYPE_ATTR, ADDR) \
do \
{ \
    (TABLE)[VECT].Offset1=((ptr_t)(ADDR))&0xFFFF; \
    (TABLE)[VECT].Selector=8; \
    (TABLE)[VECT].IST_Offset=0; \
    (TABLE)[VECT].Type_Attr=(TYPE_ATTR); \
    (TABLE)[VECT].Offset2=(((ptr_t)(ADDR))>>16)&0xFFFF; \
    (TABLE)[VECT].Offset3=((ptr_t)(ADDR))>>32; \
    (TABLE)[VECT].Zero=0; \
} \
while(0)

#define RME_X64_USER_IDT(TABLE, VECT) \
do \
{ \
	RME_X64_SET_IDT(TABLE, VECT, RME_X64_IDT_VECT, __RME_X64_USER##VECT##_Handler); \
} \
while(0)

/* Interrupt vector numbers */
/* Divide error */
#define RME_X64_FAULT_DE                     (0)
/* Debug exception */
#define RME_X64_TRAP_DB                      (1)
/* NMI error */
#define RME_X64_INT_NMI                      (2)
/* Debug breakpoint */
#define RME_X64_TRAP_BP                      (3)
/* Overflow exception */
#define RME_X64_TRAP_OF                      (4)
/* Bound range exception */
#define RME_X64_FAULT_BR                     (5)
/* Undefined instruction */
#define RME_X64_FAULT_UD                     (6)
/* Device not available */
#define RME_X64_FAULT_NM                     (7)
/* Double(nested) fault exception */
#define RME_X64_ABORT_DF                     (8)
/* Coprocessor overrun - not used later on */
#define RME_X64_ABORT_OLD_MF                 (9)
/* Invalid TSS exception */
#define RME_X64_FAULT_TS                     (10)
/* Segment not present */
#define RME_X64_FAULT_NP                     (11)
/* Stack fault exception */
#define RME_X64_FAULT_SS                     (12)
/* General protection exception */
#define RME_X64_FAULT_GP                     (13)
/* Page fault exception */
#define RME_X64_FAULT_PF                     (14)
/* Number 15 reserved */
/* X87 FPU floating-point error */
#define RME_X64_FAULT_MF                     (16)
/* Alignment check exception */
#define RME_X64_FAULT_AC                     (17)
/* Machine check exception */
#define RME_X64_ABORT_MC                     (18)
/* SIMD floating-point exception */
#define RME_X64_FAULT_XM                     (19)
/* Virtualization exception */
#define RME_X64_FAULT_VE                     (20)
/* User interrupts */
#define RME_X64_INT_USER(INT)                ((INT)+32)
#define RME_X64_INT_SYSTICK                  RME_X64_INT_USER(2)

/* User interrupts that are used by RME - map these two even further away */
#define RME_X64_INT_SPUR                     RME_X64_INT_USER(0x80-32)
#define RME_X64_INT_ERROR                    RME_X64_INT_USER(0x81-32)
#define RME_X64_INT_IPI                      RME_X64_INT_USER(0x82-32)
#define RME_X64_INT_SMP_SYSTICK              RME_X64_INT_USER(0x83-32)

/* LAPIC offsets - maybe we should use structs later on */
#define RME_X64_LAPIC_ID                     (0x0020/4)
#define RME_X64_LAPIC_VER                    (0x0030/4)
#define RME_X64_LAPIC_TPR                    (0x0080/4)
#define RME_X64_LAPIC_EOI                    (0x00B0/4)
#define RME_X64_LAPIC_SVR                    (0x00F0/4)
#define RME_X64_LAPIC_SVR_ENABLE             (0x00000100)

#define RME_X64_LAPIC_ESR                    (0x0280/4)
#define RME_X64_LAPIC_ICRLO                  (0x0300/4)
#define RME_X64_LAPIC_ICRLO_INIT             (0x00000500)
#define RME_X64_LAPIC_ICRLO_STARTUP          (0x00000600)
#define RME_X64_LAPIC_ICRLO_DELIVS           (0x00001000)
#define RME_X64_LAPIC_ICRLO_ASSERT           (0x00004000)
#define RME_X64_LAPIC_ICRLO_DEASSERT         (0x00000000)
#define RME_X64_LAPIC_ICRLO_LEVEL            (0x00008000)
#define RME_X64_LAPIC_ICRLO_BCAST            (0x00080000)
#define RME_X64_LAPIC_ICRLO_EXC_SELF         (0x000C0000)
#define RME_X64_LAPIC_ICRLO_BUSY             (0x00001000)
#define RME_X64_LAPIC_ICRLO_FIXED            (0x00000000)


#define RME_X64_LAPIC_ICRHI                  (0x0310/4)
#define RME_X64_LAPIC_TIMER                  (0x0320/4)
#define RME_X64_LAPIC_TIMER_X1               (0x0000000B)
#define RME_X64_LAPIC_TIMER_PERIODIC         (0x00020000)

#define RME_X64_LAPIC_PCINT                  (0x0340/4)
#define RME_X64_LAPIC_LINT0                  (0x0350/4)
#define RME_X64_LAPIC_LINT1                  (0x0360/4)
#define RME_X64_LAPIC_ERROR                  (0x0370/4)
#define RME_X64_LAPIC_MASKED                 (0x00010000)

#define RME_X64_LAPIC_TICR                   (0x0380/4)
#define RME_X64_LAPIC_TCCR                   (0x0390/4)
#define RME_X64_LAPIC_TDCR                   (0x03E0/4)

/* LAPIC R/W */
#define RME_X64_LAPIC_READ(REG)              (((ptr_t*)RME_X64_LAPIC_Addr)[REG])
#define RME_X64_LAPIC_WRITE(REG,VAL) \
do \
{ \
	((u32*)RME_X64_LAPIC_Addr)[REG]=(VAL); \
	/* Dummy read to an address that we never use */ \
	*(volatile u32*)(RME_X64_PA2VA(0x100))=((u32*)RME_X64_LAPIC_Addr)[RME_X64_LAPIC_ID]; \
} \
while(0)

/* IOAPIC address - consider supporting multiple ones */
#define RME_X64_IOAPIC_ADDR                 (RME_X64_PA2VA(0xFEC00000))

/* IOAPIC registers */
#define RME_X64_IOAPIC_REG_ID               (0x00)
#define RME_X64_IOAPIC_REG_VER              (0x01)
#define RME_X64_IOAPIC_REG_TABLE            (0x10)

#define RME_X64_IOAPIC_INT_DISABLED         (0x00010000)
#define RME_X64_IOAPIC_INT_LEVEL            (0x00008000)
#define RME_X64_IOAPIC_INT_ACTIVELOW        (0x00002000)
#define RME_X64_IOAPIC_INT_LOGICAL          (0x00000800)

#define RME_X64_IOAPIC_READ(REG,DATA) \
do \
{ \
	((struct RME_X64_IOAPIC_Map*)RME_X64_IOAPIC_ADDR)->Reg=(REG); \
	(DATA)=((struct RME_X64_IOAPIC_Map*)RME_X64_IOAPIC_ADDR)->Data; \
} \
while(0)

#define RME_X64_IOAPIC_WRITE(REG,DATA) \
do \
{ \
	((struct RME_X64_IOAPIC_Map*)RME_X64_IOAPIC_ADDR)->Reg=(REG); \
	((struct RME_X64_IOAPIC_Map*)RME_X64_IOAPIC_ADDR)->Data=(DATA); \
} \
while(0)

/* Processor RFLAGS register bits */
#define RME_X64_RFLAGS_IF                  (1<<9)

/* MSR addresses */
#define RME_X64_MSR_IA32_EFER              (0xC0000080)
#define RME_X64_MSR_IA32_GS_BASE           (0xC0000101)
#define RME_X64_MSR_IA32_KERNEL_GS_BASE    (0xC0000102)
#define RME_X64_MSR_IA32_STAR              (0xC0000081)
#define RME_X64_MSR_IA32_LSTAR             (0xC0000082)
#define RME_X64_MSR_IA32_FMASK             (0xC0000084)

/* MSR bits */
#define RME_X64_MSR_IA32_EFER_SCE          (1)

/* Segment definitions */
#define RME_X64_SEG_KERNEL_CODE            (1*8)
#define RME_X64_SEG_KERNEL_DATA            (2*8)
#define RME_X64_SEG_USER_CODE              (4*8+3)
#define RME_X64_SEG_USER_DATA              (5*8+3)

/* Get kernel stack addresses */
#define RME_X64_KSTACK(CPU)                (RME_X64_Layout.Stack_Start+((1+(CPU))<<RME_X64_KSTACK_ORDER))
/* Microsecond delay function */
#define RME_X64_UDELAY(US)
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
/* Architecture-related structures - we only target GCC so attribute packed is fine */
/* Root System Description Pointer descriptor */
struct RME_X64_ACPI_RDSP_Desc
{
	u8 Signature[8];
	u8 Checksum;
	u8 OEM_ID[6];
	u8 Revision;
	u32 RSDT_Addr_Phys;

	/* These are the extended parts that we do not use now */
	u32 Length;
	u64 XSDT_Addr_Phys;
	u8 Xchecksum;
	u8 Reserved[3];
} __attribute__((__packed__));

/* General ACPI descriptor header */
struct RME_X64_ACPI_Desc_Hdr
{
	u8 Signature[4];
	u32 Length;
	u8 Revision;
	u8 Checksum;
	u8 OEM_ID[6];
	u8 OEM_Table_ID[8];
	u32 OEM_Revision;
	u8 Creator_ID[4];
	u32 Creator_Revision;
} __attribute__((__packed__));

/* Root System Description Table header */
struct RME_X64_ACPI_RSDT_Hdr
{
	struct RME_X64_ACPI_Desc_Hdr Header;
    /* This is fine; GCC can take this */
	u32 Entry[0];
} __attribute__((__packed__));

/* Multiple APIC Description Table header */
struct RME_X64_ACPI_MADT_Hdr
{
	struct RME_X64_ACPI_Desc_Hdr Header;
	u32 LAPIC_Addr_Phys;
	u32 Flags;
    /* This is fine; GCC can take this */
	u8 Table[0];
} __attribute__((__packed__));

/* MADT's LAPIC record */
struct RME_X64_ACPI_MADT_LAPIC_Record
{
	u8 Type;
	u8 Length;
	u8 ACPI_ID;
	u8 APIC_ID;
	u32 Flags;
} __attribute__((__packed__));

/* MADT's IOAPIC record */
struct RME_X64_ACPI_MADT_IOAPIC_Record
{
	u8 Type;
	u8 Length;
	u8 ID;
	u8 Reserved;
	u32 Addr;
	u32 Interrupt_Base;
} __attribute__((__packed__));

/* MADT's interrupt source override record*/
struct RME_X64_ACPI_MADT_SRC_OVERRIDE_Record
{
	u8 Type;
	u8 Length;
    u8 Bus;
    u8 Source;
    u8 GS_Interrupt;
    u16 MPS_Int_Flags;
}  __attribute__((__packed__));

/* IDT entry */
struct RME_X64_IDT_Entry
{
	/* Offset bits 0..15 */
    u16 Offset1;
    /* A code segment selector in GDT or LDT */
	u16 Selector;
    /* Bits 0..2 holds Interrupt Stack Table offset, rest of bits zero */
    u8 IST_Offset;
    /* Type and attributes */
    u8 Type_Attr;
    /* Offset bits 16..31 */
    u16 Offset2;
    /* Offset bits 32..63 */
    u32 Offset3;
    /* Reserved, must be zero */
    u32 Zero;
} __attribute__((packed));

/* IOAPIC data structure */
struct RME_X64_IOAPIC_Map
{
    u32 Reg;
    u32 Pad[3];
    u32 Data;
};

/* CPUID data structure */
struct RME_X64_CPUID_Entry
{
	ptr_t CPUID;
	ptr_t Kernel_SP;
	ptr_t Temp_User_SP;
};

/* Per-CPU data structure */
struct RME_X64_CPU_Info
{
	/* The LAPIC ID of the CPU, used to distinguish between different CPUs */
	ptr_t LAPIC_ID;
	/* Is the booting done on this CPU? */
	volatile ptr_t Boot_Done;
};

/* Per-IOAPIC data structure */
struct RME_X64_IOAPIC_Info
{
    ptr_t IOAPIC_ID;
};

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
    ptr_t R8;
    ptr_t R9;
    ptr_t R10;
    ptr_t R11;
    ptr_t R12;
    ptr_t R13;
    ptr_t R14;
    ptr_t R15;
    /* If we went into this with a SYSCALL, this is 0x10000 */
    ptr_t INT_NUM;
    ptr_t ERROR_CODE;
    ptr_t RIP;
    ptr_t CS;
    ptr_t RFLAGS;
    ptr_t RSP;
    ptr_t SS;
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
#if(RME_X64_FPU_TYPE==RME_FALSE)
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
#elif(RME_X64_FPU_TYPE==RME_X64_FPU_AVX)
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
#elif(RME_X64_FPU_TYPE==RME_X64_FPU_AVX512)
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

/* Memory information - the layout is (offset from VA base):
 * |0--640k|----------16MB|-----|-----|------|------|-----|3.25G-4G|-----|-----|
 * |Vectors|Kernel&Globals|Kotbl|Pgreg|PerCPU|Kpgtbl|Kmem1|  Hole  |Kmem2|Stack|
 *  Vectors        : Interrupt vectors.
 *  Kernel&Globals : Initial kernel text segment and all static variables.
 *  Kotbl          : Kernel object registration table.
 *  PerCPU         : Per-CPU data structures.
 *  Kpgtbl         : Kernel page tables.
 *  Pgreg          : Page table registration table.
 *  Kmem1          : Kernel memory 1, linear mapping, allow creation of page tables.
 *  Hole           : Memory hole present at 3.25G-4G. For PCI devices.
 *  Kmem2          : Ker
 * The C snippet to generate this table is shown below (gcc x64):nel memory 2, nonlinear mapping, no page table creation allowed.
 *  Stacks         : Kernel stacks, per-CPU.
 *  All values are in bytes, and are virtual addresses.
 */
struct RME_X64_Layout
{
	ptr_t Kotbl_Start;
	ptr_t Kotbl_Size;

	ptr_t Pgreg_Start;
	ptr_t Pgreg_Size;

	ptr_t PerCPU_Start;
	ptr_t PerCPU_Size;

	ptr_t Kpgtbl_Start;
	ptr_t Kpgtbl_Size;

	ptr_t Kmem1_Start;
	ptr_t Kmem1_Size;

	ptr_t Hole_Start;
	ptr_t Hole_Size;

	ptr_t Kmem2_Start;
	ptr_t Kmem2_Size;

	ptr_t Stack_Start;
	ptr_t Stack_Size;
};

/* The processor features */
struct RME_X64_Features
{
	ptr_t Max_Func;
	ptr_t Max_Ext;
	ptr_t Func[16][4];
	ptr_t Ext[16][4];
};

/* Page table registration table */
struct __RME_X64_Pgreg
{
    /* How many child page tables does this page table have? */
    ptr_t Child_Cnt;
    /* How many parent page tables does this page table have? */
    ptr_t Parent_Cnt;
};

/* The first two levels of the kernel page table. The third level will be constructed on the fly */
struct __RME_X64_Kern_Pgtbl
{
	ptr_t Dummy[256];
	ptr_t PML4[256];
	ptr_t PDP[256][512];
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
/* Is there a UART in the system? */
static ptr_t RME_X64_UART_Exist;
/* Where is the multiboot information located? */
static struct multiboot_info* RME_X64_MBInfo;
/* The layout of the memory structure */
static struct RME_X64_Layout RME_X64_Layout;
/* We currently support 256 CPUs max */
static ptr_t RME_X64_Num_CPU;
/* CPU counter */
static ptr_t RME_X64_CPU_Cnt;
static struct RME_X64_CPU_Info RME_X64_CPU_Info[RME_CPU_NUM];
/* There can be max. 8 IOAPICs */
static ptr_t RME_X64_Num_IOAPIC;
static struct RME_X64_IOAPIC_Info RME_X64_IOAPIC_Info[8];
/* The LAPIC address */
static ptr_t RME_X64_LAPIC_Addr;
/* The processor features */
struct RME_X64_Features RME_X64_Feature;

/* Translate the flags into X64 specific ones - the STATIC bit will never be
 * set thus no need to consider about it here. The flag bits order is shown below:
 * [MSB                                                                                         LSB]
 * RME_PGTBL_BUFFERABLE | RME_PGTBL_CACHEABLE | RME_PGTBL_EXECUTE | RME_PGTBL_WRITE | RME_PGTBL_READ
 * The C snippet to generate this (gcc x64):
 */
static const ptr_t RME_X64_Pgflg_RME2NAT[32]=
{
	0x800000000000001D,0x800000000000001D,0x800000000000001F,0x800000000000001F,
	0x000000000000001D,0x000000000000001D,0x000000000000001F,0x000000000000001F,
	0x800000000000000D,0x800000000000000D,0x800000000000000F,0x800000000000000F,
	0x000000000000000D,0x000000000000000D,0x000000000000000F,0x000000000000000F,
	0x8000000000000015,0x8000000000000015,0x8000000000000017,0x8000000000000017,
	0x0000000000000015,0x0000000000000015,0x0000000000000017,0x0000000000000017,
	0x8000000000000005,0x8000000000000005,0x8000000000000007,0x8000000000000007,
	0x0000000000000005,0x0000000000000005,0x0000000000000007,0x0000000000000007
};
/* Translate the flags back to RME format. In order to use this table, it is needed to extract the
 * X64 bits: [63](NX) [4](PCD) [3](PWT) [1](RW). The C snippet to generate this (gcc x64): 

#include "stdio.h"

#define X64_NX                 (1<<3)
#define X64_PCD                (1<<2)
#define X64_PWT                (1<<1)
#define X64_RW                 (1<<0)

#define RME_READ               (1<<0)
#define RME_WRITE              (1<<1)
#define RME_EXECUTE            (1<<2)
#define RME_CACHEABLE          (1<<3)
#define RME_BUFFERABLE         (1<<4)
#define RME_STATIC             (1<<5)

int main(void)
{
    unsigned long long int flag;
    int count;
    
    for(count=0;count<16;count++)
    {
        flag=RME_READ;

        if((count&X64_NX)==0)
            flag|=RME_EXECUTE;

        if((count&X64_PCD)==0)
            flag|=RME_CACHEABLE;

        if((count&X64_PWT)==0)
            flag|=RME_BUFFERABLE;

        if((count&X64_RW)!=0)
            flag|=RME_WRITE;
    }
}
 */
static const ptr_t RME_X64_Pgflg_NAT2RME[16]=
{
	0x800000000000001D,0x800000000000001D,0x800000000000001F,0x800000000000001F,
	0x000000000000001D,0x000000000000001D,0x000000000000001F,0x000000000000001F,
	0x800000000000000D,0x800000000000000D,0x800000000000000F,0x800000000000000F,
	0x000000000000000D,0x000000000000000D,0x000000000000000F,0x000000000000000F
};

/* This boot code is the binary of the following boot.S, compiled with:
 * gcc -fno-pic -nostdinc -I. -o boot.o -c boot.S
 * ld -m elf_x86_64 -nodefaultlibs -N -e Start_16 -Ttext 0x7000 -o boot_block.o boot.o
 * objcopy -S -O binary -j .text boot_block.o boot_block.bin
 * hexdump -v  -e '8/1 "0x%02X,""\n"' < boot_block.bin > boot_block.c
 * objdump -S boot_block.o > boot_block.asm
 * The contents of boot_block.c is placed here. Load this to 0x7000 to boot different
 * processors.
                 .code16
                 .global        Start_16
Start_16:
                 CLI
                 XORW           %AX,%AX
                 MOVW           %AX,%DS
                 MOVW           %AX,%ES
                 MOVW           %AX,%SS
                 LGDT           Boot_GDT_Desc_16
                 MOV            %CR0,%EAX
                 ORL            $0x01,%EAX
                 MOVL           %EAX,%CR0
                 LJMPL          $8,$(Boot_32)
                 .code32
Boot_32:
                 MOVW           $16,%AX
                 MOVW           %AX,%DS
                 MOVW           %AX,%ES
                 MOVW           %AX,%SS
                 XORW           %AX,%AX
                 MOVW           %AX,%FS
                 MOVW           %AX,%GS
                 MOV            $1,%EBX
                 MOVL           (Start_16-4),%ESP
                 CALL           *(Start_16-8)
                 JMP            .

                 .p2align       2
Boot_GDT_16:
                 .word          0x0000,0x0000,0x0000,0x0000;
                 .word          0xFFFF,0x0000
                 .byte          0x00,0x9A,0xCF,0x00
                 .word          0xFFFF,0x0000
                 .byte          0x00,0x92,0xCF,0x00
Boot_GDT_Desc_16:
                 .word          (Boot_GDT_Desc_16-Boot_GDT_16-1)
                 .long          Boot_GDT_16
 */
static const u8 RME_X64_Boot_Code[]=
{
    0xFA,0x31,0xC0,0x8E,0xD8,0x8E,0xC0,0x8E,
    0xD0,0x0F,0x01,0x16,0x5C,0x70,0x0F,0x20,
    0xC0,0x66,0x83,0xC8,0x01,0x0F,0x22,0xC0,
    0x66,0xEA,0x20,0x70,0x00,0x00,0x08,0x00,
    0x66,0xB8,0x10,0x00,0x8E,0xD8,0x8E,0xC0,
    0x8E,0xD0,0x66,0x31,0xC0,0x8E,0xE0,0x8E,
    0xE8,0xBB,0x01,0x00,0x00,0x00,0x8B,0x25,
    0xFC,0x6F,0x00,0x00,0xFF,0x15,0xF8,0x6F,
    0x00,0x00,0xEB,0xFE,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
    0x00,0x9A,0xCF,0x00,0xFF,0xFF,0x00,0x00,
    0x00,0x92,0xCF,0x00,0x17,0x00,0x44,0x70,
    0x00,0x00
};
/*****************************************************************************/
/* End Private Global Variables **********************************************/

/* Private C Function Prototypes *********************************************/ 
/*****************************************************************************/
static void __RME_X64_UART_Init(void);
/* Find the RDSP */
static struct RME_X64_ACPI_RDSP_Desc* __RME_X64_RDSP_Scan(ptr_t Base, ptr_t Len);
static struct RME_X64_ACPI_RDSP_Desc* __RME_X64_RDSP_Find(void);
static ret_t __RME_X64_SMP_Detect(struct RME_X64_ACPI_MADT_Hdr* MADT);
/* Debug output helper */
static void __RME_X64_ACPI_Debug(struct RME_X64_ACPI_Desc_Hdr *Header);
/* Initialize the ACPI */
static ret_t __RME_X64_ACPI_Init(void);
/* Get processor feature bits */
static void __RME_X64_Feature_Get(void);
/* Initialize memory according to GRUB multiboot specification */
static void __RME_X64_Mem_Init(ptr_t MMap_Addr, ptr_t MMap_Length);
/* Initialize CPU-local tables */
static void __RME_X64_CPU_Local_Init(void);
/* Initialize interrupt controllers */
static void __RME_X64_PIC_Init(void);
static void __RME_X64_LAPIC_Init(void);
static void __RME_X64_IOAPIC_Init(void);
/* Enable/disable a vector in IOAPIC */
static void __RME_X64_IOAPIC_Int_Enable(ptr_t IRQ, ptr_t CPUID);
static void __RME_X64_IOAPIC_Int_Disable(ptr_t IRQ);
/* Initialize timers */
static void __RME_X64_Timer_Init(void);
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
EXTERN struct RME_X64_IDT_Entry RME_X64_IDT_Table[256];
EXTERN struct __RME_X64_Kern_Pgtbl RME_X64_Kpgt;
EXTERN ptr_t __RME_X64_Kern_Boot_Stack[0];
/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/
/* X64 specific */
EXTERN ptr_t __RME_X64_In(ptr_t Port);
EXTERN void __RME_X64_Out(ptr_t Port, ptr_t Data);
EXTERN ptr_t __RME_X64_Read_MSR(ptr_t MSR);
EXTERN void __RME_X64_Write_MSR(ptr_t MSR, ptr_t Value);
EXTERN void __RME_X64_GDT_Load(ptr_t* GDTR);
EXTERN void __RME_X64_IDT_Load(ptr_t* IDTR);
EXTERN void __RME_X64_TSS_Load(ptr_t TSS);
EXTERN ptr_t __RME_X64_CPUID_Get(ptr_t EAX, ptr_t* EBX, ptr_t* ECX, ptr_t* EDX);
EXTERN void __RME_X64_Pgtbl_Set(ptr_t Pgtbl);
/* Boot glue */
EXTERN void __RME_X64_SMP_Boot_32(void);
/* Vectors */
EXTERN void __RME_X64_FAULT_DE_Handler(void);
EXTERN void __RME_X64_TRAP_DB_Handler(void);
EXTERN void __RME_X64_INT_NMI_Handler(void);
EXTERN void __RME_X64_TRAP_BP_Handler(void);
EXTERN void __RME_X64_TRAP_OF_Handler(void);
EXTERN void __RME_X64_FAULT_BR_Handler(void);
EXTERN void __RME_X64_FAULT_UD_Handler(void);
EXTERN void __RME_X64_FAULT_NM_Handler(void);
EXTERN void __RME_X64_ABORT_DF_Handler(void);
EXTERN void __RME_X64_ABORT_OLD_MF_Handler(void);
EXTERN void __RME_X64_FAULT_TS_Handler(void);
EXTERN void __RME_X64_FAULT_NP_Handler(void);
EXTERN void __RME_X64_FAULT_SS_Handler(void);
EXTERN void __RME_X64_FAULT_GP_Handler(void);
EXTERN void __RME_X64_FAULT_PF_Handler(void);
EXTERN void __RME_X64_FAULT_MF_Handler(void);
EXTERN void __RME_X64_FAULT_AC_Handler(void);
EXTERN void __RME_X64_ABORT_MC_Handler(void);
EXTERN void __RME_X64_FAULT_XM_Handler(void);
EXTERN void __RME_X64_FAULT_VE_Handler(void);
/* Systick&SVC handler */
EXTERN void SysTick_Handler(void);
EXTERN void SysTick_SMP_Handler(void);
EXTERN void SVC_Handler(void);
/* User handlers */
EXTERN void __RME_X64_USER32_Handler(void);
EXTERN void __RME_X64_USER33_Handler(void);
EXTERN void __RME_X64_USER34_Handler(void);
EXTERN void __RME_X64_USER35_Handler(void);
EXTERN void __RME_X64_USER36_Handler(void);
EXTERN void __RME_X64_USER37_Handler(void);
EXTERN void __RME_X64_USER38_Handler(void);
EXTERN void __RME_X64_USER39_Handler(void);

EXTERN void __RME_X64_USER40_Handler(void);
EXTERN void __RME_X64_USER41_Handler(void);
EXTERN void __RME_X64_USER42_Handler(void);
EXTERN void __RME_X64_USER43_Handler(void);
EXTERN void __RME_X64_USER44_Handler(void);
EXTERN void __RME_X64_USER45_Handler(void);
EXTERN void __RME_X64_USER46_Handler(void);
EXTERN void __RME_X64_USER47_Handler(void);
EXTERN void __RME_X64_USER48_Handler(void);
EXTERN void __RME_X64_USER49_Handler(void);

EXTERN void __RME_X64_USER50_Handler(void);
EXTERN void __RME_X64_USER51_Handler(void);
EXTERN void __RME_X64_USER52_Handler(void);
EXTERN void __RME_X64_USER53_Handler(void);
EXTERN void __RME_X64_USER54_Handler(void);
EXTERN void __RME_X64_USER55_Handler(void);
EXTERN void __RME_X64_USER56_Handler(void);
EXTERN void __RME_X64_USER57_Handler(void);
EXTERN void __RME_X64_USER58_Handler(void);
EXTERN void __RME_X64_USER59_Handler(void);

EXTERN void __RME_X64_USER60_Handler(void);
EXTERN void __RME_X64_USER61_Handler(void);
EXTERN void __RME_X64_USER62_Handler(void);
EXTERN void __RME_X64_USER63_Handler(void);
EXTERN void __RME_X64_USER64_Handler(void);
EXTERN void __RME_X64_USER65_Handler(void);
EXTERN void __RME_X64_USER66_Handler(void);
EXTERN void __RME_X64_USER67_Handler(void);
EXTERN void __RME_X64_USER68_Handler(void);
EXTERN void __RME_X64_USER69_Handler(void);

EXTERN void __RME_X64_USER70_Handler(void);
EXTERN void __RME_X64_USER71_Handler(void);
EXTERN void __RME_X64_USER72_Handler(void);
EXTERN void __RME_X64_USER73_Handler(void);
EXTERN void __RME_X64_USER74_Handler(void);
EXTERN void __RME_X64_USER75_Handler(void);
EXTERN void __RME_X64_USER76_Handler(void);
EXTERN void __RME_X64_USER77_Handler(void);
EXTERN void __RME_X64_USER78_Handler(void);
EXTERN void __RME_X64_USER79_Handler(void);

EXTERN void __RME_X64_USER80_Handler(void);
EXTERN void __RME_X64_USER81_Handler(void);
EXTERN void __RME_X64_USER82_Handler(void);
EXTERN void __RME_X64_USER83_Handler(void);
EXTERN void __RME_X64_USER84_Handler(void);
EXTERN void __RME_X64_USER85_Handler(void);
EXTERN void __RME_X64_USER86_Handler(void);
EXTERN void __RME_X64_USER87_Handler(void);
EXTERN void __RME_X64_USER88_Handler(void);
EXTERN void __RME_X64_USER89_Handler(void);

EXTERN void __RME_X64_USER90_Handler(void);
EXTERN void __RME_X64_USER91_Handler(void);
EXTERN void __RME_X64_USER92_Handler(void);
EXTERN void __RME_X64_USER93_Handler(void);
EXTERN void __RME_X64_USER94_Handler(void);
EXTERN void __RME_X64_USER95_Handler(void);
EXTERN void __RME_X64_USER96_Handler(void);
EXTERN void __RME_X64_USER97_Handler(void);
EXTERN void __RME_X64_USER98_Handler(void);
EXTERN void __RME_X64_USER99_Handler(void);

EXTERN void __RME_X64_USER100_Handler(void);
EXTERN void __RME_X64_USER101_Handler(void);
EXTERN void __RME_X64_USER102_Handler(void);
EXTERN void __RME_X64_USER103_Handler(void);
EXTERN void __RME_X64_USER104_Handler(void);
EXTERN void __RME_X64_USER105_Handler(void);
EXTERN void __RME_X64_USER106_Handler(void);
EXTERN void __RME_X64_USER107_Handler(void);
EXTERN void __RME_X64_USER108_Handler(void);
EXTERN void __RME_X64_USER109_Handler(void);

EXTERN void __RME_X64_USER110_Handler(void);
EXTERN void __RME_X64_USER111_Handler(void);
EXTERN void __RME_X64_USER112_Handler(void);
EXTERN void __RME_X64_USER113_Handler(void);
EXTERN void __RME_X64_USER114_Handler(void);
EXTERN void __RME_X64_USER115_Handler(void);
EXTERN void __RME_X64_USER116_Handler(void);
EXTERN void __RME_X64_USER117_Handler(void);
EXTERN void __RME_X64_USER118_Handler(void);
EXTERN void __RME_X64_USER119_Handler(void);

EXTERN void __RME_X64_USER120_Handler(void);
EXTERN void __RME_X64_USER121_Handler(void);
EXTERN void __RME_X64_USER122_Handler(void);
EXTERN void __RME_X64_USER123_Handler(void);
EXTERN void __RME_X64_USER124_Handler(void);
EXTERN void __RME_X64_USER125_Handler(void);
EXTERN void __RME_X64_USER126_Handler(void);
EXTERN void __RME_X64_USER127_Handler(void);
EXTERN void __RME_X64_USER128_Handler(void);
EXTERN void __RME_X64_USER129_Handler(void);

EXTERN void __RME_X64_USER130_Handler(void);
EXTERN void __RME_X64_USER131_Handler(void);
EXTERN void __RME_X64_USER132_Handler(void);
EXTERN void __RME_X64_USER133_Handler(void);
EXTERN void __RME_X64_USER134_Handler(void);
EXTERN void __RME_X64_USER135_Handler(void);
EXTERN void __RME_X64_USER136_Handler(void);
EXTERN void __RME_X64_USER137_Handler(void);
EXTERN void __RME_X64_USER138_Handler(void);
EXTERN void __RME_X64_USER139_Handler(void);

EXTERN void __RME_X64_USER140_Handler(void);
EXTERN void __RME_X64_USER141_Handler(void);
EXTERN void __RME_X64_USER142_Handler(void);
EXTERN void __RME_X64_USER143_Handler(void);
EXTERN void __RME_X64_USER144_Handler(void);
EXTERN void __RME_X64_USER145_Handler(void);
EXTERN void __RME_X64_USER146_Handler(void);
EXTERN void __RME_X64_USER147_Handler(void);
EXTERN void __RME_X64_USER148_Handler(void);
EXTERN void __RME_X64_USER149_Handler(void);

EXTERN void __RME_X64_USER150_Handler(void);
EXTERN void __RME_X64_USER151_Handler(void);
EXTERN void __RME_X64_USER152_Handler(void);
EXTERN void __RME_X64_USER153_Handler(void);
EXTERN void __RME_X64_USER154_Handler(void);
EXTERN void __RME_X64_USER155_Handler(void);
EXTERN void __RME_X64_USER156_Handler(void);
EXTERN void __RME_X64_USER157_Handler(void);
EXTERN void __RME_X64_USER158_Handler(void);
EXTERN void __RME_X64_USER159_Handler(void);

EXTERN void __RME_X64_USER160_Handler(void);
EXTERN void __RME_X64_USER161_Handler(void);
EXTERN void __RME_X64_USER162_Handler(void);
EXTERN void __RME_X64_USER163_Handler(void);
EXTERN void __RME_X64_USER164_Handler(void);
EXTERN void __RME_X64_USER165_Handler(void);
EXTERN void __RME_X64_USER166_Handler(void);
EXTERN void __RME_X64_USER167_Handler(void);
EXTERN void __RME_X64_USER168_Handler(void);
EXTERN void __RME_X64_USER169_Handler(void);

EXTERN void __RME_X64_USER170_Handler(void);
EXTERN void __RME_X64_USER171_Handler(void);
EXTERN void __RME_X64_USER172_Handler(void);
EXTERN void __RME_X64_USER173_Handler(void);
EXTERN void __RME_X64_USER174_Handler(void);
EXTERN void __RME_X64_USER175_Handler(void);
EXTERN void __RME_X64_USER176_Handler(void);
EXTERN void __RME_X64_USER177_Handler(void);
EXTERN void __RME_X64_USER178_Handler(void);
EXTERN void __RME_X64_USER179_Handler(void);

EXTERN void __RME_X64_USER180_Handler(void);
EXTERN void __RME_X64_USER181_Handler(void);
EXTERN void __RME_X64_USER182_Handler(void);
EXTERN void __RME_X64_USER183_Handler(void);
EXTERN void __RME_X64_USER184_Handler(void);
EXTERN void __RME_X64_USER185_Handler(void);
EXTERN void __RME_X64_USER186_Handler(void);
EXTERN void __RME_X64_USER187_Handler(void);
EXTERN void __RME_X64_USER188_Handler(void);
EXTERN void __RME_X64_USER189_Handler(void);

EXTERN void __RME_X64_USER190_Handler(void);
EXTERN void __RME_X64_USER191_Handler(void);
EXTERN void __RME_X64_USER192_Handler(void);
EXTERN void __RME_X64_USER193_Handler(void);
EXTERN void __RME_X64_USER194_Handler(void);
EXTERN void __RME_X64_USER195_Handler(void);
EXTERN void __RME_X64_USER196_Handler(void);
EXTERN void __RME_X64_USER197_Handler(void);
EXTERN void __RME_X64_USER198_Handler(void);
EXTERN void __RME_X64_USER199_Handler(void);

EXTERN void __RME_X64_USER200_Handler(void);
EXTERN void __RME_X64_USER201_Handler(void);
EXTERN void __RME_X64_USER202_Handler(void);
EXTERN void __RME_X64_USER203_Handler(void);
EXTERN void __RME_X64_USER204_Handler(void);
EXTERN void __RME_X64_USER205_Handler(void);
EXTERN void __RME_X64_USER206_Handler(void);
EXTERN void __RME_X64_USER207_Handler(void);
EXTERN void __RME_X64_USER208_Handler(void);
EXTERN void __RME_X64_USER209_Handler(void);

EXTERN void __RME_X64_USER210_Handler(void);
EXTERN void __RME_X64_USER211_Handler(void);
EXTERN void __RME_X64_USER212_Handler(void);
EXTERN void __RME_X64_USER213_Handler(void);
EXTERN void __RME_X64_USER214_Handler(void);
EXTERN void __RME_X64_USER215_Handler(void);
EXTERN void __RME_X64_USER216_Handler(void);
EXTERN void __RME_X64_USER217_Handler(void);
EXTERN void __RME_X64_USER218_Handler(void);
EXTERN void __RME_X64_USER219_Handler(void);

EXTERN void __RME_X64_USER220_Handler(void);
EXTERN void __RME_X64_USER221_Handler(void);
EXTERN void __RME_X64_USER222_Handler(void);
EXTERN void __RME_X64_USER223_Handler(void);
EXTERN void __RME_X64_USER224_Handler(void);
EXTERN void __RME_X64_USER225_Handler(void);
EXTERN void __RME_X64_USER226_Handler(void);
EXTERN void __RME_X64_USER227_Handler(void);
EXTERN void __RME_X64_USER228_Handler(void);
EXTERN void __RME_X64_USER229_Handler(void);

EXTERN void __RME_X64_USER230_Handler(void);
EXTERN void __RME_X64_USER231_Handler(void);
EXTERN void __RME_X64_USER232_Handler(void);
EXTERN void __RME_X64_USER233_Handler(void);
EXTERN void __RME_X64_USER234_Handler(void);
EXTERN void __RME_X64_USER235_Handler(void);
EXTERN void __RME_X64_USER236_Handler(void);
EXTERN void __RME_X64_USER237_Handler(void);
EXTERN void __RME_X64_USER238_Handler(void);
EXTERN void __RME_X64_USER239_Handler(void);

EXTERN void __RME_X64_USER240_Handler(void);
EXTERN void __RME_X64_USER241_Handler(void);
EXTERN void __RME_X64_USER242_Handler(void);
EXTERN void __RME_X64_USER243_Handler(void);
EXTERN void __RME_X64_USER244_Handler(void);
EXTERN void __RME_X64_USER245_Handler(void);
EXTERN void __RME_X64_USER246_Handler(void);
EXTERN void __RME_X64_USER247_Handler(void);
EXTERN void __RME_X64_USER248_Handler(void);
EXTERN void __RME_X64_USER249_Handler(void);

EXTERN void __RME_X64_USER250_Handler(void);
EXTERN void __RME_X64_USER251_Handler(void);
EXTERN void __RME_X64_USER252_Handler(void);
EXTERN void __RME_X64_USER253_Handler(void);
EXTERN void __RME_X64_USER254_Handler(void);
EXTERN void __RME_X64_USER255_Handler(void);
/* Interrupt control */
EXTERN void __RME_Disable_Int(void);
EXTERN void __RME_Enable_Int(void);
EXTERN void __RME_X64_Halt(void);
__EXTERN__ void __RME_X64_SMP_Tick(void);
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
EXTERN void __RME_Enter_User_Mode(ptr_t Entry_Addr, ptr_t Stack_Addr, ptr_t CPUID);
__EXTERN__ ptr_t __RME_Low_Level_Init(void);
__EXTERN__ ptr_t __RME_Boot(void);
__EXTERN__ void __RME_Reboot(void);
__EXTERN__ void __RME_Shutdown(void);
/* Syscall & invocation */
EXTERN ptr_t __RME_CPUID_Get(void);
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
__EXTERN__ void __RME_X64_Fault_Handler(struct RME_Reg_Struct* Reg, ptr_t Reason);
/* Generic interrupt handler */
__EXTERN__ void __RME_X64_Generic_Handler(struct RME_Reg_Struct* Reg, ptr_t Int_Num);
/* Page table operations */
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
/* __PLATFORM_X64_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
