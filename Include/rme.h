/******************************************************************************
Filename    : rme.h
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The header of the RME RTOS. This header defines the error codes,
              operation flags and system call numbers in a generic way.
******************************************************************************/

#ifndef __RME__
#define __RME__
/* Define ********************************************************************/

/* Errors ********************************************************************/
/* The base of capability table error */
#define RME_ERR_CPT                     (0)
/* The capability is empty */
#define RME_ERR_CPT_NULL                ((-1)+RME_ERR_CPT)
/* The capability type is wrong */
#define RME_ERR_CPT_TYPE                ((-2)+RME_ERR_CPT)
/* The range of the capability is wrong */
#define RME_ERR_CPT_RANGE               ((-3)+RME_ERR_CPT)
/* The kernel object table operation of the capability is wrong */
#define RME_ERR_CPT_KOT                 ((-4)+RME_ERR_CPT)
/* The capability number already exists, or not exist, depending on the function */
#define RME_ERR_CPT_EXIST               ((-5)+RME_ERR_CPT)
/* When freezing capabilities, the refcnt is not zero, or when delegating, refcnt overflowed */
#define RME_ERR_CPT_REFCNT              ((-6)+RME_ERR_CPT)
/* The flags of the fine-grain-controlled capability is not correct */
#define RME_ERR_CPT_FLAG                ((-7)+RME_ERR_CPT)
/* Something involved is not quiescent */
#define RME_ERR_CPT_QUIE                ((-8)+RME_ERR_CPT)
/* Something involved is frozen */
#define RME_ERR_CPT_FROZEN              ((-9)+RME_ERR_CPT)
/* The capability is a root one and therefore cannot use removal */
#define RME_ERR_CPT_ROOT                ((-10)+RME_ERR_CPT)

/* The base of page table errors */
#define RME_ERR_PGT                     (-100)
/* Incorrect address */
#define RME_ERR_PGT_ADDR                ((-1)+RME_ERR_PGT)
/* Mapping failure */
#define RME_ERR_PGT_MAP                 ((-2)+RME_ERR_PGT)
/* Hardware restrictions */
#define RME_ERR_PGT_HW                  ((-3)+RME_ERR_PGT)
/* Wrong permissions */
#define RME_ERR_PGT_PERM                ((-4)+RME_ERR_PGT)

/* The base of process/thread errors */
#define RME_ERR_PTH                     (-200)
/* Incorrect address */
#define RME_ERR_PTH_PGT                 ((-1)+RME_ERR_PTH)
/* Conflicting operations happening at the same time */
#define RME_ERR_PTH_CONFLICT            ((-2)+RME_ERR_PTH)
/* Hypervisor register area address error */
#define RME_ERR_PTH_HADDR               ((-3)+RME_ERR_PTH)
/* Thread state is invalid or we have failed the CAS */
#define RME_ERR_PTH_INVSTATE            ((-4)+RME_ERR_PTH)
/* The thread priority is not correct */
#define RME_ERR_PTH_PRIO                ((-5)+RME_ERR_PTH)
/* The reference count not correct */
#define RME_ERR_PTH_REFCNT              ((-6)+RME_ERR_PTH)
/* Wrong notification target or thread ID */
#define RME_ERR_PTH_NOTIF               ((-7)+RME_ERR_PTH)
/* The time is full for this thread */
#define RME_ERR_PTH_OVERFLOW            ((-8)+RME_ERR_PTH)

/* The base of signal/invocation errors */
#define RME_ERR_SIV                     (-300)
/* This invocation capability/signal endpoint is already active, or wrong option on receive */
#define RME_ERR_SIV_ACT                 ((-1)+RME_ERR_SIV)
/* This invocation succeeded, but a fault happened in the thread, and we forced a invocation return */
#define RME_ERR_SIV_FAULT               ((-2)+RME_ERR_SIV)
/* The thread's signal counter is full */
#define RME_ERR_SIV_FULL                ((-3)+RME_ERR_SIV)
/* The thread's invocation stack is empty, cannot return */
#define RME_ERR_SIV_EMPTY               ((-4)+RME_ERR_SIV)
/* The signal capability is operated on by two threads simutaneously */
#define RME_ERR_SIV_CONFLICT            ((-5)+RME_ERR_SIV)
/* The signal receive system call ended with a result of free(forced unblock) */
#define RME_ERR_SIV_FREE                ((-6)+RME_ERR_SIV)
/* The signal receive failed because we are the boot-time thread */
#define RME_ERR_SIV_BOOT                ((-7)+RME_ERR_SIV)
/* End Errors ****************************************************************/

