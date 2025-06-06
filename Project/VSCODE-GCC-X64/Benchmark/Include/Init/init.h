/******************************************************************************
Filename    : init.h
Author      : pry
Date        : 23/09/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of mmu-based systems' user-level library.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __INIT_H_DEFS__
#define __INIT_H_DEFS__
/*****************************************************************************/
/* Priority of threads */
#define UVM_GUARD_PRIO           (UVM_MAX_PREEMPT_PRIO-1)
#define UVM_TIMD_PRIO            (UVM_VMD_PRIO)
#define UVM_VMMD_PRIO            (UVM_VMD_PRIO)
#define UVM_INTD_PRIO            (UVM_VMD_PRIO)
#define UVM_VINT_PRIO            (UVM_VMD_PRIO-1)
#define UVM_USER_PRIO            (2)
#define UVM_INIT_PRIO            (1)
#define UVM_WAIT_PRIO            (0)

/* Size of bitmap */
#define UVM_PRIO_BITMAP          ((UVM_MAX_PREEMPT_PRIO-1)/UVM_WORD_SIZE+1)
#define UVM_VECT_BITMAP(X)       ((X-1)/UVM_WORD_SIZE+1)

/* States of virtual machines */
#define UVM_VM_STATE(X)          ((X)&0xFF)
#define UVM_VM_FLAG(X)           ((X)&~0xFF)
#define UVM_VM_STATE_SET(X,S)    ((X)=(UVM_VM_FLAG(X)|(S)))

/* The virtual machine is running */
#define UVM_VM_RUNNING           (0)
/* The virtual machine is temporarily suspended and is waiting for an interrupt */
#define UVM_VM_WAITEVT           (1)

/* The virtual machine have its interrupt enabled */
#define UVM_VM_INTENA            (1<<(sizeof(ptr_t)<<2))
/* The virtual machine have finished its booting */
#define UVM_VM_BOOTDONE          (UVM_VM_INTENA<<1)
    
/* The timer wheel size */
#define UVM_WHEEL_SIZE           32
#define UVM_DLY2VM(X)            ((struct UVM_Virt*)(((ptr_t)(X))-sizeof(struct UVM_List)))
/*****************************************************************************/
/* __INIT_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __INIT_H_STRUCTS__
#define __INIT_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __INIT_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __INIT_MEMBERS__
#define __INIT_MEMBERS__

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
/* Initialization */
static void UVM_Clear(void* Addr, ptr_t Size);
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
/* Timestamp value */
__EXTERN__ ptr_t UVM_Tick;
/*****************************************************************************/

/* End Public Global Variables ***********************************************/

/* Public C Function Prototypes **********************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __INIT_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
