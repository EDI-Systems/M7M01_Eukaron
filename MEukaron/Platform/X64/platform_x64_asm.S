/******************************************************************************
Filename    : platform_x64_asm.s
Author      : pry
Date        : 19/01/2017
Description : The x64 assembly support of the RME RTOS.
              Parts of this is adapted from the xv6 port on x64.
******************************************************************************/

/* The X64 Structure **********************************************************
RAX
******************************************************************************/

/* Begin Header **************************************************************/

/* End Header ****************************************************************/

/* Begin Exports *************************************************************/
                /* Atomic compare and exchange */
                .global         __RME_Comp_Swap
                /* Atomic add */
                .global         __RME_Fetch_Add
                /* Atomic and */
                .global         __RME_Fetch_And
                /* Load page table */
                .global         __RME_Pgtbl_Set
                /* Disable all interrupts */
                .global         __RME_Disable_Int
                /* Enable all interrupts */
                .global         __RME_Enable_Int
                /* Get the MSB in a word */
                .global         __RME_MSB_Get
                /* Kernel main function wrapper */
                .global         _RME_Kmain
                /* Entering of the user mode */
                .global         __RME_Enter_User_Mode

                /* X64 specific stuff */
                /* Input from a port */
                .global         __RME_X64_In
                /* Output to a port */
                .global         __RME_X64_Out
                /* Load GDT */
                .global         __RME_X64_GDT_Load
                /* Load IDT */
                .global         __RME_X64_IDT_Load
                /* Load TSS */
                .global         __RME_X64_TSS_Load
                /* CPUID instruction */
                .global         __RME_X64_CPUID_Get

                /* Booting specific stuff */
                .global         _start
                .global         __RME_X64_Mboot_Header
                .global         __RME_X64_Mboot_Entry
                .global         RME_X64_Kpgt
                .global         __RME_X64_Kern_Boot_Stack
                .global         __RME_X64_SMP_Boot_32
/* End Exports ***************************************************************/

/* Begin Imports *************************************************************/
                /* The kernel entry of RME. This will be defined in C language. */
                .global         RME_Kmain
                /* The system call handler of RME. This will be defined in C language. */
                .global         _RME_Svc_Handler
                /* The system tick handler of RME. This will be defined in C language. */
                .global         _RME_Tick_Handler
                /* The entry of SMP after they have finished their initialization */
                .global         __RME_SMP_Low_Level_Init
/* End Imports ***************************************************************/

/* Begin Vector Table ********************************************************/

/* End Vector Table **********************************************************/

/* Begin Memory Init *********************************************************/
/* Multiboot header that will be located at 0x100000 *************************/
                 .section       .text
                 .code32
                 .align         16
__RME_X64_Mboot_Header:
                 .long          0x1BADB002                    /* Magic number */
                 .long          0x00010000                    /* Boot flags */
                 .long          (-0x1BADB002-0x00010000)      /* Checksum */
                 .long          __RME_X64_Mboot_Load_Addr     /* header_addr */
                 .long          __RME_X64_Mboot_Load_Addr     /* .text addr */
                 .long          __RME_X64_Mboot_Load_End      /* .data end addr */
                 .long          __RME_X64_Mboot_BSS_End       /* .bss end addr */
                 .long          __RME_X64_Mboot_Entry_Addr    /* Entry of the kernel */

/* Initial page table ********************************************************/
                 /* 7 pages that start from 0x101000. We can't use 1GB pages here
                  * because we do not know whether the CPU supports that or not.
                  * We will detect these later when we are making the kernel pages */
                 .align         4096
                 .space         7*4096

/* The entry point after the bootloader finishes *****************************/
                 .code32
                 .align         16