/* Operation Flags ***********************************************************/
/* Cpt */
/* This cap to captbl allows creating kernel objects in the captbl */
#define RME_CPT_FLAG_CRT                (1U<<0)
/* This cap to captbl allows deleting kernel objects in the captbl */
#define RME_CPT_FLAG_DEL                (1U<<1)
/* This cap to captbl allows freezing kernel objects in the captbl */
#define RME_CPT_FLAG_FRZ                (1U<<2)
/* This cap to captbl allows delegating kernel objects in it */
#define RME_CPT_FLAG_ADD_SRC            (1U<<3)
/* This cap to captbl allows receiving delegated kernel objects to it */
#define RME_CPT_FLAG_ADD_DST            (1U<<4)  
/* This cap to captbl allows removal operation in kernel objects(captbls) in the captbl */
#define RME_CPT_FLAG_REM                (1U<<5)
/* This cap to captbl allows itself to be used in process creation */
#define RME_CPT_FLAG_PRC_CRT            (1U<<6)
/* This cap to captbl allows itself to be used in process capability table replacement */
#define RME_CPT_FLAG_PRC_CPT            (1U<<7)
/* This cap to captbl allows all operations */
#define RME_CPT_FLAG_ALL                (RME_CPT_FLAG_CRT|RME_CPT_FLAG_DEL|RME_CPT_FLAG_FRZ| \
                                         RME_CPT_FLAG_ADD_SRC|RME_CPT_FLAG_ADD_DST|RME_CPT_FLAG_REM| \
                                         RME_CPT_FLAG_PRC_CRT|RME_CPT_FLAG_PRC_CPT)

/* Page table */
/* This cap to pgtbl allows delegating pages in it */
#define RME_PGT_FLAG_ADD_SRC            (1U<<0)
/* This cap to pgtbl allows receiving delegated pages to it */
#define RME_PGT_FLAG_ADD_DST            (1U<<1)
/* This cap to pgtbl allows removal of pages in it */
#define RME_PGT_FLAG_REM                (1U<<2) 
/* This cap to pgtbl allows to be mapped into other page tables as a child
 * or destructed from other page tables as a child */
#define RME_PGT_FLAG_CHILD              (1U<<3)
/* This cap to pgtbl allows accepting lower page table mappings */
#define RME_PGT_FLAG_CON_PARENT         (1U<<4)
/* This cap to pgtbl allows its lower level page table mappings to be destructed */
#define RME_PGT_FLAG_DES_PARENT         (1U<<5)
/* This cap to pgtbl allows itself to be used in process creation */
#define RME_PGT_FLAG_PRC_CRT            (1U<<6)
/* This cap to pgtbl allows itself to be used in process page table replacement */
#define RME_PGT_FLAG_PRC_PGT            (1U<<7)
/* This cap to pgtbl allows all operations */
#define RME_PGT_FLAG_ALL                (RME_PGT_FLAG_ADD_SRC|RME_PGT_FLAG_ADD_DST|RME_PGT_FLAG_REM| \
                                         RME_PGT_FLAG_CHILD|RME_PGT_FLAG_CON_PARENT|RME_PGT_FLAG_DES_PARENT| \
                                         RME_PGT_FLAG_PRC_CRT|RME_PGT_FLAG_PRC_PGT)

