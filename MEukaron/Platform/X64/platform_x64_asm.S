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

/* Begin Stacks **************************************************************/
/*
                .section .bss
                .align  16
Stack_Mem:      .fill    0x1000
Stack_Addr:     .fill    0x100
*/
/* End Stacks ****************************************************************/

/* Begin Header **************************************************************/

/* End Header ****************************************************************/

/* Begin Exports *************************************************************/
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
/* End Exports ***************************************************************/

/* Begin Imports *************************************************************/
                /* What keil and CMSIS provided. Have to call these. */
                .global         _start
                /* The kernel entry of RME. This will be defined in C language. */
                .global         RME_Kmain
                /* The system call handler of RME. This will be defined in C language. */
                .global         _RME_Svc_Handler
                /* The system tick handler of RME. This will be defined in C language. */
                .global         _RME_Tick_Handler
/* End Imports ***************************************************************/

/* Begin Vector Table ********************************************************/

/* End Vector Table **********************************************************/

/* Begin Memory Init *********************************************************/
                 #define        MBOOT_MAGIC      0x1BADB002
                 #define        MBOOT_FLAGS      0x00010000

                 .code32
                 .align         16
                 .global        Mboot_Header
                 .global        Mboot_Entry

                 /* When GRUB finishes, the CPU is in 32-bit real mode */
Mboot_Header:
                 .long          MBOOT_MAGIC                   /* Magic number */
                 .long          MBOOT_FLAGS                   /* Boot flags */
                 .long          (-MBOOT_MAGIC-MBOOT_FLAGS)    /* Checksum */
                 .long          mboot_load_addr               /* header_addr */
                 .long          mboot_load_addr               /* .text addr */
                 .long          mboot_load_end                /* .data end addr */
                 .long          mboot_bss_end                 /* .bss end addr */
                 .long          mboot_entry_addr              /* Entry of the kernel */

                 /* Abbreviations:
	              * PML4:Page-Map Level-4
                  * PDP:Page Directory Pointer
                  * PD:Page Directory
                  * PT:Page Table */
Mboot_Entry:
                 /* EBX contains Multiboot data structure */
                 MOV            %EBX,%EDX
                 /* Zero 4 pages for our bootstrap page tables */
                 XOR            %EAX,%EAX
                 MOV            $0x1000,%EDI
                 MOV            $0x5000,%ECX
                 REP            STOSB
                 /* PML4[0] -> PDP-A @ 0x2000 */
                 MOV            $(0x2000|3),%EAX
                 MOV            %EAX,0x1000
                 /* PML4[511] -> PDP-B @ 0x3000 */
                 MOV            $(0x3000|3),%EAX
                 MOV            %EAX,0x1FF8
                 /* PDP-A[0] -> PD @ 0x4000 */
                 MOV            $(0x4000|3),%EAX
                 MOV            %EAX,0x2000
                 /* PDP-B[510] -> PD @ 0x4000 */
                 MOV            $(0x4000|3),%EAX
                 MOV            %EAX,0x3FF0
                 /* PD[0..511] -> 0..1024MB, maps first 1GB into the array. Maybe later we should map all */
                 MOV            $0x83,%EAX
                 MOV            $0x4000,%EBX
                 MOV            $512,%ECX
Ptbl_Loop:
                 MOV            %EAX,(%EBX)
                 ADD            $0x200000,%EAX
                 ADD            $0x8,%EBX
                 DEC            %ECX
                 JNZ            Ptbl_Loop
				 /* Clear ebx for initial processor boot.
                  * When secondary processors boot, they'll call through
                  * entry32mp (from entryother), but with a nonzero ebx.
                  * We'll reuse these bootstrap pagetables and GDT. */
                 XOR            %EBX,%EBX

                 .global Entry_MP
Entry_MP:
                 /* CR3 -> PML4 @ 0x1000 PML4 */
                 MOV            $0x1000,%EAX
                 MOV            %EAX,%CR3
                 /* This is where the gdtr's value is after we finish loading all stuff */
                 LGDT           (gdtr64-Mboot_Header+mboot_load_addr)

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
                 LJMP           $8,$(entry64low-Mboot_Header+mboot_load_addr)

                 /* The actual gdt that we use - we keep this all the time */
                 .align         16
gdtr64:
                 .word          gdt64_end - gdt64_begin - 1;
                 .quad          gdt64_begin - Mboot_Header + mboot_load_addr
                 .align         16
gdt64_begin:     /* 0: null desc */
                 .long          0x00000000
                 .long          0x00000000
                 /* 1: Code, R/X, Nonconforming (transfers only possible with exceptions) */
                 .long          0x00000000
                 .long          0x00209800
                 /* 2: Data, R/W, Expand Down */
                 .long          0x00000000
                 .long          0x00009000
gdt64_end:

                 .align 16
                 .code64
entry64low:
                 movq           $entry64high,%RAX
                 jmp            *%RAX

                 .global        _start
_start:
entry64high:
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
                 JNZ            entry64mp
                 /* Setup initial stack - this is hard-coded at low memory, we will switch to ours later */
                 MOV            $0xFFFFFFFF80010000,%RAX
                 MOV            %RAX,%RSP
                 /* enter main() */
                 JMP            main
                 JMP            .

entry64mp:       /* When obtaining these stacks we need to be NUMA-aware to guarantee performance */
                 /* obtain kstack from data block before entryother */
                 MOV            $0x7000,%RAX
                 MOV            -16(%RAX),%RSP
                 JMP            .
/*
.global wrmsr
wrmsr:
  mov %rdi, %rcx     # arg0 -> msrnum
  mov %rsi, %rax     # val.low -> eax
  shr $32, %rsi
  mov %rsi, %rdx     # val.high -> edx
  wrmsr
  retq
  */
/* End Memory Init ***********************************************************/

/* Begin Handlers ************************************************************/

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

/* Begin Function:__RME_Disable_Int *******************************************
Description    : The function for disabling all interrupts.
Input          : None.
Output         : None.
Register Usage : None.
******************************************************************************/
__RME_Disable_Int:
                /* Disable all interrupts (I is primask, F is faultmask.) */
                RET
/* End Function:__RME_Disable_Int ********************************************/

/* Begin Function:__RME_Enable_Int ********************************************
Description    : The function for enabling all interrupts.
Input          : None.
Output         : None.
Register Usage : None.
******************************************************************************/
__RME_Enable_Int:
                /* Enable all interrupts. */
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
