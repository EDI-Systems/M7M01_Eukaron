/******************************************************************************
Filename   : rvmlib_STM32F767IG.h
Author     : pry
Date       : 26/06/2017
Licence    : LGPL v3+; see COPYING for details.
Description: The configuration file for STM32F767IG. The values listed here should
             be in accordance with that in the kernel.
******************************************************************************/

/* Defines *******************************************************************/

/* Kernel configurations - keep the same with the kernel *********************/
/* The virtual memory start address for the kernel objects */
#define UVM_KMEM_VA_START            0x20003000
/* The size of the kernel object virtual memory */
#define UVM_KMEM_SIZE                0xD000
/* The granularity of kernel memory allocation, order of 2 in bytes */
#define UVM_KMEM_SLOT_ORDER          4
/* Kernel stack size and address */
#define UVM_KMEM_STACK_ADDR          0x20001FF0
/* The maximum number of preemption priority levels in the system.
 * This parameter must be divisible by the word length - 32 is usually sufficient */
#define UVM_MAX_PREEMPT_PRIO         32
/* The maximum number of events in the system */
#define UVM_MAX_EVT_NUM              1024
/* Shared interrupt flag region address - this will only appear on MPU-based systems */
#define UVM_INT_FLAG_ADDR            0x20010000
/* How many interrupts are there in the hardware */
#define UVM_INT_VECT_NUM             110
/* How many interrupt mappings are we going to make? */
#define UVM_INT_MAP_NUM              1024

/* Initial kenel object frontier limit */
#define UVM_CMX_KMEM_BOOT_FRONTIER   0x20003400
/* Number of MPU regions available */
#define UVM_CMX_MPU_REGIONS          8
/* Init process's first thread's entry point address */
#define UVM_CMX_INIT_ENTRY           (0x08004000|0x01)
/* Init process's first thread's stack address */
#define UVM_CMX_INIT_STACK           0x2001FFF0
/* What is the FPU type? */
#define UVM_CMX_FPU_TYPE             UVM_CMX_FPV5_DP
/* Syslib configurations *****************************************************/
/* The user image header starting position */
#define UVM_IMAGE_HEADER_START       0x08020000

/* The stack size of the guard daemon process, in bytes */
#define UVM_GUARD_STACK_SIZE         2048
/* The stack size of the timer daemon process, in bytes */
#define UVM_TIMD_STACK_SIZE          2048
/* The stack size of the virtual machine monitor daemon process, in bytes */
#define UVM_VMMD_STACK_SIZE          2048
/* The stack size of the interrupt daemon process, in bytes */
#define UVM_INTD_STACK_SIZE          2048

/* Config settings - modifications not needed for most cases */
/* How many VMs are allowed? - This is not to exceed 32 in current implementations */
#define UVM_MAX_VM_NUM               16
/* How many different priorities are allowed? */
#define UVM_MAX_PREEMPT_PRIO         32
/* The priority levels:
 * 31      : Guard daemon - fixed
 * 30 - 5  : Handler threads
 * 4       : Timer daemon (adjustable from 29 to 2)
 *           Virtual machine monitor daemon
 *           Interrupt daemon
 * 3       : Active virtual machine - interrupt thread
 * 2       : Active virtual machine - user thread
 * 1       : Init daemon
 * 0       : Inactive virtual machine
 */
#define UVM_VMD_PRIO                 4

/* Maximum number of interrupts - must be a multiple of word size, and cannot exceed 128 */
#define UVM_MAX_INT_NUM              ((1)*(sizeof(ptr_t)*8))

/* Is debugging output enabled? */
#define UVM_DEBUG_LOG                UVM_TRUE
/* End Defines ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