/* Kernel memory */
/* This cap to kernel memory allows creation of captbl */
#define RME_KOM_FLAG_CPT                (1U<<0)
/* This cap to kernel memory allows creation of pgtbl */
#define RME_KOM_FLAG_PGT                (1U<<1)
/* This cap to kernel memory allows creation of thread */
#define RME_KOM_FLAG_THD                (1U<<2)
/* This cap to kernel memory allows creation of invocation */
#define RME_KOM_FLAG_INV                (1U<<3)
/* This cap to kernel memory allows all operations */
#define RME_KOM_FLAG_ALL                (RME_KOM_FLAG_CPT|RME_KOM_FLAG_PGT| \
                                         RME_KOM_FLAG_THD|RME_KOM_FLAG_INV)

/* Process */
/* This cap to process allows creating invocation stubs in it */
#define RME_PRC_FLAG_INV                (1U<<0)
/* This cap to process allows creating threads in it */
#define RME_PRC_FLAG_THD                (1U<<1)
/* This cap to process allows changing its capability table */
#define RME_PRC_FLAG_CPT                (1U<<2)
/* This cap to process allows changing its page table */
#define RME_PRC_FLAG_PGT                (1U<<3)
/* This cap to process allows all operations */
#define RME_PRC_FLAG_ALL                (RME_PRC_FLAG_INV|RME_PRC_FLAG_THD| \
                                         RME_PRC_FLAG_CPT|RME_PRC_FLAG_PGT)

/* Thread */
/* This cap to thread allows setting its execution parameters */
#define RME_THD_FLAG_EXEC_SET           (1U<<0)
/* This cap to thread allows setting its scheduling parameters */
#define RME_THD_FLAG_SCHED_CHILD        (1U<<1)
/* This cap to thread allows registering it as a scheduler */
#define RME_THD_FLAG_SCHED_PARENT       (1U<<2)
/* This cap to thread allows changing its priority level */
#define RME_THD_FLAG_SCHED_PRIO         (1U<<3)
/* This cap to thread allows freeing the thread from the core */
#define RME_THD_FLAG_SCHED_FREE         (1U<<4)
/* This cap to thread allows getting scheduling notifications */
#define RME_THD_FLAG_SCHED_RCV          (1U<<5)
/* This cap to thread allows acting as a source for time transfer */
#define RME_THD_FLAG_XFER_SRC           (1U<<6)
/* This cap to thread allows acting as a destination for time transfer */
#define RME_THD_FLAG_XFER_DST           (1U<<7)
/* This cap to thread allows switching to it */
#define RME_THD_FLAG_SWT                (1U<<8)
/* This cap to thread allows all operations */
#define RME_THD_FLAG_ALL                (RME_THD_FLAG_EXEC_SET|RME_THD_FLAG_SCHED_CHILD| \
                                         RME_THD_FLAG_SCHED_PARENT|RME_THD_FLAG_SCHED_PRIO| \
                                         RME_THD_FLAG_SCHED_FREE|RME_THD_FLAG_SCHED_RCV| \
                                         RME_THD_FLAG_XFER_SRC|RME_THD_FLAG_XFER_DST|RME_THD_FLAG_SWT)
/* Signal */
/* This cap to signal endpoint allows sending to it */
#define RME_SIG_FLAG_SND                (1U<<0)
/* This cap to signal endpoint allows blocking single receive on it */
#define RME_SIG_FLAG_RCV_BS             (1U<<1)
/* This cap to signal endpoint allows blocking multi receive on it */
#define RME_SIG_FLAG_RCV_BM             (1U<<2)
/* This cap to signal endpoint allows non-blocking single receive on it */
#define RME_SIG_FLAG_RCV_NS             (1U<<3)
/* This cap to signal endpoint allows non-blocking multi receive on it */
#define RME_SIG_FLAG_RCV_NM             (1U<<4)
/* This cap to signal endpoint allows all receiving operations */
#define RME_SIG_FLAG_RCV                (RME_SIG_FLAG_RCV_BS|RME_SIG_FLAG_RCV_BM| \
                                         RME_SIG_FLAG_RCV_NS|RME_SIG_FLAG_RCV_NM)
/* This cap to signal endpoint allows sending scheduler notification to it */
#define RME_SIG_FLAG_SCHED              (1U<<5)
/* This cap to signal endpoint allows all operations */
#define RME_SIG_FLAG_ALL                (RME_SIG_FLAG_SND|RME_SIG_FLAG_RCV|RME_SIG_FLAG_SCHED)

