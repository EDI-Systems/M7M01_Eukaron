/******************************************************************************
Filename    : RME.h
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the RME RTOS. This header defines the error codes,
              operation flags and system call numbers in a generic way.
******************************************************************************/

#ifndef __RME_H__
#define __RME_H__
/* Defines *******************************************************************/

/* Errors ********************************************************************/
/* The base of capability table error */
#define RME_ERR_CAP                  (0)
/* The capability is empty */
#define RME_ERR_CAP_NULL             ((-1)+RME_ERR_CAP)
/* The capability type is wrong */
#define RME_ERR_CAP_TYPE             ((-2)+RME_ERR_CAP)
/* The range of the capability is wrong */
#define RME_ERR_CAP_RANGE            ((-3)+RME_ERR_CAP)
/* The kernel object table operation of the capability is wrong */
#define RME_ERR_CAP_KOTBL            ((-4)+RME_ERR_CAP)
/* The capability number already exists, or not exist, depending on the function */
#define RME_ERR_CAP_EXIST            ((-5)+RME_ERR_CAP)
/* When freezing capabilities, the refcnt is not zero, or when delegating, refcnt overflowed */
#define RME_ERR_CAP_REFCNT           ((-6)+RME_ERR_CAP)
/* The flags of the fine-grain-controlled capability is not correct */
#define RME_ERR_CAP_FLAG             ((-7)+RME_ERR_CAP)
/* Something involved is not quiescent */
#define RME_ERR_CAP_QUIE             ((-8)+RME_ERR_CAP)
/* Something involved is frozen */
#define RME_ERR_CAP_FROZEN           ((-9)+RME_ERR_CAP)

/* The base of page table errors */
#define RME_ERR_PGT                  (-10)
/* Incorrect address */
#define RME_ERR_PGT_ADDR             ((-1)+RME_ERR_PGT)
/* Mapping failure */
#define RME_ERR_PGT_MAP              ((-2)+RME_ERR_PGT)
/* Hardware restrictions */
#define RME_ERR_PGT_HW               ((-3)+RME_ERR_PGT)
/* Wrong permissions */
#define RME_ERR_PGT_PERM             ((-4)+RME_ERR_PGT)

/* The base of process/thread errors */
#define RME_ERR_PTH                  (-20)
/* Incorrect address */
#define RME_ERR_PTH_PGTBL            ((-1)+RME_ERR_PTH)
/* Conflicting operations happening at the same time */
#define RME_ERR_PTH_CONFLICT         ((-2)+RME_ERR_PTH)
/* Thread invocation stack size error */
#define RME_ERR_PTH_INVSTK           ((-3)+RME_ERR_PTH)
/* Thread state is invalid or we have failed the CAS */
#define RME_ERR_PTH_INVSTATE         ((-4)+RME_ERR_PTH)
/* The thread priority is not correct */
#define RME_ERR_PTH_PRIO             ((-5)+RME_ERR_PTH)
/* The reference count not correct */
#define RME_ERR_PTH_REFCNT           ((-6)+RME_ERR_PTH)
/* There are no notifications sent to this thread */
#define RME_ERR_PTH_NOTIF            ((-7)+RME_ERR_PTH)
/* The time is full for this thread */
#define RME_ERR_PTH_OVERFLOW         ((-8)+RME_ERR_PTH)
/* Cannot conduct operation because target have a fault */
#define RME_ERR_PTH_FAULT            ((-9)+RME_ERR_PTH)

/* The base of signal/invocation errors */
#define RME_ERR_SIV                  (-30)
/* This invocation capability/signal endpoint is already active */
#define RME_ERR_SIV_ACT              ((-1)+RME_ERR_SIV)
/* This invocation succeeded, but a fault happened in the thread, and we forced a invocation return */
#define RME_ERR_SIV_FAULT            ((-2)+RME_ERR_SIV)
/* The thread's invocation stack/signal counter is full */
#define RME_ERR_SIV_FULL             ((-3)+RME_ERR_SIV)
/* The thread's invocation stack is empty, cannot return */
#define RME_ERR_SIV_EMPTY            ((-4)+RME_ERR_SIV)
/* The signal capability is operated on by two threads simutaneously */
#define RME_ERR_SIV_CONFLICT         ((-5)+RME_ERR_SIV)
/* The signal receive system call ended with a result of free(forced unblock) */
#define RME_ERR_SIV_FREE             ((-6)+RME_ERR_SIV)
/* The signal receive failed because we are the boot-time thread */
#define RME_ERR_SIV_BOOT             ((-7)+RME_ERR_SIV)
/* End Errors ****************************************************************/