__RME_X64_Mboot_Entry:
                 /* EBX contains Multiboot data structure, let's relocate it to some other places */
                 MOV            %EBX,%ESI
                 /* Zero 7 pages for our bootstrap page tables, PML4 @ 0x101000 */
                 CLD
                 XOR            %EAX,%EAX
                 MOV            $0x101000,%EDI
                 MOV            $0x7000,%ECX
                 REP            STOSB
                 /* PML4[0] -> PDP-A @ 0x102000 */
                 MOV            $(0x102000|3),%EAX
                 MOV            %EAX,0x101000
                 /* PML4[256] -> PDP-A @ 0x102000 */
                 MOV            $(0x102000|3),%EAX
                 MOV            %EAX,0x101800
                 /* PML4[511] -> PDP-B @ 0x103000 */
                 MOV            $(0x103000|3),%EAX
                 MOV            %EAX,0x101FF8
                 /* PDP-A[0] -> PDE-A @ 0x104000 */
                 MOV            $(0x104000|3),%EAX
                 MOV            %EAX,0x102000
                 /* PDP-A[1] -> PDE-B @ 0x105000 */
                 MOV            $(0x105000|3),%EAX
                 MOV            %EAX,0x102008
                 /* PDP-A[2] -> PDE-C @ 0x106000 */
                 MOV            $(0x106000|3),%EAX
                 MOV            %EAX,0x102010
                 /* PDP-A[3] -> PDE-D @ 0x107000 unbufferable uncacheable */
                 MOV            $(0x107000|3|0x18),%EAX
                 MOV            %EAX,0x102018
                 /* PDP-B[510] -> PDE-A @ 0x104000 */
                 MOV            $(0x104000|3),%EAX
                 MOV            %EAX,0x103FF0
                 /* PDP-B[511] -> PDE-B @ 0x105000 */
                 MOV            $(0x105000|3),%EAX
                 MOV            %EAX,0x103FF8
                 /* PDE-A/B/C/D [0..511/0...511/0..511/0..511] -> 0..4094MB */
                 MOV            $(0x83),%EAX
                 MOV            $0x104000,%EBX
                 MOV            $(512*4),%ECX
PDE_Loop:
                 MOV            %EAX,(%EBX)
                 ADD            $0x200000,%EAX
                 ADD            $0x8,%EBX
                 DEC            %ECX
                 JNZ            PDE_Loop
				 /* Clear ebx for initial processor boot.
                  * When secondary processors boot, they'll call through
                  * __RME_X64_SMP_Boot, but with a nonzero ebx.
                  * We'll reuse these bootstrap pagetables and GDT. */
                 XOR            %EBX,%EBX
                 /* SMP boot will run directly from here */
__RME_X64_SMP_Boot_32:
                 /* CR3 -> PML4 @ 0x101000 */
                 MOV            $0x101000,%EAX
                 MOV            %EAX,%CR3
                 /* Load GDT */
                 LGDT           (Boot_GDT_Desc-__RME_X64_Mboot_Header+__RME_X64_Mboot_Load_Addr)
                 /* Enable PAE - CR4.PAE=1 */
                 MOV            %CR4,%EAX
                 BTS            $5,%EAX
                 MOV            %EAX,%CR4
                 /* Enable long mode and no execute bit - EFER.LME=1, EFER.NXE=1 */
                 MOV            $0xC0000080,%ECX
                 RDMSR
                 BTS            $8,%EAX
                 BTS            $11,%EAX
                 WRMSR
                 /* Enable paging */
                 MOV            %CR0,%EAX
                 BTS            $31,%EAX
                 MOV            %EAX,%CR0
                 /* shift to 64bit segment */
                 LJMP           $8,$(Boot_Low_64-__RME_X64_Mboot_Header+__RME_X64_Mboot_Load_Addr)

/* Now we are in 64-bit mode *************************************************/
                 .align         16
                 .code64
Boot_Low_64:
                 movq           $Boot_High_64,%RAX
                 jmp            *%RAX
_start:
Boot_High_64:
                 /* ensure data segment registers are sane - zero out all of them */
                 XOR            %RAX,%RAX
                 /* Below are all 16-bit moves */
                 MOV            %AX,%SS
                 MOV            %AX,%DS
                 MOV            %AX,%ES
                 MOV            %AX,%FS
                 MOV            %AX,%GS
                 /* Check to see if we're booting a secondary core */
                 TEST           %EBX,%EBX
                 JNZ            Boot_SMP_64
                 /* Setup initial stack - this is hard-coded at low memory */
                 MOV            $__RME_X64_Kern_Boot_Stack,%RAX
                 MOV            %RAX,%RSP
                 /* Pass the physical address of RSI to it */
                 MOV            %RSI,%RDI
                 JMP            main
                 JMP            .