/* Invocation */
/* This cap to invocation allows setting parameters for it */
#define RME_INV_FLAG_SET                (1U<<0)
/* This cap to invocation allows activating it */
#define RME_INV_FLAG_ACT                (1U<<1)
/* The return operation does not need a flag, nor does it need a capability */
/* This cap to invocation allows all operations */
#define RME_INV_FLAG_ALL                (RME_INV_FLAG_SET|RME_INV_FLAG_ACT)
/* End Operation Flags *******************************************************/

/* Special Definitions *******************************************************/
/* Generic page table flags */
/* This page allows to be read */
#define RME_PGT_READ                    (1U<<0)
/* This page allows to be written */
#define RME_PGT_WRITE                   (1U<<1)
/* This page allows execution */
#define RME_PGT_EXECUTE                 (1U<<2)
/* This page is cacheable */
#define RME_PGT_CACHE                   (1U<<3)
/* This page is bufferable (write-back can be used instead of write-through) */
#define RME_PGT_BUFFER                  (1U<<4)
/* This page is pinned in TLB */
#define RME_PGT_STATIC                  (1U<<5)
/* All the permissions are set */
#define RME_PGT_ALL_PERM                (RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE| \
                                         RME_PGT_CACHE|RME_PGT_BUFFER|RME_PGT_STATIC)
                                        
/* Generic page size order definitions */
#define RME_PGT_SIZE_2B                 (1U)
#define RME_PGT_SIZE_4B                 (2U)
#define RME_PGT_SIZE_8B                 (3U)
#define RME_PGT_SIZE_16B                (4U)
#define RME_PGT_SIZE_32B                (5U)
#define RME_PGT_SIZE_64B                (6U)
#define RME_PGT_SIZE_128B               (7U)
#define RME_PGT_SIZE_256B               (8U)
#define RME_PGT_SIZE_512B               (9U)
#define RME_PGT_SIZE_1K                 (10U)
#define RME_PGT_SIZE_2K                 (11U)
#define RME_PGT_SIZE_4K                 (12U)
#define RME_PGT_SIZE_8K                 (13U)
#define RME_PGT_SIZE_16K                (14U)
#define RME_PGT_SIZE_32K                (15U)
#define RME_PGT_SIZE_64K                (16U)
#define RME_PGT_SIZE_128K               (17U)
#define RME_PGT_SIZE_256K               (18U)
#define RME_PGT_SIZE_512K               (19U)
#define RME_PGT_SIZE_1M                 (20U)
#define RME_PGT_SIZE_2M                 (21U)
#define RME_PGT_SIZE_4M                 (22U)
#define RME_PGT_SIZE_8M                 (23U)
#define RME_PGT_SIZE_16M                (24U)
#define RME_PGT_SIZE_32M                (25U)
#define RME_PGT_SIZE_64M                (26U)
#define RME_PGT_SIZE_128M               (27U)
#define RME_PGT_SIZE_256M               (28U)
#define RME_PGT_SIZE_512M               (29U)
#define RME_PGT_SIZE_1G                 (30U)
#define RME_PGT_SIZE_2G                 (31U)
#define RME_PGT_SIZE_4G                 (32U)
#define RME_PGT_SIZE_8G                 (33U)
#define RME_PGT_SIZE_16G                (34U)
#define RME_PGT_SIZE_32G                (35U)
#define RME_PGT_SIZE_64G                (36U)
#define RME_PGT_SIZE_128G               (37U)
#define RME_PGT_SIZE_256G               (38U)
#define RME_PGT_SIZE_512G               (39U)
#define RME_PGT_SIZE_1T                 (40U)
#define RME_PGT_SIZE_2T                 (41U)
#define RME_PGT_SIZE_4T                 (42U)
#define RME_PGT_SIZE_8T                 (43U)
#define RME_PGT_SIZE_16T                (44U)
#define RME_PGT_SIZE_32T                (45U)
#define RME_PGT_SIZE_64T                (46U)
#define RME_PGT_SIZE_128T               (47U)
#define RME_PGT_SIZE_256T               (48U)
#define RME_PGT_SIZE_512T               (49U)
#define RME_PGT_SIZE_1P                 (50U)
#define RME_PGT_SIZE_2P                 (51U)
#define RME_PGT_SIZE_4P                 (52U)
#define RME_PGT_SIZE_8P                 (53U)
#define RME_PGT_SIZE_16P                (54U)
#define RME_PGT_SIZE_32P                (55U)
#define RME_PGT_SIZE_64P                (56U)
#define RME_PGT_SIZE_128P               (57U)
#define RME_PGT_SIZE_256P               (58U)
#define RME_PGT_SIZE_512P               (59U)
#define RME_PGT_SIZE_1E                 (60U)
#define RME_PGT_SIZE_2E                 (61U)
#define RME_PGT_SIZE_4E                 (62U)
#define RME_PGT_SIZE_8E                 (63U)
#define RME_PGT_SIZE_16E                (64U)
#define RME_PGT_SIZE_32E                (65U)
#define RME_PGT_SIZE_64E                (66U)
#define RME_PGT_SIZE_128E               (67U)
#define RME_PGT_SIZE_256E               (68U)
#define RME_PGT_SIZE_512E               (69U)
#define RME_PGT_SIZE_1Z                 (70U)