/* Operation Flags ***********************************************************/
/* Captbl */
/* This cap to captbl allows creating kernel objects in the captbl */
#define RME_CAPTBL_FLAG_CRT          (1<<0)
/* This cap to captbl allows deleting kernel objects in the captbl */
#define RME_CAPTBL_FLAG_DEL          (1<<1)
/* This cap to captbl allows freezing kernel objects in the captbl */
#define RME_CAPTBL_FLAG_FRZ          (1<<2)
/* This cap to captbl allows delegating kernel objects in it */
#define RME_CAPTBL_FLAG_ADD_SRC      (1<<3)
/* This cap to captbl allows receiving delegated kernel objects to it */
#define RME_CAPTBL_FLAG_ADD_DST      (1<<4)  
/* This cap to captbl allows removal operation in kernel objects(captbls) in the captbl */
#define RME_CAPTBL_FLAG_REM          (1<<5)
/* This cap to captbl allows itself to be used in process creation */
#define RME_CAPTBL_FLAG_PROC_CRT     (1<<6)
/* This cap to captbl allows itself to be used in process capability table replacement */
#define RME_CAPTBL_FLAG_PROC_CPT     (1<<7)
    
/* Kernel memory */
/* This cap to kernel memory allows creation of captbl */
#define RME_KMEM_FLAG_CAPTBL         (1<<0)
/* This cap to kernel memory allows creation of pgtbl */
#define RME_KMEM_FLAG_PGTBL          (1<<1)
/* This cap to kernel memory allows creation of process */
#define RME_KMEM_FLAG_PROC           (1<<2)
/* This cap to kernel memory allows creation of thread */
#define RME_KMEM_FLAG_THD            (1<<3)
/* This cap to kernel memory allows creation of signals */
#define RME_KMEM_FLAG_SIG            (1<<4)
/* This cap to kernel memory allows creation of invocation */
#define RME_KMEM_FLAG_INV            (1<<5)

/* Page table */
/* This cap to pgtbl allows delegating pages in it */
#define RME_PGTBL_FLAG_ADD_SRC       (1<<0)
/* This cap to pgtbl allows receiving delegated pages to it */
#define RME_PGTBL_FLAG_ADD_DST       (1<<1)
/* This cap to pgtbl allows removal of pages in it */
#define RME_PGTBL_FLAG_REM           (1<<2) 
/* This cap to pgtbl allows to be mapped into other page tables */
#define RME_PGTBL_FLAG_CON_CHILD     (1<<3)
/* This cap to pgtbl allows accepting lower page table mappings */
#define RME_PGTBL_FLAG_CON_PARENT    (1<<4)
/* This cap to pgtbl allows its lower level page table mappings to be destructed */
#define RME_PGTBL_FLAG_DES           (1<<5)
/* This cap to pgtbl allows itself to be used in process creation */
#define RME_PGTBL_FLAG_PROC_CRT      (1<<6)
/* This cap to pgtbl allows itself to be used in process page table replacement */
#define RME_PGTBL_FLAG_PROC_PGT      (1<<7)

/* Process */
/* This cap to process allows creating invocation stubs in it */
#define RME_PROC_FLAG_INV            (1<<0)
/* This cap to process allows creating threads in it */
#define RME_PROC_FLAG_THD            (1<<1)
/* This cap to process allows changing its capability table */
#define RME_PROC_FLAG_CPT            (1<<2)
/* This cap to process allows changing its page table */
#define RME_PROC_FLAG_PGT            (1<<3)

/* Thread */
/* This cap to thread allows setting its execution parameters */
#define RME_THD_FLAG_EXEC_SET        (1<<0)
/* This cap to thread allows setting its hypervisor parameters */
#define RME_THD_FLAG_HYP_SET         (1<<1)
/* This cap to thread allows setting its scheduling parameters */
#define RME_THD_FLAG_SCHED_CHILD     (1<<2)
/* This cap to thread allows registering it as a scheduler */
#define RME_THD_FLAG_SCHED_PARENT    (1<<3)
/* This cap to thread allows changing its priority level */
#define RME_THD_FLAG_SCHED_PRIO      (1<<4)
/* This cap to thread allows freeing the thread from the core */
#define RME_THD_FLAG_SCHED_FREE      (1<<5)
/* This cap to thread allows getting scheduling notifications */
#define RME_THD_FLAG_SCHED_RCV       (1<<6)
/* This cap to thread allows acting as a source for time transfer */
#define RME_THD_FLAG_XFER_SRC        (1<<7)
/* This cap to thread allows acting as a destination for time transfer */
#define RME_THD_FLAG_XFER_DST        (1<<8)
/* This cap to thread allows switching to it */
#define RME_THD_FLAG_SWT             (1<<9)

/* Invocation */
/* This cap to invocation allows setting parameters for it */
#define RME_INV_FLAG_SET             (1<<0)
/* This cap to invocation allows activating it */
#define RME_INV_FLAG_ACT             (1<<1)
/* The return operation does not need a flag, nor does it need a capability */