Boot_SMP_64:     MOV            $0x7000,%RAX
                 MOV            -16(%RAX),%RSP
                 JMP            .
                 JMP            __RME_SMP_Low_Level_Init
                 JMP            .

                 /* The initial gdt. Later we will have one GDT per CPU */
                 .align         16
Boot_GDT:
                 /* 0: Null descriptor */
                 .long          0x00000000
                 .long          0x00000000
                 /* 1: Code, R/X, Nonconforming (transfers only possible with exceptions) */
                 .long          0x00000000
                 .long          0x00209800
                 /* 2: Data, R/W, Expand Down */
                 .long          0x00000000
                 .long          0x00009000
Boot_GDT_Desc:
                 .word          Boot_GDT_Desc-Boot_GDT-1;
                 .quad          Boot_GDT-__RME_X64_Mboot_Header+__RME_X64_Mboot_Load_Addr;
                 .align         16

                 /* The kernel page table */
                 .align         4096
RME_X64_Kpgt:
                 .space         8*(256+256*512)

                 /* The initial kernel stack */
                 .space         8192
__RME_X64_Kern_Boot_Stack:
/* End Memory Init ***********************************************************/

/* Begin Handlers ************************************************************/
                 .global __RME_X64_FAULT_DE_Handler
                 .global __RME_X64_TRAP_DB_Handler
                 .global __RME_X64_INT_NMI_Handler
                 .global __RME_X64_TRAP_BP_Handler
                 .global __RME_X64_TRAP_OF_Handler
                 .global __RME_X64_FAULT_BR_Handler
                 .global __RME_X64_FAULT_NM_Handler
                 .global __RME_X64_ABORT_DF_Handler
                 .global __RME_X64_ABORT_OLD_MF_Handler
                 .global __RME_X64_FAULT_TS_Handler
                 .global __RME_X64_FAULT_NP_Handler
                 .global __RME_X64_FAULT_SS_Handler
                 .global __RME_X64_FAULT_GP_Handler
                 .global __RME_X64_FAULT_PF_Handler
                 .global __RME_X64_FAULT_MF_Handler
                 .global __RME_X64_FAULT_AC_Handler
                 .global __RME_X64_ABORT_MC_Handler
                 .global __RME_X64_FAULT_XM_Handler
                 .global __RME_X64_FAULT_VE_Handler
                 .global __RME_X64_INT_USER_Handler

                 /* Currently we do nothing in these handlers */
__RME_X64_FAULT_DE_Handler:
__RME_X64_TRAP_DB_Handler:
__RME_X64_INT_NMI_Handler:
__RME_X64_TRAP_BP_Handler:
__RME_X64_TRAP_OF_Handler:
__RME_X64_FAULT_BR_Handler:
__RME_X64_FAULT_NM_Handler:
__RME_X64_ABORT_DF_Handler:
__RME_X64_ABORT_OLD_MF_Handler:
__RME_X64_FAULT_TS_Handler:
__RME_X64_FAULT_NP_Handler:
__RME_X64_FAULT_SS_Handler:
__RME_X64_FAULT_GP_Handler:
__RME_X64_FAULT_PF_Handler:
__RME_X64_FAULT_MF_Handler:
__RME_X64_FAULT_AC_Handler:
__RME_X64_ABORT_MC_Handler:
__RME_X64_FAULT_XM_Handler:
__RME_X64_FAULT_VE_Handler:
__RME_X64_INT_USER_Handler:
                 CLI
                 JMP            .
/* End Handlers **************************************************************/

/* Begin Function:__RME_X64_In ************************************************
Description    : The function for outputting something to an I/O port.
Input          : ptr_t Port - The port to output to.
                 ptr_t Data - The data to send to that port.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
__RME_X64_In:
                 PUSH            %RDX
                 PUSH            %RAX
                 MOV             %RDI,%RDX
                 MOV             %RSI,%RAX
                 INB             (%DX),%AL
                 POP             %RDX
                 POP             %RAX
                 RET
/* End Function:__RME_X64_In *************************************************/