/* Generic page table entry number definitions */
#define RME_PGT_NUM_1                   (0U)
#define RME_PGT_NUM_2                   (1U)
#define RME_PGT_NUM_4                   (2U)
#define RME_PGT_NUM_8                   (3U)
#define RME_PGT_NUM_16                  (4U)
#define RME_PGT_NUM_32                  (5U)
#define RME_PGT_NUM_64                  (6U)
#define RME_PGT_NUM_128                 (7U)
#define RME_PGT_NUM_256                 (8U)
#define RME_PGT_NUM_512                 (9U)
#define RME_PGT_NUM_1K                  (10U)
#define RME_PGT_NUM_2K                  (11U)
#define RME_PGT_NUM_4K                  (12U)
#define RME_PGT_NUM_8K                  (13U)
#define RME_PGT_NUM_16K                 (14U)
#define RME_PGT_NUM_32K                 (15U)
#define RME_PGT_NUM_64K                 (16U)
#define RME_PGT_NUM_128K                (17U)
#define RME_PGT_NUM_256K                (18U)
#define RME_PGT_NUM_512K                (19U)
#define RME_PGT_NUM_1M                  (20U)
#define RME_PGT_NUM_2M                  (21U)
#define RME_PGT_NUM_4M                  (22U)

/* Receive options */
#define RME_RCV_BS                      (0U)
#define RME_RCV_BM                      (1U)
#define RME_RCV_NS                      (2U)
#define RME_RCV_NM                      (3U)
/* End Special Definitions ***************************************************/