/* Signal */
/* This cap to signal endpoint allows sending to it */
#define RME_SIG_FLAG_SND             (1<<0)
/* This cap to signal endpoint allows receiving on it */
#define RME_SIG_FLAG_RCV             (1<<1)
/* End Operation Flags *******************************************************/

/* Special Definitions *******************************************************/
/* Generic page table flags */
/* This page allows to be read */
#define RME_PGTBL_READ               (1<<0)
/* This page allows to be written */
#define RME_PGTBL_WRITE              (1<<1)
/* This page allows execution */
#define RME_PGTBL_EXECUTE            (1<<2)
/* This page is cacheable */
#define RME_PGTBL_CACHEABLE          (1<<3)
/* This page is bufferable (write-back can be used instead of write-through) */
#define RME_PGTBL_BUFFERABLE         (1<<4)
/* This page is pinned in TLB */
#define RME_PGTBL_STATIC             (1<<5)
/* All the permissions are set */
#define RME_PGTBL_ALL_PERM           (RME_PGTBL_READ|RME_PGTBL_WRITE|RME_PGTBL_EXECUTE| \
                                      RME_PGTBL_CACHEABLE|RME_PGTBL_BUFFERABLE|RME_PGTBL_STATIC)
                                        
/* Generic page size order definitions */
#define RME_PGTBL_SIZE_2B            (1)
#define RME_PGTBL_SIZE_4B            (2)
#define RME_PGTBL_SIZE_8B            (3)
#define RME_PGTBL_SIZE_16B           (4)
#define RME_PGTBL_SIZE_32B           (5)
#define RME_PGTBL_SIZE_64B           (6)
#define RME_PGTBL_SIZE_128B          (7)
#define RME_PGTBL_SIZE_256B          (8)
#define RME_PGTBL_SIZE_512B          (9)
#define RME_PGTBL_SIZE_1K            (10)
#define RME_PGTBL_SIZE_2K            (11)
#define RME_PGTBL_SIZE_4K            (12)
#define RME_PGTBL_SIZE_8K            (13)
#define RME_PGTBL_SIZE_16K           (14)
#define RME_PGTBL_SIZE_32K           (15)
#define RME_PGTBL_SIZE_64K           (16)
#define RME_PGTBL_SIZE_128K          (17)
#define RME_PGTBL_SIZE_256K          (18)
#define RME_PGTBL_SIZE_512K          (19)
#define RME_PGTBL_SIZE_1M            (20)
#define RME_PGTBL_SIZE_2M            (21)
#define RME_PGTBL_SIZE_4M            (22)
#define RME_PGTBL_SIZE_8M            (23)
#define RME_PGTBL_SIZE_16M           (24)
#define RME_PGTBL_SIZE_32M           (25)
#define RME_PGTBL_SIZE_64M           (26)
#define RME_PGTBL_SIZE_128M          (27)
#define RME_PGTBL_SIZE_256M          (28)
#define RME_PGTBL_SIZE_512M          (29)
#define RME_PGTBL_SIZE_1G            (30)
#define RME_PGTBL_SIZE_2G            (31)
#define RME_PGTBL_SIZE_4G            (32)
#define RME_PGTBL_SIZE_8G            (33)
#define RME_PGTBL_SIZE_16G           (34)
#define RME_PGTBL_SIZE_32G           (35)
#define RME_PGTBL_SIZE_64G           (36)
#define RME_PGTBL_SIZE_128G          (37)
#define RME_PGTBL_SIZE_256G          (38)
#define RME_PGTBL_SIZE_512G          (39)
#define RME_PGTBL_SIZE_1T            (40)
#define RME_PGTBL_SIZE_2T            (41)
#define RME_PGTBL_SIZE_4T            (42)
#define RME_PGTBL_SIZE_8T            (43)
#define RME_PGTBL_SIZE_16T           (44)
#define RME_PGTBL_SIZE_32T           (45)
#define RME_PGTBL_SIZE_64T           (46)
#define RME_PGTBL_SIZE_128T          (47)
#define RME_PGTBL_SIZE_256T          (48)
#define RME_PGTBL_SIZE_512T          (49)
#define RME_PGTBL_SIZE_1P            (50)
#define RME_PGTBL_SIZE_2P            (51)
#define RME_PGTBL_SIZE_4P            (52)
#define RME_PGTBL_SIZE_8P            (53)
#define RME_PGTBL_SIZE_16P           (54)
#define RME_PGTBL_SIZE_32P           (55)
#define RME_PGTBL_SIZE_64P           (56)
#define RME_PGTBL_SIZE_128P          (57)
#define RME_PGTBL_SIZE_256P          (58)
#define RME_PGTBL_SIZE_512P          (59)
#define RME_PGTBL_SIZE_1E            (60)
#define RME_PGTBL_SIZE_2E            (61)
#define RME_PGTBL_SIZE_4E            (62)
#define RME_PGTBL_SIZE_8E            (63)
#define RME_PGTBL_SIZE_16E           (64)
#define RME_PGTBL_SIZE_32E           (65)
#define RME_PGTBL_SIZE_64E           (66)
#define RME_PGTBL_SIZE_128E          (67)
#define RME_PGTBL_SIZE_256E          (68)
#define RME_PGTBL_SIZE_512E          (69)
#define RME_PGTBL_SIZE_1Z            (70)