/* Begin Function:__RME_X64_Out ***********************************************
Description    : The function for outputting something to an I/O port.
Input          : ptr_t Port - The port to output to.
                 ptr_t Data - The data to send to that port.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
__RME_X64_Out:
                 PUSH            %RDX
                 PUSH            %RAX
                 MOV             %RDI,%RDX
                 MOV             %RSI,%RAX
                 OUTB            %AL,(%DX)
                 POP             %RDX
                 POP             %RAX
                 RET
/* End Function:__RME_X64_Out ************************************************/

/* Begin Function:__RME_X64_CPUID_Get *****************************************
Description    : The function for outputting something to an I/O port.
Input          : ptr_t EAX - The EAX value to get the CPUID for.
                 ptr_t* EBX - The EBX info.
                 ptr_t* ECX - The ECX info.
                 ptr_t* EDX - The EDX info.
Output         : ptr_t* EBX - The EBX info.
                 ptr_t* ECX - The ECX info.
                 ptr_t* EDX - The EDX info.
Return         : ptr_t - The maximum number for CPUID instruction.
Register Usage : None.
******************************************************************************/
__RME_X64_CPUID_Get:
                 PUSH            %R8
                 PUSH            %R9
                 PUSH            %RBX
                 PUSH            %RCX
                 PUSH            %RDX

                 MOV             %RDI,%RAX
                 MOV             %RDX,%R8
                 MOV             %RCX,%R9

                 MOV             (%RSI),%RBX
                 MOV             (%R8),%RCX
                 MOV             (%R9),%RDX
                 CPUID
                 MOV             %RBX,(%RSI)
                 MOV             %RCX,(%R8)
                 MOV             %RDX,(%R9)

                 POP             %RDX
                 POP             %RCX
                 POP             %RBX
                 POP             %R9
                 POP             %R8
                 RET
/* End Function:__RME_X64_CPUID_Get ******************************************/

/* Begin Function:__RME_X64_GDT_Load ******************************************
Description    : The function for loading the GDT. Every CPU needs to load their
                 own GDT. No need to flush segment registers cause we're in 64-bit
                 mode.
Input          : ptr_t* GDTR - The pointer to the GDT descriptor.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
__RME_X64_GDT_Load:
                 LGDT            (%RDI)
                 RET
/* End Function:__RME_X64_GDT_Load *******************************************/

/* Begin Function:__RME_X64_IDT_Load ******************************************
Description    : The function for loading the IDT. Every CPU needs to load their
                 own IDT.
Input          : ptr_t* IDTR - The pointer to the IDT descriptor.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
__RME_X64_IDT_Load:
                 LIDT            (%RDI)
                 RET
/* End Function:__RME_X64_IDT_Load *******************************************/

/* Begin Function:__RME_X64_TSS_Load ******************************************
Description    : The function for loading the TSS's entry in GDT.
Input          : ptr_t TSS - The TSS's position in GDT.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
__RME_X64_TSS_Load:
                 LTR             %DI
                 RET
/* End Function:__RME_X64_TSS_Load *******************************************/

/* Begin Function:__RME_Comp_Swap *********************************************
Description : The compare-and-swap atomic instruction. If the *Old value is equal to
              *Ptr, then set the *Ptr as New and return 1; else set the *Old as *Ptr,
              and return 0.
Input       : ptr_t* Ptr - The pointer to the data.
              ptr_t* Old - The old value.
              ptr_t New - The new value.
Output      : ptr_t* Ptr - The pointer to the data.
              ptr_t* Old - The old value.
Return      : ptr_t - If successful, 1; else 0.
******************************************************************************/
__RME_Comp_Swap:
                MOV              (%RSI),%RAX
                LOCK CMPXCHG     %RDX,(%RDI)
                JZ               __RME_Comp_Swap_Fail
                MOV              $1,%RAX
                RET
__RME_Comp_Swap_Fail:
                MOV              %RAX,(%RSI)
                MOV              $0,%RAX
/* End Function:__RME_Comp_Swap **********************************************/

/* Begin Function:__RME_Fetch_Add *********************************************
Description : The fetch-and-add atomic instruction. Increase the value that is
              pointed to by the pointer, and return the value before addition.
Input       : ptr_t* Ptr - The pointer to the data.
              cnt_t Addend - The number to add.
Output      : ptr_t* Ptr - The pointer to the data.
Return      : ptr_t - The value before the addition.
******************************************************************************/
__RME_Fetch_Add:
               MOV              (%RDI),%RAX
               LOCK ADD         %RSI,(%RDI)
               RET