/* Syystem Calls *************************************************************/
/* IPC activation ************************************************************/
/* Return from an invocation */
#define RME_SVC_INV_RET                 (0U)
/* Activate the invocation */
#define RME_SVC_INV_ACT                 (1U)
/* Send to a signal endpoint */
#define RME_SVC_SIG_SND                 (2U)
/* Receive from a signal endpoint */
#define RME_SVC_SIG_RCV                 (3U)
/* Kernel function calling ***************************************************/
#define RME_SVC_KFN                     (4U)
/* The operations that may cause a context switch ****************************/
/* Free a thread from some core */
#define RME_SVC_THD_SCHED_FREE          (5U)
/* Set entry&stack */
#define RME_SVC_THD_EXEC_SET            (6U)
/* Changing thread priority */
#define RME_SVC_THD_SCHED_PRIO          (7U)
/* Transfer time to a thread */
#define RME_SVC_THD_TIME_XFER           (8U)
/* Switch to another thread */
#define RME_SVC_THD_SWT                 (9U)
/* Capability table operations ***********************************************/
/* Create */
#define RME_SVC_CPT_CRT                 (10U)
/* Delete */
#define RME_SVC_CPT_DEL                 (11U)
/* Freeze */
#define RME_SVC_CPT_FRZ                 (12U)
/* Add */
#define RME_SVC_CPT_ADD                 (13U)
/* Remove */
#define RME_SVC_CPT_REM                 (14U)
/* Page table operations *****************************************************/
/* Create */
#define RME_SVC_PGT_CRT                 (15U)
/* Delete */
#define RME_SVC_PGT_DEL                 (16U)
/* Add */
#define RME_SVC_PGT_ADD                 (17U)
/* Remove */
#define RME_SVC_PGT_REM                 (18U)
/* Construction */
#define RME_SVC_PGT_CON                 (19U)
/* Destruction */
#define RME_SVC_PGT_DES                 (20U)
/* Process operations ********************************************************/
/* Create */
#define RME_SVC_PRC_CRT                 (21U)
/* Delete */
#define RME_SVC_PRC_DEL                 (22U)
/* Change captbl */
#define RME_SVC_PRC_CPT                 (23U)
/* Change pgtbl */ 
#define RME_SVC_PRC_PGT                 (24U)
/* Thread operations *********************************************************/
/* Create */
#define RME_SVC_THD_CRT                 (25U)
/* Delete */
#define RME_SVC_THD_DEL                 (26U)
/* Bind to the current processor */
#define RME_SVC_THD_SCHED_BIND          (27U)
/* Try to receive scheduling notifications */
#define RME_SVC_THD_SCHED_RCV           (28U)
/* Signal operations *********************************************************/
/* Create */
#define RME_SVC_SIG_CRT                 (29U)
/* Delete */
#define RME_SVC_SIG_DEL                 (30U)
/* Invocation operations *****************************************************/
/* Create */
#define RME_SVC_INV_CRT                 (31U)
/* Delete */
#define RME_SVC_INV_DEL                 (32U)
/* Set entry&stack */
#define RME_SVC_INV_SET                 (33U)
/* End System Calls **********************************************************/