/* Generic page table entry number definitions */
#define RME_PGTBL_NUM_2             (1)
#define RME_PGTBL_NUM_4             (2)
#define RME_PGTBL_NUM_8             (3)
#define RME_PGTBL_NUM_16            (4)
#define RME_PGTBL_NUM_32            (5)
#define RME_PGTBL_NUM_64            (6)
#define RME_PGTBL_NUM_128           (7)
#define RME_PGTBL_NUM_256           (8)
#define RME_PGTBL_NUM_512           (9)
#define RME_PGTBL_NUM_1K            (10)
#define RME_PGTBL_NUM_2K            (11)
#define RME_PGTBL_NUM_4K            (12)
#define RME_PGTBL_NUM_8K            (13)
#define RME_PGTBL_NUM_16K           (14)
#define RME_PGTBL_NUM_32K           (15)
#define RME_PGTBL_NUM_64K           (16)
#define RME_PGTBL_NUM_128K          (17)
#define RME_PGTBL_NUM_256K          (18)
#define RME_PGTBL_NUM_512K          (19)
#define RME_PGTBL_NUM_1M            (20)
#define RME_PGTBL_NUM_2M            (21)
#define RME_PGTBL_NUM_4M            (22)
/* End Special Definitions ***************************************************/

/* Syystem Calls *************************************************************/
/* IPC activation ************************************************************/
/* Return from an invocation */
#define RME_SVC_INV_RET             0
/* Activate the invocation */
#define RME_SVC_INV_ACT             1
/* Send to a signal endpoint */
#define RME_SVC_SIG_SND             2
/* Receive from a signal endpoint */
#define RME_SVC_SIG_RCV             3
/* Kernel function calling ***************************************************/
#define RME_SVC_KERN                4
/* The operations that may cause a context switch ****************************/
/* Changing thread priority */
#define RME_SVC_THD_SCHED_PRIO      5
/* Free a thread from some core */
#define RME_SVC_THD_SCHED_FREE      6
/* Transfer time to a thread */
#define RME_SVC_THD_TIME_XFER       7
/* Switch to another thread */
#define RME_SVC_THD_SWT             8
/* Capability table operations ***********************************************/
/* Create */
#define RME_SVC_CAPTBL_CRT          9
/* Delete */
#define RME_SVC_CAPTBL_DEL          10
/* Freeze */
#define RME_SVC_CAPTBL_FRZ          11
/* Add */
#define RME_SVC_CAPTBL_ADD          12
/* Remove */
#define RME_SVC_CAPTBL_REM          13
/* Page table operations *****************************************************/
/* Create */
#define RME_SVC_PGTBL_CRT           14
/* Delete */
#define RME_SVC_PGTBL_DEL           15
/* Add */
#define RME_SVC_PGTBL_ADD           16
/* Remove */
#define RME_SVC_PGTBL_REM           17
/* Construction */
#define RME_SVC_PGTBL_CON           18
/* Destruction */
#define RME_SVC_PGTBL_DES           19
/* Process operations ********************************************************/
/* Create */
#define RME_SVC_PROC_CRT            20
/* Delete */
#define RME_SVC_PROC_DEL            21
/* Change captbl */
#define RME_SVC_PROC_CPT            22
/* Change pgtbl */ 
#define RME_SVC_PROC_PGT            23
/* Thread operations *********************************************************/
/* Create */
#define RME_SVC_THD_CRT             24
/* Delete */
#define RME_SVC_THD_DEL             25
/* Set entry&stack */
#define RME_SVC_THD_EXEC_SET        26
/* Set this as a hypervisor - managed thread */
#define RME_SVC_THD_HYP_SET         27
/* Bind to the current processor */
#define RME_SVC_THD_SCHED_BIND      28
/* Try to receive scheduling notifications */
#define RME_SVC_THD_SCHED_RCV       29
/* Signal operations *********************************************************/
/* Create */
#define RME_SVC_SIG_CRT             30
/* Delete */
#define RME_SVC_SIG_DEL             31
/* Invocation operations *****************************************************/
/* Create */
#define RME_SVC_INV_CRT             32
/* Delete */
#define RME_SVC_INV_DEL             33
/* Set entry&stack */
#define RME_SVC_INV_SET             34
/* End System Calls **********************************************************/
/* End Defines ***************************************************************/

#endif /* __RME_H__ */

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