/* End Function:__RME_Fetch_Add **********************************************/

/* Begin Function:__RME_Fetch_And *********************************************
Description : The fetch-and-logic-and atomic instruction. Logic AND the pointer
              value with the operand, and return the value before logic AND.
              On Cortex-M there is only one core. There's basically no need to do
              anything special.
Input       : ptr_t* Ptr - The pointer to the data.
              cnt_t Operand - The number to logic AND with the destination.
Output      : ptr_t* Ptr - The pointer to the data.
Return      : ptr_t - The value before the AND operation.
******************************************************************************/
__RME_Fetch_And:
               MOV              (%RDI),%RAX
               LOCK AND         %RSI,(%RDI)
               RET
/* End Function:__RME_Fetch_And **********************************************/

/* Begin Function:__RME_Pgtbl_Set *********************************************
Description : Set the processor's page table.
Input       : ptr_t Pgtbl - The physical address of the page table.
Output      : None.
Return      : None.
******************************************************************************/
__RME_Pgtbl_Set:
               MOV              %RDI,%CR3
               RET
/* End Function:__RME_Pgtbl_Set **********************************************/

/* Begin Function:__RME_Disable_Int *******************************************
Description    : The function for disabling all interrupts.
Input          : None.
Output         : None.
Register Usage : None.
******************************************************************************/
__RME_Disable_Int:
                /* Disable all interrupts */
                CLI
                RET
/* End Function:__RME_Disable_Int ********************************************/

/* Begin Function:__RME_Enable_Int ********************************************
Description    : The function for enabling all interrupts.
Input          : None.
Output         : None.
Register Usage : None.
******************************************************************************/
__RME_Enable_Int:
                /* Enable all interrupts */
                STI
                RET
/* End Function:__RME_Enable_Int *********************************************/

/* Begin Function:__RME_CMX_WFI ***********************************************
Description    : Wait until a new interrupt comes, to save power.
Input          : None.
Output         : None.
Register Usage : None.
******************************************************************************/
__RME_CMX_WFI:
                /* Wait for interrupt. */
                RET
/* End Function:__RME_CMX_WFI ************************************************/

/* Begin Function:_RME_Kmain **************************************************
Description    : The entry address of the kernel. Never returns.
Input          : ptr_t Stack - The stack address to set SP to.
Output         : None.
Return         : None.
Register Usage : None.
******************************************************************************/
_RME_Kmain:
                 MOV      %RDI,%RSP
                 JMP      RME_Kmain
/* End Function:_RME_Kmain ***************************************************/

/* Begin Function:__RME_MSB_Get ***********************************************
Description    : Get the MSB of the word.
Input          : ptr_t Val - The value.
Output         : None.
Return         : ptr_t - The MSB position.
Register Usage : None.
******************************************************************************/
__RME_MSB_Get:
                RET
/* End Function:__RME_MSB_Get ************************************************/

/* Begin Function:__RME_Enter_User_Mode ***************************************
Description : Entering of the user mode, after the system finish its preliminary
              booting. The function shall never return. This function should only
              be used to boot the first process in the system.
Input       : R0 - The user execution startpoint.
              R1 - The user stack.
Output      : None.
******************************************************************************/
__RME_Enter_User_Mode:
                RET
/* End Function:__RME_Enter_User_Mode ****************************************/

/* Begin Function:SysTick_Handler *********************************************
Description : The System Tick Timer handler routine. This will in fact call a
              C function to resolve the system service routines.
Input       : None.
Output      : None.
******************************************************************************/
SysTick_Handler:
                RET
/* End Function:SysTick_Handler **********************************************/

/* Begin Function:SVC_Handler *************************************************
Description : The SVC handler routine. This will in fact call a C function to resolve
              the system service routines.
Input       : None.
Output      : None.
******************************************************************************/
SVC_Handler:
                RET
/* End Function:SVC_Handler **************************************************/

/* Begin Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler *********
Description : The multi-purpose handler routine. This will in fact call
              a C function to resolve the system service routines.
Input       : None.
Output      : None.
******************************************************************************/

/* End Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler **********/

;/* End Of File **************************************************************/

;/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