/* Kernel Functions **********************************************************/
/* Page table operations *****************************************************/
/* Clear the whole TLB */
#define RME_KFN_PGT_CACHE_CLR           (0xF000U)
/* Clear a single TLB line */
#define RME_KFN_PGT_LINE_CLR            (0xF001U)
/* Set the ASID of a page table */
#define RME_KFN_PGT_ASID_SET            (0xF002U)
/* Lock a page into the TLB */
#define RME_KFN_PGT_TLB_LOCK            (0xF003U)
/* Query or modify the content of an entry */
#define RME_KFN_PGT_ENTRY_MOD           (0xF004U)
/* Interrupt controller operations *******************************************/
/* Modify local interrupt controller */
#define RME_KFN_INT_LOCAL_MOD           (0xF100U)
/* Modify global interrupt controller */
#define RME_KFN_INT_GLOBAL_MOD          (0xF101U)
/* Trigger a local interrupt */
#define RME_KFN_INT_LOCAL_TRIG          (0xF102U)
/* Trigger a local event */
#define RME_KFN_EVT_LOCAL_TRIG          (0xF103U)
/* Cache operations **********************************************************/
/* Modify cache state */
#define RME_KFN_CACHE_MOD               (0xF200U)
/* Configure cache */
#define RME_KFN_CACHE_CONFIG            (0xF201U)
/* Invalidate cache */
#define RME_KFN_CACHE_MAINT             (0xF202U)
/* Lock cache */
#define RME_KFN_CACHE_LOCK              (0xF203U)
/* Modify prefetcher state */
#define RME_KFN_PRFTH_MOD               (0xF204U)
/* Hot plug and pull operations **********************************************/
/* Modify physical CPU configuration */
#define RME_KFN_HPNP_PCPU_MOD           (0xF300U)
/* Modify logical CPU configuration */
#define RME_KFN_HPNP_LCPU_MOD           (0xF301U)
/* Modify physical memory configuration */
#define RME_KFN_HPNP_PMEM_MOD           (0xF302U)
/* Power and frequency adjustment operations *********************************/
/* Put CPU into idle sleep mode */
#define RME_KFN_IDLE_SLEEP              (0xF400U)
/* Reboot the whole system */
#define RME_KFN_SYS_REBOOT              (0xF401U)
/* Shutdown the whole system */
#define RME_KFN_SYS_SHDN                (0xF402U)
/* Modify voltage configuration */
#define RME_KFN_VOLT_MOD                (0xF403U)
/* Modify frequency configuration */
#define RME_KFN_FREQ_MOD                (0xF404U)
/* Modify power state */
#define RME_KFN_PMOD_MOD                (0xF405U)
/* Modify safety lock state */
#define RME_KFN_SAFETY_MOD              (0xF406U)
/* Performance monitoring operations *****************************************/
/* Query or modify CPU function configuration */
#define RME_KFN_PERF_CPU_FUNC           (0xF500U)
/* Query or modify performance monitor configuration */
#define RME_KFN_PERF_MON_MOD            (0xF501U)
/* Query or modify counting performance monitor register */
#define RME_KFN_PERF_CNT_MOD            (0xF502U)
/* Query or modify clock cycle performance monitor register */
#define RME_KFN_PERF_CYCLE_MOD          (0xF503U)
/* Query or modify data performance monitor register */
#define RME_KFN_PERF_DATA_MOD           (0xF504U)
/* Query or modify physical monitor register */
#define RME_KFN_PERF_PHYS_MOD           (0xF505U)
/* Query or modify cumulative monitor register */
#define RME_KFN_PERF_CUMUL_MOD          (0xF506U)
/* Hardware virtualization operations ****************************************/
/* Create a virtual machine */
#define RME_KFN_VM_CRT                  (0xF600U)
/* Delete a virtual machine */
#define RME_KFN_VM_DEL                  (0xF601U)
/* Assign a user-level page table to the virtual machine */
#define RME_KFN_VM_PGT                  (0xF602U)
/* Query or modify virtual machine state */
#define RME_KFN_VM_MOD                  (0xF603U)
/* Create a virtual CPU */
#define RME_KFN_VCPU_CRT                (0xF604U)
/* Bind a virtual CPU to a virtual machine */
#define RME_KFN_VCPU_BIND               (0xF605U)
/* Free a virtual CPU from a virtual machine */
#define RME_KFN_VCPU_FREE               (0xF606U)
/* Delete a virtual CPU */
#define RME_KFN_VCPU_DEL                (0xF607U)
/* Query or modify virtual registers */
#define RME_KFN_VCPU_MOD                (0xF608U)
/* Run the VCPU on this thread */
#define RME_KFN_VCPU_RUN                (0xF609U)
/* Security monitor operations ***********************************************/
/* Create an enclave */
#define RME_KFN_ECLV_CRT                (0xF700U)
/* Query or modify an enclave */
#define RME_KFN_ECLV_MOD                (0xF701U)
/* Delete an enclave */
#define RME_KFN_ECLV_DEL                (0xF702U)
/* Call into an enclave */
#define RME_KFN_ECLV_ACT                (0xF703U)
/* Return from an enclave */
#define RME_KFN_ECLV_RET                (0xF704U)
/* Debugging operations ******************************************************/
/* Debug printing - a single character or a series of characters */
#define RME_KFN_DEBUG_PRINT             (0xF800U)
/* Modify thread register content */
#define RME_KFN_DEBUG_REG_MOD           (0xF801U)
/* Modify thread invocation register content */
#define RME_KFN_DEBUG_INV_MOD           (0xF802U)
/* Get thread exception register content */
#define RME_KFN_DEBUG_EXC_GET           (0xF803U)
/* Modify debug engine configuration */
#define RME_KFN_DEBUG_MODE_MOD          (0xF804U)
/* Modify instruction breakpoint state */
#define RME_KFN_DEBUG_IBP_MOD           (0xF805U)
/* Modify data breakpoint state */
#define RME_KFN_DEBUG_DBP_MOD           (0xF806U)
/* End Kernel Functions ******************************************************/
/* End Define ****************************************************************/

#endif /* __RME__ */

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
