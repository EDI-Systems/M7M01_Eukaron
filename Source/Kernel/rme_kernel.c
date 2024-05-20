/******************************************************************************
Filename    : rme_kernel.c
Author      : pry
Date        : 23/03/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The system call processing path, debugging primitives and kernel
              capability implementation for the RME system.

* Generic Code Section ********************************************************
Provides some utilities and handlers that every operating system have.

* Capability Table Code Section ***********************************************
This section of code might be confusing if the underlying principles
of capability-based systems is not well understood. 
1. Owning a capability means you have the power to use the function
   represented by that capability.
2. Capabilities have an field called flags, which denotes what operations
   is allowed on that captbl.
3. Owning a capability to a capability table means that you have the
   power to modify the capability table's contents.
4. Creation and deletion of kernel objects is an operation on capability
   table, thus requiring the capability to the capability table.

Remember we do not check our master table to see if it is frozen, or if it is
captbl, or something, because if the master table's cap is not explicitly 
passed in, we do not operate on it at all; If it is explicitly passed in, it
will be checked.

There are 4 basic types of operations, as listed below:
Operation                     What it does
-------------------------------------------------------------------------------
Create/Add-Dst                CAS the slot to CREATING state.
                              Update timestamp.
                              Create kernel object.
                              Atomically update header to complete creation.
Use/Add-Src                   Use the kernel object, have a WCET.
Freeze                        Check timestamp for create-freeze QUIESCENCE.
                              Update timestamp.
                              CAS the slot to FROZEN state.
Delete/Removal                Check FROZEN.
                              Check timestamp for freeze-delete QUIESCENCE.
                              Check REFCNT (delete only).
                              CAS the slot to empty.
                              Delete the kernel object (delete only).

Hazard Table: (Operation 2 follows Operation 1)
Operation 1    Operation 2    Reason why it is safe
-------------------------------------------------------------------------------
Create         Create         Only one creation will be successful, because 
                              CREATING slot is done by CAS.
Create         Delete         Create only set the CREATING. Delete will require
                              a TYPE data, which will only be set after the 
                              creation completes. ABA problem cannot occur
                              because of create-freeze quiescence.
                              If there is no quiescence between Create-Freeze, 
                              the following may occur:
                              T1: Check ---------------------------- Delete(CAS)
                              T2: Check - Delete - Create - Freeze -------------
                              In this case, thread 1 will perform a wrong 
                              deletion on the new capability (the CAS will be
                              successful), but this cap is actually a new cap 
                              created by the thread 2 at the same location, not
                              the old cap, and its quiescence may not be satisfied.
Create         Freeze         OCCUPY only set the FROZEN. FROZEN will require
                              that bit not set.
Create         Add-Src        Add-Src will require a TYPE data, which will only
                              be set after the creation completes.
Create         Add-Dst        Only one creation will be successful, because 
                              OCCUPY slot is done by CAS.
Create         Remove         OCCUPY only set the FROZEN. Remove will require a
                              TYPE data, which will only be set after the
                              creation completes. See Create-Delete for details.
Create         Use            OCCUPY only set the FROZEN. Use the cap will 
                              require a TYPE data, which will only be set after
                              the creation completes.
-------------------------------------------------------------------------------
Delete         Delete         Actual deletion done by CAS so only one deletion
                              will be successful.
Delete         Freeze         If the deletion fails and clears the FROZEN flag,
                              nothing will be done;
                              If it does not fail, then the cap will be erased,
                              and the FREEZE CAS will not succeed.
Delete         Remove         Only one will be successful because only the root
                              can be DELETED.
Delete         Others         Banned by the FROZEN flag before deletion. 
-------------------------------------------------------------------------------
Freeze         Create         Create fails because slot is still OCCUPY.
Freeze         Delete         Delete fails if not FROZEN, or not QUIESCENT.
Freeze         Remove         Remove fails if not FROZEN, or not QUIESCENT.
Freeze         Freeze         Freeze done by CAS, and only one will be successful.
Freeze         Others         Freeze bans them if they attempt after FROZEN set.
-------------------------------------------------------------------------------
Add-Src        Create         Impossible because something in that slot.
Add-Src        Freeze         Cannot freeze if already increased REFCNT. If they
                              increase REFCNT just after FROZEN set, let it be.
                              The cap can't be deleted because of non-zero REFCNT.
Add-Src        Delete         Impossible because cap not FROZEN.
Add-Src        Remove         Impossible because cap not FROZEN.
Add-Src        Others         These operations can be parallel, which is fine.
-------------------------------------------------------------------------------
Add-Dst         ...           Conclusion same as Create operation.
-------------------------------------------------------------------------------
Remove          ...           Conclusion same as Delete operation.
-------------------------------------------------------------------------------
Use            Create         Impossible because something in that slot.
Use            Delete         Impossible because not FROZEN. The use can't be
                              from leaf caps as well because deletion will check
                              the REFCNT, and if the REFCNT is 0, then the only
                              case where an unsettled use can happen is that it
                              happens within WCET time to REFCNT check time. This
                              unsettled use must come from a leaf cap, as the use
                              happened after the root gets FROZEN. This leaf cap
                              itself, will set the REFCNT to 1, and it have no
                              chance to freeze then remove itself before a WCET.
                              The unsettled use case is thus impossible and there
                              is no race. As long as all new reference to caps
                              require an active cap passed in, there is no such
                              race. Also, for cap creation, the header create step 
                              must be the last step (after refcnt can be seen on
                              all cores as we use write release semantics), and
                              this ensures that no new leaf caps will be available
                              for use before REFCNT is seen by all cores.
Use            Freeze         It is fine.
Use            Add-Src        It is fine.
Use            Add-Dst        Impossible because something in that slot.
Use            Remove         Impossible because not FROZEN.
Use            Use            It is fine.

* Page Table Code Section *****************************************************
Different from most large-scale operating systems, RME requires the page tables
to be constructed by the user-level rather than kernel logic. Yet, RME provided
sufficient utilities for the user to conduct the necessary paging operations:

1> Creating page directories;
2> Deletiing page directories;
3> Adding(mapping) pages into page directories;
4> Deleting(unmapping) pages from page directories.
5> Constructing hierachical page tables;
6> Destructing hierachical page tables.

* Kernel Memory Code Section **************************************************
Different seL4 and Composite, RME applys a principle that resembles Fiasco.OC's
kernel object factory. However, this is simplified very much in RME - just a table
for registering kernel memory usage! Some may think that this prohibits retyping;
this is not true because we can implement it with a trusted user-level proxy.

* Process and Thread Code Section *********************************************
RME provided process as a light-weight virtual machine/container abstraction, 
and a versatile thread abstraction. Processes enforce isolation, while threads
carry out the task.
There is no bind-bind race because bind is done using CAS.
There is no bind-unbind race for scheduler thread because all are core-local.

* Signal and Invocation Code Section ******************************************
RME employs simple signal endpoints for interrupt passing, inter-core interrupt
and asynchronous communication. Different from most operating systems, it employs
thread migration model for cross-boundary synchronous communication rather than
simple blockpoints. This invocation design have many benefits in many facets.

* Kernel Function Code Section ************************************************
There's no perfect operating system for a particular hardware. A hardware may
have its own idiosyncrasies that needs extra hacks. RME's kernel function utility
provides a disciplined way of making such hacks, in case you need to add new 
system calls or directly manipulate hardware.

* The Use of 'volatile' *******************************************************
'volatile' is not needed in the kernel because the syscall interface acts as a
natural compiler barrier. We're safe to assume that, during one syscall, data
in memory remains unchanged. If anything changes, it has been dealt with the 
dedicated assembly atomics. If LTO has been enabled, there are three cases:
(1) Uniprocessor with C-implemented "atomics" with no real atomic support.
    In this case, compiler barriers are not needed due to no kernel concurrency.
(2) Multiprocessor with assembly-implemented atomics, but the compiler LTO does
    not honor the assembly functions; instead it thinks that they are opaque.
    In this case, the opaque function call itself is a full compiler barrier.
(3) Multiprocessor with assembly-implemented atomics, and the compiler LTO does
    honor the assembly functions.
    In this case, the compiler should be aware of the semantics of the assembly,
    and produce correct code.
* Function Name Rules *********************************************************
(1) No "_": OS entry function RME_Kmain and C entry "main".
(2) "_"   : Kernel functions that are be called by kernel.
(3) "__"  : HAL functions that are called by kernel.
(4) "___" : HAL functions that should be called by HAL.
******************************************************************************/

/* Include *******************************************************************/
#define __HDR_DEF__
#include "rme_platform.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_DEF__

#define __HDR_STRUCT__
#include "rme_platform.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_STRUCT__

/* Private include */
#include "Kernel/rme_kernel.h"

#define __HDR_PUBLIC__
#include "rme_platform.h"
#undef __HDR_PUBLIC__
/* End Include ***************************************************************/

/* Function:_RME_MSB_Generic **************************************************
Description : Find the MSB's position. This is a portable solution for all
              processors; if your processor does not have fast built-in bit
              manipulation support, you can resort to this when porting.
Input       : rme_ptr_t Value - The value to compute for.
Output      : None.
Return      : rme_ptr_t - The result. 0 will be returned for 0.
******************************************************************************/
rme_ptr_t _RME_MSB_Generic(rme_ptr_t Value)
{
    rme_ptr_t Bit;
    static const rme_u8_t Table[256U]=
    {
        0U,0U,1U,1U,2U,2U,2U,2U,3U,3U,3U,3U,3U,3U,3U,3U,
        4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,4U,
        5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,
        5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,5U,
        6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,
        6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,
        6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,
        6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,6U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,
        7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U,7U
    };

#if(RME_WORD_ORDER==4U)
    /* 15-8 */
    if(Value>=RME_POW2(8U))
    {
        RME_COVERAGE_MARKER();
        Bit=8U;
    }
    /* 7-0 */
    else
    {
        RME_COVERAGE_MARKER();
        Bit=0U;
    }
#elif(RME_WORD_ORDER==5U)
    /* 31-16 */
    if(Value>=RME_POW2(16U))
    {
        RME_COVERAGE_MARKER();
        /* 31-24 */
        if(Value>=RME_POW2(24U))
        {
            RME_COVERAGE_MARKER();
            Bit=24U;
        }
        /* 24-16 */
        else
        {
            RME_COVERAGE_MARKER();
            Bit=16U;
        }
    }
    /* 15-0 */
    else
    {
        RME_COVERAGE_MARKER();
        /* 15-8 */
        if(Value>=RME_POW2(8U))
        {
            RME_COVERAGE_MARKER();
            Bit=8U;
        }
        /* 7-0 */
        else
        {
            RME_COVERAGE_MARKER();
            Bit=0U;
        }
    }
#elif(RME_WORD_ORDER==6U)
    /* 63-32 */
    if(Value>=RME_POW2(32U))
    {
        RME_COVERAGE_MARKER();
        /* 63-48 */
        if(Value>=RME_POW2(48U))
        {
            RME_COVERAGE_MARKER();
            /* 63-56 */
            if(Value>=RME_POW2(56U))
            {
                RME_COVERAGE_MARKER();
                Bit=56U;
            }
            /* 56-48 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=48U;
            }
        }
        /* 47-32 */
        else
        {
            RME_COVERAGE_MARKER();
            /* 47-40 */
            if(Value>=RME_POW2(40U))
            {
                RME_COVERAGE_MARKER();
                Bit=40U;
            }
            /* 39-32 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=32U;
            }
        }
    }
    /* 31-0 */
    else
    {
        RME_COVERAGE_MARKER();
        /* 31-16 */
        if(Value>=RME_POW2(16U))
        {
            RME_COVERAGE_MARKER();
            /* 31-24 */
            if(Value>=RME_POW2(24U))
            {
                RME_COVERAGE_MARKER();
                Bit=24U;
            }
            /* 24-16 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=16U;
            }
        }
        /* 15-0 */
        else
        {
            RME_COVERAGE_MARKER();
            /* 15-8 */
            if(Value>=RME_POW2(8U))
            {
                RME_COVERAGE_MARKER();
                Bit=8U;
            }
            /* 7-0 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=0U;
            }
        }
    }
#else
#error Generic MSB for 128-bits & above are not implemented.
#endif

    return Table[Value>>Bit]+Bit;
}
/* End Function:_RME_MSB_Generic *********************************************/

/* Function:_RME_LSB_Generic **************************************************
Description : Find the LSB's position. This is a portable solution for all
              processors; if your processor does not have fast built-in bit
              manipulation support, you can resort to this when porting.
Input       : rme_ptr_t Value - The value to count.
Output      : None.
Return      : rme_ptr_t - The result. 0 will be returned for 0.
******************************************************************************/
rme_ptr_t _RME_LSB_Generic(rme_ptr_t Value)
{
    rme_ptr_t Bit;
    static const rme_u8_t Table[256U]=
    {
        0U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        5U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        6U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        5U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        7U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        5U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        6U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        5U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U,
        4U,0U,1U,0U,2U,0U,1U,0U,3U,0U,1U,0U,2U,0U,1U,0U
    };
    
#if(RME_WORD_ORDER==4U)
    /* 16-8 */
    if((Value<<8U)==0U)
    {
        RME_COVERAGE_MARKER();
        Bit=8U;
    }
    /* 7-0 */
    else
    {
        RME_COVERAGE_MARKER();
        Bit=0U;
    }
#elif(RME_WORD_ORDER==5U)
    /* 31-16 */
    if((Value<<16U)==0U)
    {
        RME_COVERAGE_MARKER();
        /* 31-24 */
        if((Value<<8U)==0U)
        {
            RME_COVERAGE_MARKER();
            Bit=24U;
        }
        /* 24-16 */
        else
        {
            RME_COVERAGE_MARKER();
            Bit=16U;
        }
    }
    /* 15-0 */
    else
    {
        RME_COVERAGE_MARKER();
        /* 15-8 */
        if((Value<<24U)==0U)
        {
            RME_COVERAGE_MARKER();
            Bit=8U;
        }
        /* 7-0 */
        else
        {
            RME_COVERAGE_MARKER();
            Bit=0U;
        }
    }
#elif(RME_WORD_ORDER==6U)
    /* 63-32 */
    if((Value<<32U)==0U)
    {
        RME_COVERAGE_MARKER();
        /* 63-48 */
        if((Value<<16U)==0U)
        {
            RME_COVERAGE_MARKER();
            /* 63-56 */
            if((Value<<8U)==0U)
            {
                RME_COVERAGE_MARKER();
                Bit=56U;
            }
            /* 56-48 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=48U;
            }
        }
        /* 47-32 */
        else
        {
            RME_COVERAGE_MARKER();
            /* 47-40 */
            if((Value<<24U)==0U)
            {
                RME_COVERAGE_MARKER();
                Bit=40U;
            }
            /* 39-32 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=32U;
            }
        }
    }
    /* 31-0 */
    else
    {
        RME_COVERAGE_MARKER();
        /* 31-16 */
        if((Value<<48U)==0U)
        {
            RME_COVERAGE_MARKER();
            /* 31-24 */
            if((Value<<40U)==0U)
            {
                RME_COVERAGE_MARKER();
                Bit=24U;
            }
            /* 24-16 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=16U;
            }
        }
        /* 15-0 */
        else
        {
            RME_COVERAGE_MARKER();
            /* 15-8 */
            if((Value<<56U)==0U)
            {
                RME_COVERAGE_MARKER();
                Bit=8U;
            }
            /* 7-0 */
            else
            {
                RME_COVERAGE_MARKER();
                Bit=0U;
            }
        }
    }
#else
#error Generic LSB for 128-bits & above are not implemented.
#endif

    return Table[(rme_u8_t)(Value>>Bit)]+Bit;
}
/* End Function:_RME_LSB_Generic *********************************************/

/* Function:_RME_RBT_Generic **************************************************
Description : Reverse bit order. This is a portable solution meant for all
              processors; if your processor does not have fast built-in RBT,
              you can resort to this instead.
              This is implemented to be as fast as possible.
Input       : rme_ptr_t Value - The value to count.
Output      : None.
Return      : rme_ptr_t - The result.
******************************************************************************/
rme_ptr_t _RME_RBT_Generic(rme_ptr_t Value)
{
    rme_ptr_t Ret;
    rme_ptr_t Src;
    rme_u8_t* To;
    rme_u8_t* From;

    static const rme_u8_t Table[256U]=
    {
        0x00U,0x80U,0x40U,0xC0U,0x20U,0xA0U,0x60U,0xE0U,
        0x10U,0x90U,0x50U,0xD0U,0x30U,0xB0U,0x70U,0xF0U,
        0x08U,0x88U,0x48U,0xC8U,0x28U,0xA8U,0x68U,0xE8U,
        0x18U,0x98U,0x58U,0xD8U,0x38U,0xB8U,0x78U,0xF8U,
        0x04U,0x84U,0x44U,0xC4U,0x24U,0xA4U,0x64U,0xE4U,
        0x14U,0x94U,0x54U,0xD4U,0x34U,0xB4U,0x74U,0xF4U,
        0x0CU,0x8CU,0x4CU,0xCCU,0x2CU,0xACU,0x6CU,0xECU,
        0x1CU,0x9CU,0x5CU,0xDCU,0x3CU,0xBCU,0x7CU,0xFCU,
        0x02U,0x82U,0x42U,0xC2U,0x22U,0xA2U,0x62U,0xE2U,
        0x12U,0x92U,0x52U,0xD2U,0x32U,0xB2U,0x72U,0xF2U,
        0x0AU,0x8AU,0x4AU,0xCAU,0x2AU,0xAAU,0x6AU,0xEAU,
        0x1AU,0x9AU,0x5AU,0xDAU,0x3AU,0xBAU,0x7AU,0xFAU,
        0x06U,0x86U,0x46U,0xC6U,0x26U,0xA6U,0x66U,0xE6U,
        0x16U,0x96U,0x56U,0xD6U,0x36U,0xB6U,0x76U,0xF6U,
        0x0EU,0x8EU,0x4EU,0xCEU,0x2EU,0xAEU,0x6EU,0xEEU,
        0x1EU,0x9EU,0x5EU,0xDEU,0x3EU,0xBEU,0x7EU,0xFEU,
        0x01U,0x81U,0x41U,0xC1U,0x21U,0xA1U,0x61U,0xE1U,
        0x11U,0x91U,0x51U,0xD1U,0x31U,0xB1U,0x71U,0xF1U,
        0x09U,0x89U,0x49U,0xC9U,0x29U,0xA9U,0x69U,0xE9U,
        0x19U,0x99U,0x59U,0xD9U,0x39U,0xB9U,0x79U,0xF9U,
        0x05U,0x85U,0x45U,0xC5U,0x25U,0xA5U,0x65U,0xE5U,
        0x15U,0x95U,0x55U,0xD5U,0x35U,0xB5U,0x75U,0xF5U,
        0x0DU,0x8DU,0x4DU,0xCDU,0x2DU,0xADU,0x6DU,0xEDU,
        0x1DU,0x9DU,0x5DU,0xDDU,0x3DU,0xBDU,0x7DU,0xFDU,
        0x03U,0x83U,0x43U,0xC3U,0x23U,0xA3U,0x63U,0xE3U,
        0x13U,0x93U,0x53U,0xD3U,0x33U,0xB3U,0x73U,0xF3U,
        0x0BU,0x8BU,0x4BU,0xCBU,0x2BU,0xABU,0x6BU,0xEBU,
        0x1BU,0x9BU,0x5BU,0xDBU,0x3BU,0xBBU,0x7BU,0xFBU,
        0x07U,0x87U,0x47U,0xC7U,0x27U,0xA7U,0x67U,0xE7U,
        0x17U,0x97U,0x57U,0xD7U,0x37U,0xB7U,0x77U,0xF7U,
        0x0FU,0x8FU,0x4FU,0xCFU,0x2FU,0xAFU,0x6FU,0xEFU,
        0x1FU,0x9FU,0x5FU,0xDFU,0x3FU,0xBFU,0x7FU,0xFFU
    };

    Src=Value;
    To=(rme_u8_t*)(&Ret);
    From=(rme_u8_t*)(&Src);

#if(RME_WORD_ORDER==4U)
    To[0]=Table[From[1]];
    To[1]=Table[From[0]];
#elif(RME_WORD_ORDER==5U)
    To[0]=Table[From[3]];
    To[1]=Table[From[2]];
    To[2]=Table[From[1]];
    To[3]=Table[From[0]];
#elif(RME_WORD_ORDER==6U)
    To[0]=Table[From[7]];
    To[1]=Table[From[6]];
    To[2]=Table[From[5]];
    To[3]=Table[From[4]];
    To[4]=Table[From[3]];
    To[5]=Table[From[2]];
    To[6]=Table[From[1]];
    To[7]=Table[From[0]];
#else
#error Generic RBT for 128-bits & above are not implemented.
#endif

    return Ret;
}
/* End Function:_RME_RBT_Generic *********************************************/

/* Function:_RME_Comp_Swap_Single *********************************************
Description : The compare-and-swap atomic instruction. If the Old value is
              equal to *Ptr, then set the *Ptr as New and return 1; else return
              0.
              This is for use on single-core processors.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_ptr_t Old - The old value.
              rme_ptr_t New - The new value.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - If successful, 1; else 0.
******************************************************************************/
rme_ptr_t _RME_Comp_Swap_Single(volatile rme_ptr_t* Ptr,
                                rme_ptr_t Old,
                                rme_ptr_t New)
{
    if(*Ptr==Old)
    {
        *Ptr=New;
        return 1U;
    }

    return 0U;
}
/* End Function:_RME_Comp_Swap_Single ****************************************/

/* Function:_RME_Fetch_Add_Single *********************************************
Description : The fetch-and-add atomic instruction. Increase the value that is
              pointed to by the pointer, and return the value before addition.
              This is for use on single-core processors.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Addend - The number to add.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the addition.
*******************************************************************************/
rme_ptr_t _RME_Fetch_Add_Single(volatile rme_ptr_t* Ptr,
                                rme_cnt_t Addend)
{
    rme_cnt_t Old;

    Old=(rme_cnt_t)(*Ptr);
    *Ptr=(rme_ptr_t)(Old+Addend);

    return (rme_ptr_t)Old;
}
/* End Function:_RME_Fetch_Add_Single ****************************************/

/* Function:_RME_Fetch_And_Single *********************************************
Description : The fetch-and-logic-and atomic instruction. Logic AND the pointer
              value with the operand, and return the value before logic AND.
              This is for use on single-core processors.
Input       : volatile rme_ptr_t* Ptr - The pointer to the data.
              rme_cnt_t Operand - The number to logic AND with the destination.
Output      : volatile rme_ptr_t* Ptr - The pointer to the data.
Return      : rme_ptr_t - The value before the AND operation.
******************************************************************************/
rme_ptr_t _RME_Fetch_And_Single(volatile rme_ptr_t* Ptr,
                                rme_ptr_t Operand)
{
    rme_ptr_t Old;

    Old=*Ptr;
    *Ptr=Old&Operand;

    return Old;
}
/* End Function:_RME_Fetch_And_Single ****************************************/

/* Function:_RME_List_Crt *****************************************************
Description : Create a doubly linked list.
Input       : struct RME_List* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_List_Crt(struct RME_List* Head)
{
    Head->Prev=Head;
    Head->Next=Head;
}
/* End Function:_RME_List_Crt ************************************************/

/* Function:_RME_List_Del *****************************************************
Description : Delete a node from the doubly-linked list.
Input       : struct RME_List* Prev - The previous node.
              struct RME_List* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_List_Del(struct RME_List* Prev,
                   struct RME_List* Next)
{
    Next->Prev=Prev;
    Prev->Next=Next;
}
/* End Function:_RME_List_Del ************************************************/

/* Function:_RME_List_Ins *****************************************************
Description : Insert a node to the doubly-linked list.
Input       : struct RME_List* New - The new node to insert.
              struct RME_List* Prev - The previous node.
              struct RME_List* Next - The next node.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_List_Ins(struct RME_List* New,
                   struct RME_List* Prev,
                   struct RME_List* Next)
{
    Next->Prev=New;
    New->Next=Next;
    New->Prev=Prev;
    Prev->Next=New;
}
/* End Function:_RME_List_Ins ************************************************/

/* Function:_RME_Clear ********************************************************
Description : Memset a memory area to zero. This is not fast due to byte operations;
              this is not meant for large memory. However, it is indeed secure.
Input       : void* Addr - The address to clear.
              rme_ptr_t Size - The size to clear.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_Clear(void* Addr,
                rme_ptr_t Size)
{
    rme_ptr_t Count;

    for(Count=0U;Count<Size;Count++)
        ((rme_u8_t*)Addr)[Count]=0U;
}
/* End Function:_RME_Clear ***************************************************/

/* Function:_RME_Memcmp *******************************************************
Description : Compare two memory segments to see if they are equal. This is not
              fast due to byte operations; this is not meant for large memory.
Input       : const void* Ptr1 - The first memory region.
              const void* Ptr2 - The second memory region.
              rme_ptr_t Num - The number of bytes to compare.
Output      : None.
Return      : rme_ret_t - If Ptr1>Ptr2, then return a positive value; else a negative
                          value. If Ptr1==Ptr2, then return 0;
******************************************************************************/
rme_ret_t _RME_Memcmp(const void* Ptr1,
                      const void* Ptr2,
                      rme_ptr_t Num)
{
    const rme_s8_t* Dst;
    const rme_s8_t* Src;
    rme_ptr_t Count;

    Dst=(const rme_s8_t*)Ptr1;
    Src=(const rme_s8_t*)Ptr2;

    for(Count=0U;Count<Num;Count++)
    {
        if(Dst[Count]!=Src[Count])
        {
            RME_COVERAGE_MARKER();
            
            return Dst[Count]-Src[Count];
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }

    return 0;
}
/* End Function:_RME_Memcmp **************************************************/

/* Function:_RME_Memcpy *******************************************************
Description : Copy one segment of memory to another segment. This is not fast
              due to byte operations; this is not meant for large memory.
Input       : volatile void* Dst - The first memory region.
              volatile void* Src - The second memory region.
              rme_ptr_t Num - The number of bytes to compare.
              rme_ptr_t Size - The size to clear.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_Memcpy(void* Dst,
                 void* Src,
                 rme_ptr_t Num)
{
    rme_ptr_t Count;

    for(Count=0U;Count<Num;Count++)
    {
        ((volatile rme_u8_t*)Dst)[Count]=((volatile rme_u8_t*)Src)[Count];
    }
}
/* End Function:_RME_Memcpy **************************************************/

/* Function:_RME_Distance *****************************************************
Description : Compute the absolute distance between two numbers, when integer
              wraparound is considered.
Input       : rme_ptr_t Num1 - The first number.
              rme_ptr_t Num2 - The second number.
Output      : None.
Return      : rme_ptr_t - The distance.
******************************************************************************/
rme_ptr_t _RME_Distance(rme_ptr_t Num1,
                        rme_ptr_t Num2)
{
    rme_ptr_t Diff1;
    rme_ptr_t Diff2;
    
    Diff1=Num1-Num2;
    Diff2=Num2-Num1;
    
    if(Diff1>Diff2)
    {
        RME_COVERAGE_MARKER();
        return Diff2;
    }
    else
    {
        RME_COVERAGE_MARKER();
        return Diff1;
    }
}
/* End Function:_RME_Distance ************************************************/

/* Function:RME_Int_Print *****************************************************
Description : Print a signed integer on the debugging console. This integer is
              printed as decimal with sign.
Input       : rme_cnt_t Int - The integer to print.
Output      : None.
Return      : rme_cnt_t - The length of the string printed.
******************************************************************************/
#if(RME_DEBUG_PRINT==1U)
rme_cnt_t RME_Int_Print(rme_cnt_t Int)
{
    rme_cnt_t Num;
    rme_cnt_t Abs;
    rme_cnt_t Iter;
    rme_cnt_t Count;
    rme_cnt_t Div;
    
    /* Exit on zero */
    if(Int==0)
    {
        RME_COVERAGE_MARKER();

        __RME_Putchar('0');
        return 1;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }


    /* Correct all negatives into positives */
    if(Int<0)
    {
        RME_COVERAGE_MARKER();

        __RME_Putchar('-');
        Abs=-Int;
        Num=1;
    }
    else
    {
        RME_COVERAGE_MARKER();

        Abs=Int;
        Num=0;
    }

    /* How many digits are there? */
    Count=0;
    Div=1;
    Iter=Abs;
    while(1U)
    {
        Iter/=10;
        Count++;
        if(Iter!=0)
        {
            RME_COVERAGE_MARKER();

            Div*=10;
        }
        else
        {
            RME_COVERAGE_MARKER();

            break;
        }
    }
    Num+=Count;

    /* Print the integer */
    Iter=Abs;
    while(Count>0)
    {
        Count--;
        __RME_Putchar((rme_s8_t)(Iter/Div)+'0');
        Iter=Iter%Div;
        Div/=10;
    }
    
    return Num;
}
#endif
/* End Function:RME_Int_Print ************************************************/

/* Function:RME_Hex_Print *****************************************************
Description : Print a unsigned integer on the debugging console. This integer is
              printed as hexadecimal.
Input       : rme_ptr_t Uint - The unsigned integer to print.
Output      : None.
Return      : rme_cnt_t - The length of the string printed.
******************************************************************************/
#if(RME_DEBUG_PRINT==1U)
rme_cnt_t RME_Hex_Print(rme_ptr_t Uint)
{
    rme_ptr_t Iter;
    rme_ptr_t Count;
    rme_ptr_t Num;
    
    /* Exit on zero */
    if(Uint==0U)
    {
        RME_COVERAGE_MARKER();
        
        __RME_Putchar('0');
        return 1;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Filter out all the zeroes */
    Count=0U;
    Iter=Uint;
    while((Iter>>((sizeof(rme_ptr_t)*8U)-4U))==0U)
    {
        Iter<<=4;
        Count++;
    }
    
    /* Count is the number of pts to print */
    Count=-Count+sizeof(rme_ptr_t)*2U;
    Num=Count;
    while(Count>0U)
    {
        Count--;
        Iter=(Uint>>(Count*4U))&0x0FU;
        if(Iter<10U)
        {
            RME_COVERAGE_MARKER();

            __RME_Putchar(((rme_s8_t)Iter)+'0');
        }
        else
        {
            RME_COVERAGE_MARKER();

            __RME_Putchar(((rme_s8_t)Iter)+'A'-10);
        }
    }
    
    return (rme_cnt_t)Num;
}
#endif
/* End Function:RME_Hex_Print ************************************************/

/* Function:RME_Str_Print *****************************************************
Description : Print a string the kernel console.
              This is only used for kernel-level debugging.
Input       : rme_s8_t* String - The string to print
Output      : None.
Return      : rme_cnt_t - The length of the string printed, the '\0' is not included.
******************************************************************************/
#if(RME_DEBUG_PRINT==1U)
rme_cnt_t RME_Str_Print(rme_s8_t* String)
{
    rme_ptr_t Count;
    
    Count=0;
    while(Count<RME_DEBUG_PRINT_MAX)
    {
        if(String[Count]=='\0')
        {
            RME_COVERAGE_MARKER();
            
            break;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        __RME_Putchar(String[Count++]);
    }
    
    return (rme_cnt_t)Count;
}
#endif
/* End Function:RME_Str_Print ************************************************/

/* Function:RME_Kmain *********************************************************
Description : The entry of the operating system.
Input       : None.
Output      : None.
Return      : rme_ret_t - This function never returns.
******************************************************************************/
rme_ret_t RME_Kmain(void)
{
    /* Disable all interrupts first */
    __RME_Int_Disable();
    /* Some low-level kernel assertions */
    _RME_Lowlvl_Check();
    /* Hardware low-level init */
    __RME_Lowlvl_Init();
    /* Initialize the kernel page tables or memory mappings */
    __RME_Pgt_Kom_Init();
    
    /* Initialize the kernel object allocation table - default init */
    _RME_Kot_Init(RME_KOT_WORD_NUM);
    
    /* Boot into the first process */
    __RME_Boot();
    
    /* Should never reach here */
    return 0;
}
/* End Function:RME_Kmain ****************************************************/

/* Function:_RME_Lowlvl_Check *************************************************
Description : Do some low-level checking for the operating system.
Input       : None.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Lowlvl_Check(void)
{
    /* Make sure the machine is at least 32-bit */
    RME_ASSERT(RME_WORD_ORDER>=5U);
    /* Check if the word order setting is correct */
    RME_ASSERT(RME_WORD_BITS==RME_POW2(RME_WORD_ORDER));
    /* Check if the struct sizes are correct */
    RME_ASSERT(sizeof(struct RME_Cap_Struct)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Cpt)==RME_CAP_SIZE);
#if(RME_PGT_RAW_USER==0U)
    RME_ASSERT(sizeof(struct RME_Cap_Pgt)==RME_CAP_SIZE);
#endif
    RME_ASSERT(sizeof(struct RME_Cap_Prc)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Thd)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Sig)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Inv)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Kfn)==RME_CAP_SIZE);
    RME_ASSERT(sizeof(struct RME_Cap_Kom)==RME_CAP_SIZE);
    /* Check if the other configurations are correct */
    /* Kernel memory allocation minimal size aligned to word boundary */
    RME_ASSERT(RME_KOM_SLOT_ORDER>=RME_WORD_ORDER-3U);
    /* Make sure the number of priorities does not exceed half-word boundary */
    RME_ASSERT(RME_PREEMPT_PRIO_NUM<=RME_POW2(RME_WORD_BITS>>1));
    return 0;
}
/* End Function:_RME_Lowlvl_Check ********************************************/

/* Function:_RME_Svc_Handler **************************************************
Description : The system call handler of the operating system. The register set 
              of the current thread shall be passed in as a parameter.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void _RME_Svc_Handler(struct RME_Reg_Struct* Reg)
{
    /* What's the system call number and major capability ID? */
    rme_ptr_t Svc;
    rme_ptr_t Cid;
    rme_ptr_t Param[3];
    rme_ret_t Retval;
    rme_ptr_t Svc_Num;
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Inv_Struct* Inv_Top;
    struct RME_Cap_Cpt* Cpt;

    /* Get the system call parameters from the system call */
    __RME_Svc_Param_Get(Reg, &Svc, &Cid, Param);
    Svc_Num=Svc&0x3FU;
    
    /* Ultra-fast path - synchronous invocation returning */
    if(Svc_Num==RME_SVC_INV_RET)
    {
        RME_COVERAGE_MARKER();
        
        Retval=_RME_Inv_Ret(Reg,                                            /* volatile struct RME_Reg_Struct* Reg */
                            Param[0],                                       /* rme_ptr_t Retval */
                            0U);                                            /* rme_ptr_t Is_Exc */
        RME_SWITCH_RETURN(Reg, Retval);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Get our current capability table. No need to check whether it is frozen
     * because it can't be deleted anyway */
    Thd_Cur=RME_CPU_LOCAL()->Thd_Cur;
    Inv_Top=RME_INVSTK_TOP(Thd_Cur);
    if(Inv_Top==RME_NULL)
    {
        RME_COVERAGE_MARKER();
        
        Cpt=Thd_Cur->Sched.Prc->Cpt;
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        Cpt=Inv_Top->Prc->Cpt;
    }

    /* Fast path - synchronous invocation activation */
    if(Svc_Num==RME_SVC_INV_ACT)
    {
        RME_COVERAGE_MARKER();
        
        Retval=_RME_Inv_Act(Cpt,
                            Reg,                                            /* volatile struct RME_Reg_Struct* Reg */
                            (rme_cid_t)Param[0],                            /* rme_cid_t Cap_Inv */
                            Param[1]);                                      /* rme_ptr_t Param */
        RME_SWITCH_RETURN(Reg,Retval);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if this operation can potentially cause a context switch. All the 
     * functions that may cause a context switch is listed here. The behavior
     * of these functions shall be: If the function is successful, they shall
     * perform the return value saving on proper register stacks by themselves;
     * if the function fails, it should not conduct such return value saving.
     * These paths are less optimized than synchronous invocation, but are still
     * optimized */
    switch(Svc_Num)
    {
        /* Send to a signal endpoint */
        case RME_SVC_SIG_SND:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Sig_Snd(Cpt,
                                Reg,                                        /* volatile struct RME_Reg_Struct* Reg */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Sig */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Receive from a signal endpoint */
        case RME_SVC_SIG_RCV:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Sig_Rcv(Cpt,
                                Reg,                                        /* volatile struct RME_Reg_Struct* Reg */
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Sig */
                                Param[1]);                                  /* rme_ptr_t Option */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Call kernel functions */
        case RME_SVC_KFN:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Kfn_Act(Cpt,
                                Reg,                                        /* volatile struct RME_Reg_Struct* Reg */
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Kfn */
                                RME_PARAM_D0(Param[0]),                     /* rme_ptr_t Func_ID */
                                RME_PARAM_D1(Param[0]),                     /* rme_ptr_t Sub_ID */
                                Param[1],                                   /* rme_ptr_t Param1 */
                                Param[2]);                                  /* rme_ptr_t Param2 */
            RME_SWITCH_RETURN(Reg, Retval);
        }
        /* Changing thread priority (up to three threads at once) */
        case RME_SVC_THD_SCHED_PRIO:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Sched_Prio(Cpt,
                                       Reg,                                 /* volatile struct RME_Reg_Struct* Reg */
                                       Cid,                                 /* rme_ptr_t Number */
                                       (rme_cid_t)RME_PARAM_D0(Param[0]),   /* rme_cid_t Cap_Thd0 */
                                       RME_PARAM_D1(Param[0]),              /* rme_ptr_t Prio0 */
                                       (rme_cid_t)RME_PARAM_D0(Param[1]),   /* rme_cid_t Cap_Thd1 */
                                       RME_PARAM_D1(Param[1]),              /* rme_ptr_t Prio1 */
                                       (rme_cid_t)RME_PARAM_D0(Param[2]),   /* rme_cid_t Cap_Thd2 */
                                       RME_PARAM_D1(Param[2]));             /* rme_ptr_t Prio2 */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Free a thread from some core */
        case RME_SVC_THD_SCHED_FREE:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Sched_Free(Cpt,
                                       Reg,                                 /* volatile struct RME_Reg_Struct* Reg */
                                       (rme_cid_t)Param[0]);                /* rme_cid_t Cap_Thd */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Transfer time to a thread */
        case RME_SVC_THD_TIME_XFER:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Time_Xfer(Cpt,
                                      Reg,                                  /* volatile struct RME_Reg_Struct* Reg */
                                      (rme_cid_t)Param[0],                  /* rme_cid_t Cap_Thd_Dst */
                                      (rme_cid_t)Param[1],                  /* rme_cid_t Cap_Thd_Src */
                                      Param[2]);                            /* rme_ptr_t Time */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        /* Switch to another thread */
        case RME_SVC_THD_SWT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Swt(Cpt,
                                Reg,                                        /* volatile struct RME_Reg_Struct* Reg */
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Thd */
                                Param[1]);                                  /* rme_ptr_t Full_Yield */
            RME_SWITCH_RETURN(Reg,Retval);
        }
        default:
        {
            RME_COVERAGE_MARKER();
            break;
        }
    } 

    /* It is guaranteed that these functions will never cause a context switch */
    switch(Svc_Num)
    {
        /* Capability table */
        case RME_SVC_CPT_CRT:
        {
            RME_COVERAGE_MARKER();
            Retval=_RME_Cpt_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Crt */
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Kom */
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Crt */
                                Param[1],                                   /* rme_ptr_t Raddr */
                                Param[2]);                                  /* rme_ptr_t Entry_Num */
            break;
        }
        case RME_SVC_CPT_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Cpt_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Del */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Cpt */
            break;
        }
        case RME_SVC_CPT_FRZ:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Cpt_Frz(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Frz */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Frz */
            break;
        }
        case RME_SVC_CPT_ADD:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Cpt_Add(Cpt,
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Cpt_Dst */
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Dst */
                                (rme_cid_t)RME_PARAM_D1(Param[1]),          /* rme_cid_t Cap_Cpt_Src */
                                (rme_cid_t)RME_PARAM_D0(Param[1]),          /* rme_cid_t Cap_Src */
                                Param[2],                                   /* rme_ptr_t Flag */
                                RME_PARAM_KM(Svc,Cid));                     /* rme_ptr_t Ext_Flag */
            break;
        }
        case RME_SVC_CPT_REM:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Cpt_Rem(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Rem */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Rem */
            break;
        }
        
        /* Page table */
#if(RME_PGT_RAW_USER==0U)
        case RME_SVC_PGT_CRT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Kom */
                                (rme_cid_t)RME_PARAM_Q1(Param[0]),          /* rme_cid_t Cap_Pgt */
                                Param[1],                                   /* rme_ptr_t Raddr */
                                Param[2]&(RME_ALLBITS<<1),                  /* rme_ptr_t Base */
                                RME_PARAM_PT(Param[2]),                     /* rme_ptr_t Is_Top */
                                RME_PARAM_Q0(Param[0]),                     /* rme_ptr_t Size_Order */
                                RME_PARAM_PC(Svc));                         /* rme_ptr_t Num_Order */
            break;
        }
        case RME_SVC_PGT_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Pgt */
            break;
        }
        case RME_SVC_PGT_ADD:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Add(Cpt,
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Pgt_Dst */
                                RME_PARAM_D0(Param[0]),                     /* rme_ptr_t Pos_Dst */
                                Cid,                                        /* rme_ptr_t Flag_Dst */
                                (rme_cid_t)RME_PARAM_D1(Param[1]),          /* rme_cid_t Cap_Pgt_Src */
                                RME_PARAM_D0(Param[1]),                     /* rme_ptr_t Pos_Src */
                                Param[2]);                                  /* rme_ptr_t Index */
            break;
        }
        case RME_SVC_PGT_REM:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Rem(Cpt,
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Pgt */
                                Param[1]);                                  /* rme_ptr_t Pos */
            break;
        }
        case RME_SVC_PGT_CON:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Con(Cpt,
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Pgt_Parent */
                                Param[1],                                   /* rme_ptr_t Pos */
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Pgt_Child */
                                Param[2]);                                  /* rme_ptr_t Flag_Child */
            break;
        }
        case RME_SVC_PGT_DES:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Pgt_Des(Cpt,
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Pgt_Parent */
                                Param[1],                                   /* rme_ptr_t Pos */
                                (rme_cid_t)Param[2]);                       /* rme_cid_t Cap_Pgt_Child */
            break;
        }
#endif
        /* Process */
        case RME_SVC_PRC_CRT:
        {
#if(RME_PGT_RAW_USER==0U)
            Retval=_RME_Prc_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Crt */
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Prc */
                                (rme_cid_t)Param[1],                        /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[2]);                       /* rme_cid_t Cap_Pgt */
#else
            Retval=_RME_Prc_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt_Crt */
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Prc */
                                (rme_cid_t)Param[1],                        /* rme_cid_t Cap_Cpt */
                                (rme_ptr_t)Param[2]);                       /* rme_ptr_t Raw_Pgt */
#endif
            break;
        }
        case RME_SVC_PRC_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Prc_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Prc */
            break;
        }
        case RME_SVC_PRC_CPT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Prc_Cpt(Cpt,
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Prc */
                                (rme_cid_t)Param[1]);                       /* rme_cid_t Cap_Cpt */
            break;
        }
        case RME_SVC_PRC_PGT:
        {
            RME_COVERAGE_MARKER();
#if(RME_PGT_RAW_USER==0U)
            Retval=_RME_Prc_Pgt(Cpt,
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Prc */
                                (rme_cid_t)Param[1]);                       /* rme_cid_t Cap_Pgt */
#else
            Retval=_RME_Prc_Pgt(Cpt,
                                (rme_cid_t)Param[0],                        /* rme_cid_t Cap_Prc */
                                Param[1]);                                  /* rme_ptr_t Raw_Pgt */
#endif
            break;
        }
        
        /* Thread */
        case RME_SVC_THD_CRT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Kom */
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Thd */
                                (rme_cid_t)RME_PARAM_D1(Param[1]),          /* rme_cid_t Cap_Prc */
                                RME_PARAM_D0(Param[1]),                     /* rme_ptr_t Prio_Max */
                                Param[2],                                   /* rme_ptr_t Raddr */
                                Svc>>7,                                     /* rme_ptr_t Attr */
                                Svc&0x40U);                                 /* rme_ptr_t Is_Hyp */
            break;
        }
        case RME_SVC_THD_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Thd */
            break;
        }
        case RME_SVC_THD_EXEC_SET:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Exec_Set(Cpt,
                                     (rme_cid_t)Cid,                        /* rme_cid_t Cap_Thd */
                                     Param[0],                              /* rme_ptr_t Entry */
                                     Param[1],                              /* rme_ptr_t Stack */
                                     Param[2]);                             /* rme_ptr_t Param */
            break;
        }
        case RME_SVC_THD_SCHED_BIND:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Sched_Bind(Cpt,
                                       (rme_cid_t)Cid,                      /* rme_cid_t Cap_Thd */
                                       (rme_cid_t)RME_PARAM_D1(Param[0]),   /* rme_cid_t Cap_Thd_Sched */
                                       (rme_cid_t)RME_PARAM_D0(Param[0]),   /* rme_cid_t Cap_Sig */
                                       (rme_tid_t)RME_PARAM_D1(Param[1]),   /* rme_tid_t TID */
                                       RME_PARAM_D0(Param[1]),              /* rme_ptr_t Prio */
                                       Param[2]);                           /* rme_ptr_t Haddr */
            break;
        }
        case RME_SVC_THD_SCHED_RCV:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Thd_Sched_Rcv(Cpt,
                                      (rme_cid_t)Param[0]);                 /* rme_cid_t Cap_Thd */
            break;
        }
        
        /* Signal */
        case RME_SVC_SIG_CRT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Sig_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Sig */
            break;
        }
        case RME_SVC_SIG_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Sig_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Sig */
            break;
        }
        
        /* Invocation */
        case RME_SVC_INV_CRT:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Inv_Crt(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)RME_PARAM_D1(Param[0]),          /* rme_cid_t Cap_Kom */
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Inv */
                                (rme_cid_t)Param[1],                        /* rme_cid_t Cap_Prc */
                                Param[2]);                                  /* rme_ptr_t Raddr */
            break;
        }
        case RME_SVC_INV_DEL:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Inv_Del(Cpt,
                                (rme_cid_t)Cid,                             /* rme_cid_t Cap_Cpt */
                                (rme_cid_t)Param[0]);                       /* rme_cid_t Cap_Inv */
            break;
        }
        case RME_SVC_INV_SET:
        {
            RME_COVERAGE_MARKER();
            
            Retval=_RME_Inv_Set(Cpt,
                                (rme_cid_t)RME_PARAM_D0(Param[0]),          /* rme_cid_t Cap_Inv */
                                Param[1],                                   /* rme_ptr_t Entry */
                                Param[2],                                   /* rme_ptr_t Stack */
                                RME_PARAM_D1(Param[0]));                    /* rme_ptr_t Is_Exc_Ret */
            break;
        }
        /* This is an error */
        default: 
        {
            RME_COVERAGE_MARKER();
            
            Retval=RME_ERR_CPT_NULL;
            break;
        }
    }
    
    /* We set the registers and return */
    __RME_Svc_Retval_Set(Reg,Retval);
}
/* End Function:_RME_Svc_Handler *********************************************/

/* Function:_RME_Tim_Handler **************************************************
Description : The system tick timer handler of RME.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Slice - Number of slices passed since last call of
                                _RME_Tim_Elapse or _RME_Tim_Handler.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void _RME_Tim_Handler(struct RME_Reg_Struct* Reg,
                      rme_ptr_t Slice)
{
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    
    Local=RME_CPU_LOCAL();
    Thd_Cur=Local->Thd_Cur;
    if(Thd_Cur->Sched.Slice<RME_THD_INF_TIME)
    {
        RME_COVERAGE_MARKER();

        /* Decrease timeslice count, and see if the timeslice is used up */
        if(Slice<Thd_Cur->Sched.Slice)
        {
            RME_COVERAGE_MARKER();
            
            Thd_Cur->Sched.Slice-=Slice;
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* Running out of time. Kick this guy out and pick someone else */
            Thd_Cur->Sched.Slice=0U;
            Thd_Cur->Sched.State=RME_THD_TIMEOUT;
            /* Delete it from runqueue */
            _RME_Run_Del(Thd_Cur);
            /* Send a scheduler notification to its parent */
            _RME_Run_Notif(Thd_Cur);
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Send to the system tick timer endpoint. This endpoint is per-core */
    _RME_Kern_Snd(Local->Sig_Tim);

    /* All kernel send complete, now pick the highest priority thread to run */
    _RME_Kern_High(Reg,Local);
}
/* End Function:_RME_Tim_Handler *********************************************/

/* Function:_RME_Tim_Elapse ***************************************************
Description : Honor the elapse of time from the last timer firing.
Input       : rme_ptr_t Slice - Number of slices passed since last call of
                                _RME_Tim_Elapse or _RME_Tim_Handler.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_Tim_Elapse(rme_ptr_t Slice)
{
    struct RME_Thd_Struct* Thd_Cur;
    
    Thd_Cur=RME_CPU_LOCAL()->Thd_Cur;
    
    /* We don't want the slices less than 1 because we want to keep the kernel
     * SVC invariants - the current thread must still be running after this */
    if(Thd_Cur->Sched.Slice<RME_THD_INF_TIME)
    {
        RME_COVERAGE_MARKER();

        /* Decrease timeslice count, but no less than 1, so the thread is
         * always running, which keeps the invariant of the kernel */
        if(Slice<Thd_Cur->Sched.Slice)
            Thd_Cur->Sched.Slice-=Slice;
        else
            Thd_Cur->Sched.Slice=1U;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
}
/* End Function:_RME_Tim_Elapse **********************************************/

/* Function:_RME_Tim_Future ***************************************************
Description : Honor the elapse of time from the last timer firing.
Input       : None.
Output      : None.
Return      : rme_ptr_t - How many slices to program until the next timeout.
******************************************************************************/
rme_ptr_t _RME_Tim_Future(void)
{
    /* If we're running an infinite thread, just program infinite time,
     * these values are very large so that's fine */
    return RME_CPU_LOCAL()->Thd_Cur->Sched.Slice;
}
/* End Function:_RME_Tim_Future **********************************************/

/* Function:_RME_Cpt_Boot_Init ************************************************
Description : Create the first capability table in the system, at boot-time. 
              This function must be called at system startup before setting up
              any other kernel objects.
              This function does not require a kernel memory capability.
Input       : rme_cid_t Cap_Cpt - The capability slot that you want this newly
                                  created capability table capability to be in.
                                  1-Level.
              rme_ptr_t Vaddr - The kernel virtual address to store the
                                capability table.
              rme_ptr_t Entry_Num - The number of capability entries in the
                                    capability table.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Boot_Init(rme_cid_t Cap_Cpt,
                             rme_ptr_t Vaddr,
                             rme_ptr_t Entry_Num)
{
    rme_ptr_t Count;
    struct RME_Cap_Cpt* Cpt;

    /* See if the entry number is too big */
    if((Entry_Num==0U)||(Entry_Num>RME_CID_2L))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_RANGE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr,RME_CPT_SIZE(Entry_Num))!=0)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Object init */
    for(Count=0U;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));

    Cpt=&(((struct RME_Cap_Cpt*)Vaddr)[Cap_Cpt]);
    
    /* Header init */
    Cpt->Head.Root_Ref=1U;
    Cpt->Head.Object=Vaddr;
    Cpt->Head.Flag=RME_CPT_FLAG_ALL;
    
    /* Info init */
    Cpt->Entry_Num=Entry_Num;

    /* At last, write into slot the correct information, and set status to VALID */
    RME_WRITE_RELEASE(&(Cpt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_CPT,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return Cap_Cpt;
}
/* End Function:_RME_Cpt_Boot_Init *******************************************/

/* Function:_RME_Cpt_Boot_Crt *************************************************
Description : Create a boot-time capability table at the designated memory
              address.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Crt - The capability table that contains the 
                                      newly created cap to captbl.
                                      2-Level.
              rme_cid_t Cap_Crt - The capability slot that you want this newly
                                  created capability table capability to be in.
                                  1-Level.
              rme_ptr_t Vaddr - The kernel virtual address to store the 
                                capability table.
              rme_ptr_t Entry_Num - The number of capabilities in the capability table.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt_Crt,
                            rme_cid_t Cap_Crt,
                            rme_ptr_t Vaddr,
                            rme_ptr_t Entry_Num)
{
    rme_ptr_t Count;
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Cpt* Cpt_Crt;
    rme_ptr_t Type_Stat;
    
    /* See if the entry number is too big - this is not restricted by RME_CPT_ENTRY_MAX */
    if((Entry_Num==0U)||(Entry_Num>RME_CID_2L))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_RANGE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,
                   Cap_Cpt_Crt,
                   RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,
                   Cpt_Op,
                   Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);

    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Crt,struct RME_Cap_Cpt*,Cpt_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Cpt_Crt);

    /* Try to mark this area as populated */
    if(_RME_Kot_Mark(Vaddr,RME_CPT_SIZE(Entry_Num))!=0)
    {
        RME_COVERAGE_MARKER();
        
        /* Abort the creation process */
        RME_WRITE_RELEASE(&(Cpt_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Object init */
    for(Count=0U;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));

    /* Header init */
    Cpt_Crt->Head.Root_Ref=0U;
    Cpt_Crt->Head.Object=Vaddr;
    Cpt_Crt->Head.Flag=RME_CPT_FLAG_ALL;
    /* Info init */
    Cpt_Crt->Entry_Num=Entry_Num;

    /* Establish cap */
    RME_WRITE_RELEASE(&(Cpt_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_CPT,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Cpt_Boot_Crt ********************************************/

/* Function:_RME_Cpt_Crt ******************************************************
Description : Create a capability table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Crt - The capability table that contains the 
                                      newly created cap to captbl.
                                      2-Level.
              rme_cid_t Cap_Kom - The kernel memory capability.
                                  2-Level.
              rme_cid_t Cap_Crt - The capability slot that you want this newly
                                  created capability table capability to be in.
                                  1-Level.
              rme_ptr_t Raddr - The relative virtual address to store the 
                                capability table.
              rme_ptr_t Entry_Num - The number of entries in that capability
                                    table.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Crt,
                       rme_cid_t Cap_Kom,
                       rme_cid_t Cap_Crt,
                       rme_ptr_t Raddr,
                       rme_ptr_t Entry_Num)
{
    rme_ptr_t Count;
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Kom* Kom_Op;
    volatile struct RME_Cap_Cpt* Cpt_Crt;
    rme_ptr_t Type_Stat;
    rme_ptr_t Vaddr;

    /* See if the entry number is too big */
    if((Entry_Num==0U)||(Entry_Num>RME_CID_2L))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_RANGE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Are we overrunning the size limit? */
#if(RME_CPT_ENTRY_MAX!=0U)
    if(Entry_Num>RME_CPT_ENTRY_MAX)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_RANGE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
#endif

    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Crt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Kom,RME_CAP_TYPE_KOM,
                   struct RME_Cap_Kom*,Kom_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    /* See if the creation is valid for this kmem range */
    RME_KOM_CHECK(Kom_Op,RME_KOM_FLAG_CPT,Raddr,Vaddr,RME_CPT_SIZE(Entry_Num));

    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Crt,struct RME_Cap_Cpt*,Cpt_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Cpt_Crt);

    /* Try to mark this area as populated */
    if(_RME_Kot_Mark(Vaddr,RME_CPT_SIZE(Entry_Num))<0)
    {
        RME_COVERAGE_MARKER();
        
        /* Failure. Set the Type_Stat back to 0 and abort the creation process */
        RME_WRITE_RELEASE(&(Cpt_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Object init */
    for(Count=0U;Count<Entry_Num;Count++)
        RME_CAP_CLEAR(&(((struct RME_Cap_Struct*)Vaddr)[Count]));

    /* Header init */
    Cpt_Crt->Head.Root_Ref=0U;
    Cpt_Crt->Head.Object=Vaddr;
    Cpt_Crt->Head.Flag=RME_CPT_FLAG_ALL;
    
    /* Info init */
    Cpt_Crt->Entry_Num=Entry_Num;

    /* Establish cap */
    RME_WRITE_RELEASE(&(Cpt_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_CPT,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Cpt_Crt *************************************************/

/* Function:_RME_Cpt_Del ******************************************************
Description : Delete a layer of capability table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Del - The capability table that contains the
                                      cap to captbl.
                                      2-Level.
              rme_cid_t Cap_Del - The capability to the capability table to be
                                  deleted.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Del,
                       rme_cid_t Cap_Del)
{
    rme_ptr_t Count;
    rme_ptr_t Entry_Num;
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Cpt* Cpt_Del;
    struct RME_Cap_Struct* Table;
    rme_ptr_t Type_Stat;
    /* These are used for deletion */
    rme_ptr_t Object;
    rme_ptr_t Size;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Del,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Del,struct RME_Cap_Cpt*,Cpt_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Cpt_Del,Type_Stat,RME_CAP_TYPE_CPT);
    
    /* Is there any capability in this capability table? If yes, we cannot destroy it.
     * We will check every slot to make sure nothing is there. This is surely,
     * predictable but not so perfect. So, if the time of such operations is to be 
     * bounded, the user must control the maximum number of entries in the table
     * by configuring RME_CPT_ENTRY_MAX to a non-zero value. */
    Table=RME_CAP_GETOBJ(Cpt_Del,struct RME_Cap_Struct*);
    Entry_Num=Cpt_Del->Entry_Num;
    for(Count=0U;Count<Entry_Num;Count++)
    {
        if(Table[Count].Head.Type_Stat!=0U)
        {
            RME_COVERAGE_MARKER();
            
            RME_CAP_DEFROST(Cpt_Del,Type_Stat);
            return RME_ERR_CPT_EXIST;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    
    /* Remember these two variables for deletion */
    Object=RME_CAP_GETOBJ(Cpt_Del,rme_ptr_t);
    Size=RME_CPT_SIZE(Cpt_Del->Entry_Num);

    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Cpt_Del,Type_Stat);

    /* Try to depopulate the area - this must be successful */
    RME_ASSERT(_RME_Kot_Erase(Object,Size)==0);
    
    return 0;
}
/* End Function:_RME_Cpt_Del *************************************************/

/* Function:_RME_Cpt_Frz ******************************************************
Description : Freeze a capability in the capability table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Frz  - The capability table containing the cap
                                       to captbl for this operation.
                                       2-Level.
              rme_cid_t Cap_Frz - The cap to the kernel object being freezed.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Frz(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Frz,
                       rme_cid_t Cap_Frz)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Struct* Capobj_Frz;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Frz,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_FRZ);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Frz,struct RME_Cap_Struct*,Capobj_Frz);
    
    /* Check if anything is there. If nothing there, the Type_Stat must be 0. 
     * Need a read acquire barrier here to avoid stale reads below. */
    Type_Stat=RME_READ_ACQUIRE(&(Capobj_Frz->Head.Type_Stat));
    /* See if there is a cap */
    if(RME_CAP_TYPE(Type_Stat)==RME_CAP_TYPE_NOP)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_NULL;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* If this is a root capability, check if the reference count allows freezing */
    if(RME_CAP_ATTR(Type_Stat)==RME_CAP_ATTR_ROOT)
    {
        if(Capobj_Frz->Head.Root_Ref!=0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_REFCNT;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* The capability is already frozen - why do it again? */
    if(RME_CAP_STAT(Type_Stat)==RME_CAP_STAT_FROZEN)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_FROZEN;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the slot is quiescent */
    if(RME_UNLIKELY(RME_CAP_QUIE(Capobj_Frz->Head.Timestamp)==0U))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_QUIE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Update the timestamp */
    Capobj_Frz->Head.Timestamp=RME_TIMESTAMP;
    
    /* Finally, freeze it. We do not report error here because if we CASFAIL someone must have helped us */
    RME_COMP_SWAP(&(Capobj_Frz->Head.Type_Stat),Type_Stat,
                  RME_CAP_TYPE_STAT(RME_CAP_TYPE(Type_Stat),
                                    RME_CAP_STAT_FROZEN,
                                    RME_CAP_ATTR(Type_Stat)));

    return 0;
}
/* End Function:_RME_Cpt_Frz *************************************************/

/* Function:_RME_Cpt_Add ******************************************************
Description : Delegate capability from one capability table to another.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Dst - The capability to the destination 
                                      capability table.
                                      2-Level.
              rme_cid_t Cap_Dst - The capability slot you want to add to.
                                  1-Level.
              rme_cid_t Cap_Cpt_Src - The capability to the source capability
                                      table.
                                      2-Level.
              rme_cid_t Cap_Src - The capability in the source capability table
                                  to delegate.
                                  1-Level.
              rme_ptr_t Flag - The flags for the capability.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Add(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Dst,
                       rme_cid_t Cap_Dst, 
                       rme_cid_t Cap_Cpt_Src,
                       rme_cid_t Cap_Src,
                       rme_ptr_t Flag,
                       rme_ptr_t Ext_Flag)
{
    struct RME_Cap_Cpt* Cpt_Dst;
    struct RME_Cap_Cpt* Cpt_Src;
    volatile struct RME_Cap_Struct* Capobj_Dst;
    volatile struct RME_Cap_Struct* Capobj_Src;
    rme_ptr_t Type_Stat;
    rme_ptr_t Src_Type;
    
    /* These variables are only used for kernel memory checks */
    rme_ptr_t Kom_Begin;
    rme_ptr_t Kom_End;
    rme_ptr_t Kom_Flag;

    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Dst,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Dst,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Src,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Src,Type_Stat);
    /* Check if both captbls are not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Dst,RME_CPT_FLAG_ADD_DST);
    RME_CAP_CHECK(Cpt_Src,RME_CPT_FLAG_ADD_SRC);
    
    /* Get the cap slots */
    RME_CPT_GETSLOT(Cpt_Dst,Cap_Dst,struct RME_Cap_Struct*,Capobj_Dst);
    RME_CPT_GETSLOT(Cpt_Src,Cap_Src,struct RME_Cap_Struct*,Capobj_Src);
    
    /* Atomic read - Read barrier to avoid premature checking of the rest */
    Type_Stat=RME_READ_ACQUIRE(&(Capobj_Src->Head.Type_Stat));
    /* Is the source cap frozen? */
    if(RME_CAP_STAT(Type_Stat)==RME_CAP_STAT_FROZEN)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_FROZEN;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    /* Does the source cap exist at all? */
    if(Type_Stat==0U)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_CPT_NULL;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Dewarn some compilers that complain about uninitialized variables */
    Kom_Begin=0U;
    Kom_End=0U;
    Kom_Flag=0U;
    
    /* Is there a flag conflict? - For page tables, we have different checking mechanisms */
    Src_Type=RME_CAP_TYPE(Type_Stat);
    if(Src_Type==RME_CAP_TYPE_PGT)
    {
        RME_COVERAGE_MARKER();
        
        /* Check the delegation range */
        if(RME_PGT_FLAG_HIGH(Flag)>RME_PGT_FLAG_HIGH(Capobj_Src->Head.Flag))
        {
            RME_COVERAGE_MARKER();
        
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if(RME_PGT_FLAG_LOW(Flag)<RME_PGT_FLAG_LOW(Capobj_Src->Head.Flag))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if(RME_PGT_FLAG_HIGH(Flag)<RME_PGT_FLAG_LOW(Flag))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        /* Check the flags - if there are extra ones, or all zero */
        if(RME_PGT_FLAG_FLAG(Flag)==0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if((RME_PGT_FLAG_FLAG(Flag)&(~RME_PGT_FLAG_FLAG(Capobj_Src->Head.Flag)))!=0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else if(Src_Type==RME_CAP_TYPE_KFN)
    {
        RME_COVERAGE_MARKER();
        
        /* Kernel capabilities only have ranges, no flags - check the delegation range */
        /* Check the delegation range */
        if(RME_KFN_FLAG_HIGH(Flag)>RME_KFN_FLAG_HIGH(Capobj_Src->Head.Flag))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if(RME_KFN_FLAG_LOW(Flag)<RME_KFN_FLAG_LOW(Capobj_Src->Head.Flag))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if(RME_KFN_FLAG_HIGH(Flag)<RME_KFN_FLAG_LOW(Flag))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else if(Src_Type==RME_CAP_TYPE_KOM)
    {
        RME_COVERAGE_MARKER();
        
        /* The Kom_End here is exclusive */
        Kom_Begin=RME_KOM_FLAG_LOW(Flag,Ext_Flag);
        Kom_End=RME_KOM_FLAG_HIGH(Flag,Ext_Flag);
        Kom_Flag=RME_KOM_FLAG_KOM(Ext_Flag);
        
        /* Round start and end to the slot boundary, if we are using slots bigger than 64 bytes */
#if(RME_KOM_SLOT_ORDER>6U)
        Kom_End=RME_ROUND_DOWN(Kom_End,RME_KOM_SLOT_ORDER);
        Kom_Begin=RME_ROUND_UP(Kom_Begin,RME_KOM_SLOT_ORDER);
#endif
        if(Kom_End<=Kom_Begin)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }

        /* Convert relative addresses to absolute addresses and check for overflow */
        Kom_Begin+=((volatile struct RME_Cap_Kom*)Capobj_Src)->Begin;
        if(Kom_Begin<((volatile struct RME_Cap_Kom*)Capobj_Src)->Begin)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        Kom_End+=((volatile struct RME_Cap_Kom*)Capobj_Src)->Begin;
        if(Kom_End<((volatile struct RME_Cap_Kom*)Capobj_Src)->Begin)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }

        /* Check the ranges of kernel memory */
        if(((volatile struct RME_Cap_Kom*)Capobj_Src)->Begin>Kom_Begin)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        /* Internal encoding of 'end' is inclusive */
        if(((volatile struct RME_Cap_Kom*)Capobj_Src)->End<(Kom_End-1U))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Check the flags - if there are extra ones, or all zero */
        if(Kom_Flag==0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if((Kom_Flag&(~(Capobj_Src->Head.Flag)))!=0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    /* All other caps */
    else
    {
        RME_COVERAGE_MARKER();
        
        /* Check the flags - if there are extra ones, or all zero */
        if(Flag==0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        if((Flag&(~(Capobj_Src->Head.Flag)))!=0U)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_CPT_FLAG;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    
    /* Is the destination slot unoccupied? */
    if(Capobj_Dst->Head.Type_Stat!=0U)
    {
        RME_COVERAGE_MARKER();
            
        return RME_ERR_CPT_EXIST;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Try to take the empty slot */
    RME_CPT_OCCUPY(Capobj_Dst);
    
    /* All done, we replicate the cap with flags */
    if(Src_Type==RME_CAP_TYPE_KOM)
    {
        RME_COVERAGE_MARKER();
            
        RME_CAP_COPY(Capobj_Dst,Capobj_Src,Kom_Flag);
        /* Write absolute range information for kernel memory caps */
        ((volatile struct RME_Cap_Kom*)Capobj_Dst)->Begin=Kom_Begin;
        /* The Kom_End encoded inclusively to avoid overflow at max address */
        ((volatile struct RME_Cap_Kom*)Capobj_Dst)->End=Kom_End-1U;
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        RME_CAP_COPY(Capobj_Dst,Capobj_Src,Flag);
    }
    
    /* Set the parent and increase reference count - if this is actually needed.
     * The only two cases where this is not needed are KFN and KOM. These two
     * capability types are standalone on their own and do not need to reference
     * their parent, nor will they update the parent's reference count. This
     * design decision comes from the fact that these two capability types are
     * always created on boot and delegated everywhere, and they don't actually
     * have an object. If we use refcnt on these, we may cause scalability 
     * issues. The parent cap can't be deleted anyway, so this is fine. */
    if((Src_Type!=RME_CAP_TYPE_KOM)&&(Src_Type!=RME_CAP_TYPE_KFN))
    {
        RME_COVERAGE_MARKER();
        
        /* Register root */
        Capobj_Dst->Head.Root_Ref=RME_CAP_CONV_ROOT(Capobj_Src,rme_ptr_t);
    
        /* Increase the parent's refcnt - never overflows, guaranteed by field size */
        RME_FETCH_ADD(&(((volatile struct RME_Cap_Struct*)
                        (Capobj_Dst->Head.Root_Ref))->Head.Root_Ref),1U);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Establish cap */
    RME_WRITE_RELEASE(&(Capobj_Dst->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(Src_Type,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_LEAF));

    return 0;
}
/* End Function:_RME_Cpt_Add *************************************************/

/* Function:_RME_Cpt_Rem ******************************************************
Description : Remove one capability from the capability table. This function
              reverts the delegation.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Rem - The capability to the capability table to
                                      remove from.
                                      2-Level.
              rme_cid_t Cap_Rem - The capability slot you want to remove.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Cpt_Rem(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Rem,
                       rme_cid_t Cap_Rem)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Struct* Capobj_Rem;
    rme_ptr_t Type_Stat;
    rme_ptr_t Rem_Type;
    /* This is used for removal */
    volatile struct RME_Cap_Struct* Capobj_Root;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Rem,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_REM);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Rem,struct RME_Cap_Struct*,Capobj_Rem);
    /* Removal check */
    RME_CAP_REM_CHECK(Capobj_Rem,Type_Stat);
    
    /* If we are KFN or KOM, we don't care about parent or refcnt */
    Rem_Type=RME_CAP_TYPE(Type_Stat);
    if((Rem_Type!=RME_CAP_TYPE_KOM)&&(Rem_Type!=RME_CAP_TYPE_KFN))
    {
        RME_COVERAGE_MARKER();
        
        /* Remember this for refcnt operations */
        Capobj_Root=(struct RME_Cap_Struct*)(Capobj_Rem->Head.Root_Ref);
        
        RME_CAP_DELETE(Capobj_Rem,Type_Stat);

        /* Check done, decrease its parent's refcnt. This must be done at last */
        RME_FETCH_ADD(&(Capobj_Root->Head.Root_Ref),-1);
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* Helping also applies here */
        RME_CAP_DELETE(Capobj_Rem,Type_Stat);
    }
    
    return 0;
}
/* End Function:_RME_Cpt_Rem *************************************************/

/* Function:_RME_Pgt_Boot_Crt *************************************************
Description : Create a boot-time page table.
              This function does not require a kernel memory capability.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability table that contains the newly
                                  created cap to pgtbl.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability slot that you want this newly
                                  created page table capability to be in.
                                  1-Level.
              rme_ptr_t Vaddr - The virtual address to store the page table.
              rme_ptr_t Base - The virtual address to start mapping for this
                               page table. This address must be aligned to the
                               total size of the table.
              rme_ptr_t Is_Top - Whether this page table is the top-level. If
                                 it is, we will map all the kernel page
                                 directories into it.
              rme_ptr_t Size_Order - The size order of the page table. The size
                                     refers to the size of each page in the
                                     page directory.
              rme_ptr_t Num_Order - The number order of entries in the page
                                    table.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Pgt,
                            rme_ptr_t Vaddr,
                            rme_ptr_t Base,
                            rme_ptr_t Is_Top,
                            rme_ptr_t Size_Order,
                            rme_ptr_t Num_Order)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Pgt* Pgt_Crt;
    rme_ptr_t Type_Stat;
    rme_ptr_t Table_Size;
    
    /* Check if the total representable memory exceeds our maximum possible
     * addressible memory under the machine word length */
    if((Size_Order+Num_Order)>RME_POW2(RME_WORD_ORDER))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    
    /* Check if these parameters are feasible */
    if(__RME_Pgt_Check(Base,Is_Top,Size_Order,Num_Order,Vaddr)!=0)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Check if the base address is properly aligned to the total order of the page table */
    if((Base&RME_MASK_END(Size_Order+Num_Order-1U))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Pgt,struct RME_Cap_Pgt*,Pgt_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Pgt_Crt);

    /* Are we creating the top level? */
    if(Is_Top!=0U)
    {
        RME_COVERAGE_MARKER();
        
        Table_Size=RME_PGT_SIZE_TOP(Num_Order);
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        Table_Size=RME_PGT_SIZE_NOM(Num_Order);
    }
    
    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr, Table_Size)!=0)
    {
        RME_COVERAGE_MARKER();
    
        RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat), 0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Header init */
    Pgt_Crt->Head.Root_Ref=0U;
    Pgt_Crt->Head.Object=Vaddr;
    /* Set the property of the page table to only act as source and creating process */
    Pgt_Crt->Head.Flag=RME_PGT_FLAG_FULL_RANGE|RME_PGT_FLAG_ADD_SRC|
                       RME_PGT_FLAG_PRC_CRT|RME_PGT_FLAG_PRC_PGT;
    
    /* Info init */
    Pgt_Crt->Base=Base|Is_Top;
    Pgt_Crt->Order=RME_PGT_ORDER(Size_Order,Num_Order);
    Pgt_Crt->ASID=0U;

    /* Object init - need to add all kernel pages if they are top-level */
    if(__RME_Pgt_Init(Pgt_Crt)<0)
    {
        RME_COVERAGE_MARKER();
        
        /* This must be successful */
        RME_ASSERT(_RME_Kot_Erase(Vaddr, Table_Size)==0);

        /* Unsuccessful. Revert operations */
        RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat), 0U);
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Establish cap */
    RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_PGT,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
#endif
/* End Function:_RME_Pgt_Boot_Crt ********************************************/

/* Function:_RME_Pgt_Boot_Add *************************************************
Description : This function is exclusively used to set up the Init process's
              memory mappings in the booting process. When the system has
              booted, it won't possible to fabricate pages like this.
              Additionally, this function will set the cap to page table's 
              property as unremovable. This means that it is not allowed to
              remove any pages in the directory. It will set the reference
              count of the capability as 1, thus making the capability to the
              initial page table undeletable.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt - The capability to the page table.
                                  2-Level.
              rme_ptr_t Paddr - The physical address to map from.
              rme_ptr_t Pos - The page table position to map to.
              rme_ptr_t Flag - The flags for the user page.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Boot_Add(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Pgt, 
                            rme_ptr_t Paddr,
                            rme_ptr_t Pos,
                            rme_ptr_t Flag)
{
    struct RME_Cap_Pgt* Pgt_Op;
    rme_ptr_t Type_Stat;
    rme_ptr_t Szord;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen, but don't check their properties */
    RME_CAP_CHECK(Pgt_Op,0U);

    Szord=RME_PGT_SZORD(Pgt_Op->Order);
#if(RME_VA_EQU_PA!=0U)
    /* Check if we force identical mapping */
    if(Szord==RME_WORD_BITS)
    {
        RME_COVERAGE_MARKER();
        
        if((Paddr!=0U)||(Pos!=0U))
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else if(Paddr!=(RME_PGT_BASE(Pgt_Op->Base)+(Pos<<Szord)))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
#endif

    /* See if the mapping range and the granularity is allowed */
    if(((Pos>>RME_PGT_NMORD(Pgt_Op->Order))!=0U)||
       ((Paddr&RME_MASK_END(Szord-1U))!=0U))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict */
    if(__RME_Pgt_Page_Map(Pgt_Op,Paddr,Pos,Flag)!=0)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
#endif
/* End Function:_RME_Pgt_Boot_Add ********************************************/

/* Function:_RME_Pgt_Boot_Con *************************************************
Description : Map a child page table from the parent page table at boot-time.
              This does not check flags.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt_Parent - The capability to the parent page 
                                         table.
                                         2-Level.
              rme_ptr_t Pos - The parent page table position to map the child
                              page table to.
              rme_cid_t Cap_Pgt_Child - The capability to the child page table.
                                        2-Level.
              rme_ptr_t Flag_Child - Mapping flags.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Boot_Con(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Pgt_Parent,
                            rme_ptr_t Pos,
                            rme_cid_t Cap_Pgt_Child,
                            rme_ptr_t Flag_Child)
{
    struct RME_Cap_Pgt* Pgt_Parent;
    struct RME_Cap_Pgt* Pgt_Child;
    volatile struct RME_Cap_Pgt* Pgt_Root;
    rme_ptr_t Type_Stat;
    rme_ptr_t Order_Child;
    rme_ptr_t Szord_Parent;
#if(RME_VA_EQU_PA!=0U)
    rme_ptr_t Begin_Parent;
    rme_ptr_t End_Parent;
#endif
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Parent,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Parent,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Child,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Child,Type_Stat);
    /* Check if both page table caps are not frozen but don't check flags */
    RME_CAP_CHECK(Pgt_Parent,0U);
    RME_CAP_CHECK(Pgt_Child,0U);
    
    /* See if the mapping range is allowed */
    if((Pos>>RME_PGT_NMORD(Pgt_Parent->Order))!=0U)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the child table falls within one slot of the parent table */
    Order_Child=RME_PGT_NMORD(Pgt_Child->Order)+RME_PGT_SZORD(Pgt_Child->Order);
    Szord_Parent=RME_PGT_SZORD(Pgt_Parent->Order);
    if(Szord_Parent<Order_Child)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

#if(RME_VA_EQU_PA!=0U)
    /* Check if the child falls into the correct slot */
    if(Szord_Parent<RME_WORD_BITS)
    {
        /* Check if the virtual address mapping is correct - note that the child
         * could also be a top-level page table. Whether constructing a top into 
         * another top is allowed is HAL-defined. */
        Begin_Parent=RME_PGT_BASE(Pgt_Parent->Base)+(Pos<<Szord_Parent);
        if(RME_PGT_BASE(Pgt_Child->Base)<Begin_Parent)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        End_Parent=Begin_Parent+RME_POW2(Szord_Parent);
        
        /* If this is zero, then the parent table's slot expands to the end of 
         * memory, and we are sure that overflow won't happen because start
         * address is always aligned to the total order of the child table. 
         * No UB for Order_Child here because Szord_Parent>=Order_Child */
        if(End_Parent!=0U)
        {
            if((RME_PGT_BASE(Pgt_Child->Base)+RME_POW2(Order_Child))>End_Parent)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_PGT_ADDR;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    /* Parent is full range, check exempt */
    else
    {
        RME_COVERAGE_MARKER();
    }
#else
    /* Force no path compression when virtual mappings are enabled */
    if(Szord_Parent!=Order_Child)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
#endif

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict */
    if(__RME_Pgt_Pgdir_Map(Pgt_Parent,Pos,Pgt_Child,Flag_Child)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Increase refcnt for both parent/child */
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Parent,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),1U);
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Child,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),1U);

    return 0;
}
#endif
/* End Function:_RME_Pgt_Boot_Con ********************************************/

/* Function:_RME_Pgt_Crt ******************************************************
Description : Create a page table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability table that contains the newly
                                  created cap to pgtbl.
                                  2-Level.
              rme_cid_t Cap_Kom - The kernel memory capability.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability slot that you want this newly
                                  created page table capability to be in.
                                  1-Level.
              rme_ptr_t Raddr - The relative virtual address to store the page
                                table kernel object.
              rme_ptr_t Base - The virtual address to start mapping for this
                               page table. This address must be aligned to the
                               total size of the table.
              rme_ptr_t Is_Top - Whether this page table is the top-level. If
                                 it is, we will map all the kernel page 
                                 directories into this one.
              rme_ptr_t Size_Order - The size order of the page table. The size
                                     refers to the size of each page in the 
                                     page directory.
              rme_ptr_t Num_Order - The number order of entries in the table.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Kom,
                       rme_cid_t Cap_Pgt,
                       rme_ptr_t Raddr,
                       rme_ptr_t Base,
                       rme_ptr_t Is_Top,
                       rme_ptr_t Size_Order,
                       rme_ptr_t Num_Order)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Kom* Kom_Op;
    struct RME_Cap_Pgt* Pgt_Crt;
    rme_ptr_t Type_Stat;
    rme_ptr_t Vaddr;
    rme_ptr_t Table_Size;
    
    /* Check if the total representable memory exceeds our maximum possible
     * addressible memory under the machine word length */
    if((Size_Order+Num_Order)>RME_POW2(RME_WORD_ORDER))
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Kom,RME_CAP_TYPE_KOM,
                   struct RME_Cap_Kom*,Kom_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op, RME_CPT_FLAG_CRT);
    
    /* Are we creating the top-level? */
    if(Is_Top!=0U)
    {
        RME_COVERAGE_MARKER();

        Table_Size=RME_PGT_SIZE_TOP(Num_Order);
    }
    else
    {
        RME_COVERAGE_MARKER();

        Table_Size=RME_PGT_SIZE_NOM(Num_Order);
    }
    
    /* See if the creation is valid for this kmem range */
    RME_KOM_CHECK(Kom_Op,RME_KOM_FLAG_PGT,Raddr,Vaddr,Table_Size);

    /* Check if these parameters are feasible */
    if(__RME_Pgt_Check(Base,Is_Top,Size_Order,Num_Order,Vaddr)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Check if the start address is properly aligned to the total order of the page table */
    if((Base&RME_MASK_END(Size_Order+Num_Order-1U))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Pgt,struct RME_Cap_Pgt*,Pgt_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Pgt_Crt);

    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr,Table_Size)!=0)
    {
        RME_COVERAGE_MARKER();

        RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Header init */
    Pgt_Crt->Head.Root_Ref=0U;
    Pgt_Crt->Head.Object=Vaddr;
    Pgt_Crt->Head.Flag=RME_PGT_FLAG_FULL_RANGE|RME_PGT_FLAG_ALL;
    
    /* Info init */
    Pgt_Crt->Base=Base|Is_Top;
    Pgt_Crt->Order=RME_PGT_ORDER(Size_Order,Num_Order);
    Pgt_Crt->ASID=0U;
    
    /* Object init - need to add all kernel pages if they are top-level */
    if(__RME_Pgt_Init(Pgt_Crt)<0)
    {
        RME_COVERAGE_MARKER();

        /* This must be successful */
        RME_ASSERT(_RME_Kot_Erase(Vaddr,Table_Size)==0);
        
        /* Unsuccessful. Revert operations */
        RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat),0U);
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Creation complete */
    RME_WRITE_RELEASE(&(Pgt_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_PGT,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
#endif
/* End Function:_RME_Pgt_Crt *************************************************/

/* Function:_RME_Pgt_Del ******************************************************
Description : Delete a page table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the captbl that may contain
                                  the cap to new captbl.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability slot that you want this newly
                                  created page table capability to be in.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Pgt)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Pgt* Pgt_Del;
    rme_ptr_t Type_Stat;
    /* These are used for deletion */
    rme_ptr_t Object;
    rme_ptr_t Table_Size;
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Pgt,struct RME_Cap_Pgt*,Pgt_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Pgt_Del,Type_Stat,RME_CAP_TYPE_PGT);
    
    /* Hardware related deletion check passed down to the HAL. The driver should make
     * sure that it does not reference any lower level tables. If the driver layer does
     * not conform to this, the deletion of page table is not guaranteed to main kernel
     * consistency, and such consistency must be maintained by the user-level. It is 
     * recommended that the driver layer enforce such consistency. */
    if(__RME_Pgt_Del_Check(Pgt_Del)<0)
    {
        RME_COVERAGE_MARKER();

        RME_CAP_DEFROST(Pgt_Del,Type_Stat);
        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Remember these two variables for deletion */
    Object=RME_CAP_GETOBJ(Pgt_Del,rme_ptr_t);
    if(((Pgt_Del->Base)&RME_PGT_TOP)!=0U)
    {
        RME_COVERAGE_MARKER();

        Table_Size=RME_PGT_SIZE_TOP(RME_PGT_NMORD(Pgt_Del->Order));
    }
    else
    {
        RME_COVERAGE_MARKER();

        Table_Size=RME_PGT_SIZE_NOM(RME_PGT_NMORD(Pgt_Del->Order));
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Pgt_Del,Type_Stat);

    /* Try to erase the area - This must be successful */
    RME_ASSERT(_RME_Kot_Erase(Object,Table_Size)==0);

    return 0;
}
#endif
/* End Function:_RME_Pgt_Del *************************************************/

/* Function:_RME_Pgt_Add ******************************************************
Description : Delegate a page from one page table to another. This is the only
              way to add pages to new page tables after the system boots.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt_Dst - The capability to the destination page
                                      directory.
                                      2-Level.
              rme_ptr_t Pos_Dst - The position to delegate to in the
                                  destination page directory.
              rme_ptr_t Flag_Dst - The page access permission for the
                                   destination page.
              rme_cid_t Cap_Pgt_Src - The capability to the source page 
                                      directory.
                                      2-Level.
              rme_ptr_t Pos_Dst - The position to delegate from in the source
                                  page directory.
              rme_ptr_t Index - The index of the physical address frame to
                                delegate. For example, if the destination
                                directory's page size is 1/4 of that of the
                                source directory, index=0 will delegate the
                                first 1/4, index=1 will delegate the second
                                1/4, index=2 will delegate the third 1/4, and
                                index=3 will delegate the last 1/4.
                                All other index values are illegal.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Add(struct RME_Cap_Cpt* Cpt, 
                       rme_cid_t Cap_Pgt_Dst,
                       rme_ptr_t Pos_Dst,
                       rme_ptr_t Flag_Dst,
                       rme_cid_t Cap_Pgt_Src,
                       rme_ptr_t Pos_Src,
                       rme_ptr_t Index)
{
    struct RME_Cap_Pgt* Pgt_Src;
    struct RME_Cap_Pgt* Pgt_Dst;
    rme_ptr_t Paddr_Dst;
    rme_ptr_t Paddr_Src;
    rme_ptr_t Flag_Src;
    rme_ptr_t Type_Stat;
    rme_ptr_t Szord_Src;
    rme_ptr_t Szord_Dst;

    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Dst,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Dst,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Src,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Src,Type_Stat);
    /* Check if both page table caps are not frozen and allows such operations */
    RME_CAP_CHECK(Pgt_Dst,RME_PGT_FLAG_ADD_DST);
    RME_CAP_CHECK(Pgt_Src,RME_PGT_FLAG_ADD_SRC);
    /* Check the operation range - This is page table specific */
    if((Pos_Dst>RME_PGT_FLAG_HIGH(Pgt_Dst->Head.Flag))||
       (Pos_Dst<RME_PGT_FLAG_LOW(Pgt_Dst->Head.Flag))||
       (Pos_Src>RME_PGT_FLAG_HIGH(Pgt_Src->Head.Flag))||
       (Pos_Src<RME_PGT_FLAG_LOW(Pgt_Src->Head.Flag)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if the size order relationship is correct */
    Szord_Dst=RME_PGT_SZORD(Pgt_Dst->Order);
    Szord_Src=RME_PGT_SZORD(Pgt_Src->Order);
    if(Szord_Dst>Szord_Src)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if the positions are out of range - NMORD is restricted, no UB */
    if(((Pos_Dst>>RME_PGT_NMORD(Pgt_Dst->Order))!=0U)||
       ((Pos_Src>>RME_PGT_NMORD(Pgt_Src->Order))!=0U))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if the source subposition is out of range - avoid UB */
    if(Szord_Src<RME_WORD_BITS)
    {
        /* No UB because Szord_Dst<=Szord_Src */
        if(RME_POW2(Szord_Src)<=(Index<<Szord_Dst))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the physical address and RME standard flags of that source page */
    if(__RME_Pgt_Lookup(Pgt_Src,Pos_Src,&Paddr_Src,&Flag_Src)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_HW;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Calculate the destination physical address - avoid UB */
    if(Szord_Dst<RME_WORD_BITS)
    {
        RME_COVERAGE_MARKER();
        
        Paddr_Dst=Paddr_Src+(Index<<Szord_Dst);
#if(RME_VA_EQU_PA!=0U)
        /* Check if we force identical mapping */
        if(Paddr_Dst!=(RME_PGT_BASE(Pgt_Dst->Base)+(Pos_Dst<<Szord_Dst)))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
#endif
    }
    /* The destination is also full range */
    else
    {
        RME_COVERAGE_MARKER();
        
        Paddr_Dst=Paddr_Src;
#if(RME_VA_EQU_PA!=0U)
        if(Paddr_Dst!=RME_PGT_BASE(Pgt_Dst->Base))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
#endif
    }
    
    /* Analyze the flags - we do not allow expansion of access permissions */
    if(((Flag_Dst)&(~Flag_Src))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_PERM;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict */
    if(__RME_Pgt_Page_Map(Pgt_Dst,Paddr_Dst,Pos_Dst,Flag_Dst)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
#endif
/* End Function:_RME_Pgt_Add *************************************************/

/* Function:_RME_Pgt_Rem ******************************************************
Description : Remove a page from the page table. We are doing unmapping of a
              page.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt - The capability to the page table.
                                  2-Level.
              rme_ptr_t Pos - The page table position to unmap from.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Rem(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Pgt,
                       rme_ptr_t Pos)
{
    struct RME_Cap_Pgt* Pgt_Rem;
    rme_ptr_t Type_Stat;
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Pgt*,Pgt_Rem,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Pgt_Rem,RME_PGT_FLAG_REM);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGT_FLAG_HIGH(Pgt_Rem->Head.Flag))||
       (Pos<RME_PGT_FLAG_LOW(Pgt_Rem->Head.Flag)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if the unmapping range is allowed */
    if((Pos>>RME_PGT_NMORD(Pgt_Rem->Order))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict */
    if(__RME_Pgt_Page_Unmap(Pgt_Rem,Pos)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
#endif
/* End Function:_RME_Pgt_Rem *************************************************/

/* Function:_RME_Pgt_Con ******************************************************
Description : Map a child page table into the parent page table, to construct
              an address space tree.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt_Parent - The capability to the parent page 
                                         table.
                                         2-Level.
              rme_ptr_t Pos - The parent page table position to map the child
                              page table to.
              rme_cid_t Cap_Pgt_Child - The capability to the child page table.
                                        2-Level.
              rme_ptr_t Flag_Child - Mapping flags.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Con(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Pgt_Parent,
                       rme_ptr_t Pos,
                       rme_cid_t Cap_Pgt_Child,
                       rme_ptr_t Flag_Child)
{
    struct RME_Cap_Pgt* Pgt_Parent;
    struct RME_Cap_Pgt* Pgt_Child;
    volatile struct RME_Cap_Pgt* Pgt_Root;
    rme_ptr_t Order_Child;
    rme_ptr_t Szord_Parent;
#if(RME_VA_EQU_PA!=0U)
    rme_ptr_t Begin_Parent;
    rme_ptr_t End_Parent;
#endif
    rme_ptr_t Type_Stat;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Parent,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Parent,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Child,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Child,Type_Stat);
    /* Check if both page table caps are not frozen and allows such operations */
    RME_CAP_CHECK(Pgt_Parent, RME_PGT_FLAG_CON_PARENT);
    RME_CAP_CHECK(Pgt_Child, RME_PGT_FLAG_CHILD);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGT_FLAG_HIGH(Pgt_Parent->Head.Flag))||
       (Pos<RME_PGT_FLAG_LOW(Pgt_Parent->Head.Flag)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the mapping range is allowed */
    if((Pos>>RME_PGT_NMORD(Pgt_Parent->Order))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the child table falls within one slot of the parent table */
    Order_Child=RME_PGT_NMORD(Pgt_Child->Order)+RME_PGT_SZORD(Pgt_Child->Order);
    Szord_Parent=RME_PGT_SZORD(Pgt_Parent->Order);
    if(Szord_Parent<Order_Child)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

#if(RME_VA_EQU_PA!=0U)
    /* Check if the child falls into the correct slot */
    if(Szord_Parent<RME_WORD_BITS)
    {
        /* Check if the virtual address mapping is correct - note that the child
         * could also be a top-level page table. Whether constructing a top into 
         * another top is allowed is HAL-defined. */
        Begin_Parent=RME_PGT_BASE(Pgt_Parent->Base)+(Pos<<Szord_Parent);
        if(RME_PGT_BASE(Pgt_Child->Base)<Begin_Parent)
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_PGT_ADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        End_Parent=Begin_Parent+RME_POW2(Szord_Parent);
        
        /* If this is zero, then the parent table's slot expands to the end of 
         * memory, and we are sure that overflow won't happen because start
         * address is always aligned to the total order of the child table. 
         * No UB for Order_Child here because Szord_Parent>=Order_Child */
        if(End_Parent!=0U)
        {
            if((RME_PGT_BASE(Pgt_Child->Base)+RME_POW2(Order_Child))>End_Parent)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_PGT_ADDR;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    /* Parent is full range, check exempt */
    else
    {
        RME_COVERAGE_MARKER();
    }
#else
    /* Force no path compression when virtual mappings are enabled */
    if(Szord_Parent!=Order_Child)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
#endif

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict */
    if(__RME_Pgt_Pgdir_Map(Pgt_Parent,Pos,Pgt_Child,Flag_Child)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Increase refcnt for both parent/child */
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Parent,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),1);
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Child,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),1);

    return 0;
}
#endif
/* End Function:_RME_Pgt_Con *************************************************/

/* Function:_RME_Pgt_Des ******************************************************
Description : Unmap a child page table from the parent page table, destructing
              the address space tree.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Pgt_Parent - The capability to the parent page
                                         table.
                                         2-Level.
              rme_ptr_t Pos - The parent page table position to position unmap
                              the child page table from. The child page table
                              must be there for the destruction to succeed.
              rme_cid_t Cap_Pgt_Child - The capability to the child page table.
                                        2-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Pgt_Des(struct RME_Cap_Cpt* Cpt, 
                       rme_cid_t Cap_Pgt_Parent,
                       rme_ptr_t Pos,
                       rme_cid_t Cap_Pgt_Child)
{
    struct RME_Cap_Pgt* Pgt_Parent;
    struct RME_Cap_Pgt* Pgt_Child;
    struct RME_Cap_Pgt* Pgt_Root;
    rme_ptr_t Type_Stat;
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Parent,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Pgt*,Pgt_Parent,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Pgt_Child,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Pgt*,Pgt_Child,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Pgt_Parent, RME_PGT_FLAG_DES_PARENT);
    RME_CAP_CHECK(Pgt_Child, RME_PGT_FLAG_CHILD);
    /* Check the operation range - This is page table specific */
    if((Pos>RME_PGT_FLAG_HIGH(Pgt_Parent->Head.Flag))||
       (Pos<RME_PGT_FLAG_LOW(Pgt_Parent->Head.Flag)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if the unmapping range is allowed */
    if((Pos>>RME_PGT_NMORD(Pgt_Parent->Order))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_ADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Actually do the mapping - This work is passed down to the HAL. 
     * Under multi-core, HAL should use CAS to avoid a conflict. Also,
     * the HAL needs to guarantee that the Child is actually mapped there,
     * and use that as the old value in CAS */
    if(__RME_Pgt_Pgdir_Unmap(Pgt_Parent,Pos,Pgt_Child)<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PGT_MAP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Decrease refcnt for both parent/child */
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Parent,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),-1);
    Pgt_Root=RME_CAP_CONV_ROOT(Pgt_Child,struct RME_Cap_Pgt*);
    RME_FETCH_ADD(&(Pgt_Root->Head.Root_Ref),-1);

    return 0;
}
#endif
/* End Function:_RME_Pgt_Des *************************************************/

/* Function:_RME_Kot_Init *****************************************************
Description : Initialize the kernel object table according to the total kernel
              memory size, which decides the number of words in the table.
Input       : rme_ptr_t Words - the number of words in the table.
Output      : None.
Return      : rme_ret_t - If the number of words are is not sufficient to hold 
                          all kernel memory, -1; else 0.
******************************************************************************/
rme_ret_t _RME_Kot_Init(rme_ptr_t Word)
{
    rme_ptr_t Count;
    
    if(Word<RME_KOT_WORD_NUM)
    {
        RME_COVERAGE_MARKER();

        return -1;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Zero out the whole table */
    for(Count=0U;Count<Word;Count++)
        RME_KOT_VA_BASE[Count]=0U;

    return 0;
}
/* End Function:_RME_Kot_Init ************************************************/

/* Function:_RME_Kot_Mark *****************************************************
Description : Populate the kernel object bitmap contiguously.
Input       : rme_ptr_t Kaddr - The kernel virtual address.
              rme_ptr_t Size - The size of the memory to populate.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Kot_Mark(rme_ptr_t Kaddr,
                        rme_ptr_t Size)
{
    rme_ptr_t Count;
    /* The old value */
    rme_ptr_t Old_Val;
    /* Whether we need to undo our operations because of CAS failure */
    rme_ptr_t Undo;
    /* The actual word to start the marking */
    rme_ptr_t Start;
    /* The actual word to end the marking */
    rme_ptr_t End;
    /* The mask at the begin word */
    rme_ptr_t Mask_Begin;
    /* The mask at the end word */
    rme_ptr_t Mask_End;

    /* Check if the marking is well aligned */
    if((Kaddr&RME_MASK_END(RME_KOM_SLOT_ORDER-1U))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_KOT_BMP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Round the marking to RME_KOM_SLOT_ORDER boundary, and rely on compiler for optimization */
    Start=(Kaddr-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER;
    Mask_Begin=RME_MASK_BEGIN(Start&RME_MASK_END(RME_WORD_ORDER-1U));
    Start=Start>>RME_WORD_ORDER;
    
    End=(Kaddr+Size-1U-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER;
    Mask_End=RME_MASK_END(End&RME_MASK_END(RME_WORD_ORDER-1U));
    End=End>>RME_WORD_ORDER;
    
    /* See if the start and end are in the same word */
    if(Start==End)
    {
        RME_COVERAGE_MARKER();

        /* Someone already populated something here */
        Old_Val=RME_KOT_VA_BASE[Start];
        if((Old_Val&(Mask_Begin&Mask_End))!=0U)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Check done, do the marking with CAS */
        if(RME_COMP_SWAP(&RME_KOT_VA_BASE[Start],
                         Old_Val,
                         Old_Val|(Mask_Begin&Mask_End))==RME_CASFAIL)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        Undo=0U;
        /* Check&Mark the start */
        Old_Val=RME_KOT_VA_BASE[Start];
        if((Old_Val&Mask_Begin)!=0U)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        if(RME_COMP_SWAP(&RME_KOT_VA_BASE[Start],
                         Old_Val,
                         Old_Val|Mask_Begin)==RME_CASFAIL)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Check&Mark the middle */
        for(Count=Start+1U;Count<End;Count++)
        {
            Old_Val=RME_KOT_VA_BASE[Count];
            if(Old_Val!=0U)
            {
                RME_COVERAGE_MARKER();

                Undo=1U;
                break;
            }
            else
            {
                RME_COVERAGE_MARKER();
                
                if(RME_COMP_SWAP(&RME_KOT_VA_BASE[Count],
                                 Old_Val,
                                 RME_ALLBITS)==RME_CASFAIL)
                {
                    RME_COVERAGE_MARKER();
                    
                    Undo=1U;
                    break;
                }
                else
                {
                    RME_COVERAGE_MARKER();
                }
            }
        }
        
        /* See if the middle part failed. If yes, we skip the end marking */
        if(Undo==0U)
        {
            RME_COVERAGE_MARKER();

            /* Check&Mark the end */
            Old_Val=RME_KOT_VA_BASE[End];
            if((Old_Val&Mask_End)!=0U)
            {
                RME_COVERAGE_MARKER();

                Undo=1U;
            }
            else
            {
                RME_COVERAGE_MARKER();

                if(RME_COMP_SWAP(&RME_KOT_VA_BASE[End],
                                 Old_Val,
                                 Old_Val|Mask_End)==RME_CASFAIL)
                {
                    RME_COVERAGE_MARKER();

                    Undo=1U;
                }
                else
                {
                    RME_COVERAGE_MARKER();
                }
            }
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* See if we need to undo. If yes, proceed to unroll and return error */
        if(Undo!=0U)
        {
            RME_COVERAGE_MARKER();

            /* Undo the middle part - no CAS neeeded, write back is always atomic */
            for(Count--;Count>Start;Count--)
            {
                RME_KOT_VA_BASE[Count]=0U;
            }
            /* Undo the first word - need atomic instructions */
            RME_FETCH_AND(&(RME_KOT_VA_BASE[Start]),~Mask_Begin);
            /* Return failure */
            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }

    return 0;
}
/* End Function:_RME_Kot_Mark ************************************************/

/* Function:_RME_Kot_Erase ****************************************************
Description : Depopulate the kernel object bitmap contiguously. We do not need 
              CAS on erasure operations.
Input       : rme_ptr_t Kaddr - The kernel virtual address.
              rme_ptr_t Size - The size of the memory to depopulate.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Kot_Erase(rme_ptr_t Kaddr,
                         rme_ptr_t Size)
{
    /* The actual word to start the marking */
    rme_ptr_t Start;
    /* The actual word to end the marking */
    rme_ptr_t End;
    /* The mask at the begin word */
    rme_ptr_t Mask_Begin;
    /* The mask at the end word */
    rme_ptr_t Mask_End;
    rme_ptr_t Count;

    /* Check if the marking is well aligned */
    if((Kaddr&RME_MASK_END(RME_KOM_SLOT_ORDER-1U))!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_KOT_BMP;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Round the marking to RME_KOM_SLOT_ORDER boundary, and rely on compiler for optimization */
    Start=(Kaddr-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER;
    Mask_Begin=RME_MASK_BEGIN(Start&RME_MASK_END(RME_WORD_ORDER-1U));
    Start=Start>>RME_WORD_ORDER;
    
    End=(Kaddr+Size-1U-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER;
    Mask_End=RME_MASK_END(End&RME_MASK_END(RME_WORD_ORDER-1U));
    End=End>>RME_WORD_ORDER;
    
    /* See if the start and end are in the same word */
    if(Start==End)
    {
        RME_COVERAGE_MARKER();

        /* This address range is not fully populated */
        if((RME_KOT_VA_BASE[Start]&(Mask_Begin&Mask_End))!=(Mask_Begin&Mask_End))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }

        /* Check done, do the unmarking - need atomic operations */
        RME_FETCH_AND(&(RME_KOT_VA_BASE[Start]),~(Mask_Begin&Mask_End));
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* Check the start */
        if((RME_KOT_VA_BASE[Start]&Mask_Begin)!=Mask_Begin)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Check the middle */
        for(Count=Start+1U;Count<End-1U;Count++)
        {
            if(RME_KOT_VA_BASE[Count]!=RME_ALLBITS)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_KOT_BMP;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
        }

        /* Check the end */
        if((RME_KOT_VA_BASE[End]&Mask_End)!=Mask_End)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_KOT_BMP;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Erase the start - make it atomic */
        RME_FETCH_AND(&(RME_KOT_VA_BASE[Start]),~Mask_Begin);
        /* Erase the middle - do not need atomics here */
        for(Count=Start+1U;Count<End-1U;Count++)
        {
            RME_KOT_VA_BASE[Count]=0U;
        }
        /* Erase the end - make it atomic */
        RME_FETCH_AND(&(RME_KOT_VA_BASE[End]),~Mask_End);
    }

    return 0;
}
/* End Function:_RME_Kot_Erase ***********************************************/

/* Function:_RME_Kom_Boot_Crt *************************************************
Description : Create boot-time kernel memory capability. Kernel memory allow
              you to create specific types of kernel objects in a specific 
              kernel memory range. The initial kernel memory capability's
              content is supplied by the kernel according to config.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the captbl that may contain
                                  the cap to kernel function.
                                  2-Level.
              rme_cid_t Cap_Kom - The capability to the kernel memory.
                                  1-Level.
              rme_ptr_t Begin - The begin address of the kernel memory, aligned
                                to kotbl granularity.
              rme_ptr_t End - The end address of the kernel memory, aligned to
                              kotbl granularity, then -1.
              rme_ptr_t Flag - The operation flags for this kernel memory. Set
                               acoording to your needs.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Kom_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Kom,
                            rme_ptr_t Begin,
                            rme_ptr_t End,
                            rme_ptr_t Flag)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Kom* Kom_Crt;
    rme_ptr_t Kom_Begin;
    rme_ptr_t Kom_End;
    rme_ptr_t Type_Stat;
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Kom,struct RME_Cap_Kom*,Kom_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Kom_Crt);
    
    /* Align addresses */
#if(RME_KOM_SLOT_ORDER>6U)
    Kom_End=RME_ROUND_DOWN(End+1U,RME_KOM_SLOT_ORDER);
    Kom_Begin=RME_ROUND_UP(Start,RME_KOM_SLOT_ORDER);
#else
    Kom_End=RME_ROUND_DOWN(End+1U,6U);
    Kom_Begin=RME_ROUND_UP(Begin,6U);
#endif

    /* Must at least allow creation of something */
    RME_ASSERT(Flag!=0U);

    /* Header init */
    Kom_Crt->Head.Root_Ref=1U;
    Kom_Crt->Head.Object=0U;
    Kom_Crt->Head.Flag=Flag;
    
    /* Info init */
    Kom_Crt->Begin=Kom_Begin;
    Kom_Crt->End=Kom_End-1U;

    /* Establish cap */
    RME_WRITE_RELEASE(&(Kom_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_KOM,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Kom_Boot_Crt ********************************************/

/* Function:_RME_CPU_Local_Init ***********************************************
Description : Initialize the CPU-local data structure.
Input       : struct RME_CPU_Local* Local - The pointer to the per-CPU values.
              rme_ptr_t CPUID - The CPUID of the CPU.
Output      : None.
Return      : None.
******************************************************************************/
void _RME_CPU_Local_Init(struct RME_CPU_Local* Local,
                         rme_ptr_t CPUID)
{
    rme_ptr_t Prio_Cnt;
    
    Local->CPUID=CPUID;
    Local->Thd_Cur=RME_NULL;
    Local->Sig_Vct=RME_NULL;
    Local->Sig_Tim=RME_NULL;
    
    /* Initialize the run-queue and bitmap */
    for(Prio_Cnt=0U;Prio_Cnt<RME_PREEMPT_PRIO_NUM;Prio_Cnt++)
    {
        Local->Run.Bitmap[Prio_Cnt>>RME_WORD_ORDER]=0U;
        _RME_List_Crt(&(Local->Run.List[Prio_Cnt]));
    }
}
/* End Function:_RME_CPU_Local_Init ******************************************/

/* Function:_RME_Thd_Fatal ***************************************************
Description : The fatal fault handler of RME. This handler will be called by
              the ISR that handles the exceptions. This indicates that a fatal
              exception has happened and we need to see if this thread is in a
              synchronous invocation. If yes, we stop the invocation, and
              possibly return a fault value to the old register set. If not, we
              just kill the thread. If the thread is killed, a notification
              will be sent to its scheduler, and if we try to delegate time to
              it, the time delegation will just fail. A thread execution set is
              required to clear the exception pending status of the thread.
              Some processors may raise some exceptions that are difficult to
              attribute to a particular thread, either due to the fact that
              they are asynchronous, or they are derived from exception entry.
              A good example is ARMv7-M: its autostacking feature derives fault
              from exception entry, and some of its bus faults are asynchronous
              and can cross context boundaries. RME requires that all these
              exceptions be dropped rather than handled; or there will be 
              integrity and availability compromises.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Reg_Struct* Reg - The updated register set.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Thd_Fatal(struct RME_Reg_Struct* Reg)
{
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    
    /* Attempt to return from the invocation, from fault */
    if(_RME_Inv_Ret(Reg,0U,1U)!=0)
    {
        RME_COVERAGE_MARKER();

        /* Return failure, we are not in an invocation. Killing the thread now */
        Local=RME_CPU_LOCAL();
        Thd_Cur=Local->Thd_Cur;
        
        /* Are we attempting to kill the init threads? If yes, panic */
        if(Thd_Cur->Sched.Slice==RME_THD_INIT_TIME)
        {
            RME_COVERAGE_MARKER();
            RME_DBG_S("Attempted to kill init thread.");
            RME_ASSERT(0U);
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Deprive it of all its timeslices */
        Thd_Cur->Sched.Slice=0U;
        
        /* Set the fault flag and reason of the fault */
        Thd_Cur->Sched.State=RME_THD_EXC_PEND;
        _RME_Run_Del(Thd_Cur);
        
        /* Send a scheduler notification to its parent */
        _RME_Run_Notif(Thd_Cur);
        
        /* All kernel send complete, now pick the highest priority thread to run */
        _RME_Kern_High(Reg,Local);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
        
    return 0;
}
/* End Function:_RME_Thd_Fatal ***********************************************/

/* Function:_RME_Run_Ins ******************************************************
Description : Insert a thread into the runqueue. In this function we do not
              check if the thread is on the current core, or is runnable,
              because it should have been checked by someone else.
Input       : struct RME_Thd_Struct* Thd - The thread to insert.
              rme_ptr_t CPUID - The cpu to consult.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Run_Ins(struct RME_Thd_Struct* Thd)
{
    rme_ptr_t Prio;
    struct RME_CPU_Local* Local;
    
    Prio=Thd->Sched.Prio;
    Local=Thd->Sched.Local;
    
    /* It can't be free or there must be an error */
    RME_ASSERT(Local!=RME_THD_FREE);
    
    /* Insert this thread into the runqueue */
    _RME_List_Ins(&(Thd->Sched.Run),
                  Local->Run.List[Prio].Prev,
                  &(Local->Run.List[Prio]));
    
    /* Set the bit in the bitmap */
    Local->Run.Bitmap[Prio>>RME_WORD_ORDER]|=
    RME_POW2(Prio&RME_MASK_END(RME_WORD_ORDER-1U));

    return 0;
}
/* End Function:_RME_Run_Ins *************************************************/

/* Function:_RME_Run_Del ******************************************************
Description : Delete a thread from the runqueue.
Input       : struct RME_Thd_Struct* Thd - The thread to delete.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Run_Del(struct RME_Thd_Struct* Thd)
{
    rme_ptr_t Prio;
    struct RME_CPU_Local* Local;
    
    Prio=Thd->Sched.Prio;
    Local=Thd->Sched.Local;
    /* It can't be free or there must be an error */
    RME_ASSERT(Local!=RME_THD_FREE);
    
    /* Delete this thread from the runqueue */
    _RME_List_Del(Thd->Sched.Run.Prev, Thd->Sched.Run.Next);
    
    /* See if there are any thread on this priority level. If no, clear the bit */
    if(Local->Run.List[Prio].Next==&(Local->Run.List[Prio]))
    {
        RME_COVERAGE_MARKER();

        Local->Run.Bitmap[Prio>>RME_WORD_ORDER]&=
        ~(RME_POW2(Prio&RME_MASK_END(RME_WORD_ORDER-1U)));
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
/* End Function:_RME_Run_Del *************************************************/

/* Function:_RME_Run_High *****************************************************
Description : Find the thread with the highest priority on the core.
Input       : struct RME_CPU_Local* Local - The CPU-local data structure.
Output      : None.
Return      : struct RME_Thd_Struct* - The thread returned.
******************************************************************************/
struct RME_Thd_Struct* _RME_Run_High(struct RME_CPU_Local* Local)
{
    rme_cnt_t Count;
    rme_ptr_t Prio;
    
    /* We start looking for preemption priority levels from the highest */
    for(Count=(rme_cnt_t)(RME_PRIO_WORD_NUM-1U);Count>=0;Count--)
    {
        if(Local->Run.Bitmap[Count]!=0U)
        {
            RME_COVERAGE_MARKER();
            
            break;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    
    /* It must be possible to find one thread per core */
    RME_ASSERT(Count>=0);

    /* Get the first "1"'s position in the word */
    Prio=RME_MSB_GET(Local->Run.Bitmap[Count]);
    Prio+=((rme_ptr_t)Count)<<RME_WORD_ORDER;

    /* Now there is something at this priority level. Get it and start to run */
    return (struct RME_Thd_Struct*)(Local->Run.List[Prio].Next);
}
/* End Function:_RME_Run_High ************************************************/

/* Function:_RME_Run_Notif ****************************************************
Description : Send a notification to the thread's parent, to notify that this 
              thread is currently out of time, or have a fault.
              This function includes kernel send, so we need to call 
              _RME_Kern_High after this. The only exception being the
              _RME_Thd_Swt system call, which we use a more optimized routine.
Input       : struct RME_Thd_Struct* Thd - The thread to send notification for.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Run_Notif(struct RME_Thd_Struct* Thd)
{
    struct RME_Thd_Struct* Sched_Thd;
    
    Sched_Thd=Thd->Sched.Sched_Thd;
    
    /* See if there is already a notification. If yes, do not do the send again */
    if(Thd->Sched.Notif.Next==&(Thd->Sched.Notif))
    {
        RME_COVERAGE_MARKER();

        _RME_List_Ins(&(Thd->Sched.Notif), 
                      Sched_Thd->Sched.Event.Prev,&(Sched_Thd->Sched.Event));
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* If this guy have an endpoint, send to it */
    if(Thd->Sched.Sched_Sig!=0U)
    {
        RME_COVERAGE_MARKER();
        _RME_Kern_Snd(Thd->Sched.Sched_Sig);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
/* End Function:_RME_Run_Notif ***********************************************/

/* Function:_RME_Thd_Pgt ******************************************************
Description : Get a thread's page table. 
Input       : struct RME_Thd_Struct* Thd - The thread.
Output      : None.
Return      : struct RME_Cap_Pgt* / rme_ptr_t - The page table.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
struct RME_Cap_Pgt* _RME_Thd_Pgt(struct RME_Thd_Struct* Thd)
#else
rme_ptr_t _RME_Thd_Pgt(struct RME_Thd_Struct* Thd)
#endif
{
    struct RME_Inv_Struct* Inv_Top;
    
    Inv_Top=RME_INVSTK_TOP(Thd);
    
    if(Inv_Top==RME_NULL)
    {
        RME_COVERAGE_MARKER();

        return Thd->Sched.Prc->Pgt;
    }
    else
    {
        RME_COVERAGE_MARKER();

        return Inv_Top->Prc->Pgt;
    }
}
/* End Function:_RME_Thd_Pgt *************************************************/

/* Function:_RME_Run_Swt ******************************************************
Description : Switch the register set and page table to another thread. 
Input       : struct RME_Reg_Struct* Reg - The register set.
              struct RME_Thd_Struct* Thd_Cur - The current thread.
              struct RME_Thd_Struct* Thd_New - The next thread.
Output      : None.
Return      : rme_ret_t - Always 0.
******************************************************************************/
rme_ret_t _RME_Run_Swt(struct RME_Reg_Struct* Reg,
                       struct RME_Thd_Struct* Thd_Cur, 
                       struct RME_Thd_Struct* Thd_New)
{
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Pgt_Cur;
    struct RME_Cap_Pgt* Pgt_New;
#else
    rme_ptr_t Pgt_Cur;
    rme_ptr_t Pgt_New;
#endif
    struct RME_Reg_Struct* Reg_Cur;
    struct RME_Reg_Struct* Reg_New;
    
    Reg_Cur=&(Thd_Cur->Ctx.Reg->Reg);
    Reg_New=&(Thd_New->Ctx.Reg->Reg);
    
    /* Swap normal context */
    __RME_Thd_Reg_Copy(Reg_Cur, Reg);
    __RME_Thd_Reg_Copy(Reg, Reg_New);
    
    /* If coprocessor is enabled, handle coprocessor context as well */
#if(RME_COP_NUM!=0U)
    __RME_Thd_Cop_Swap(RME_THD_ATTR(Thd_New->Ctx.Hyp_Attr),
                       Reg_New,Thd_New->Ctx.Reg->Cop,
                       RME_THD_ATTR(Thd_Cur->Ctx.Hyp_Attr),
                       Reg_Cur,Thd_Cur->Ctx.Reg->Cop);
#endif

    /* Are we going to switch page tables? If yes, we change it now */
    Pgt_Cur=_RME_Thd_Pgt(Thd_Cur);
    Pgt_New=_RME_Thd_Pgt(Thd_New);
    
#if(RME_PGT_RAW_USER==0U)
    /* The page tables here must be root cap */
    RME_ASSERT(RME_CAP_IS_ROOT(Pgt_Cur)!=0U);
    RME_ASSERT(RME_CAP_IS_ROOT(Pgt_New)!=0U);
#endif
    
#if(RME_PGT_RAW_USER==0U)
    if(RME_CAP_GETOBJ(Pgt_Cur,rme_ptr_t)!=RME_CAP_GETOBJ(Pgt_New,rme_ptr_t))
#else
    if(Pgt_Cur!=Pgt_New)
#endif
    {
        RME_COVERAGE_MARKER();
        
        __RME_Pgt_Set(Pgt_New);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
/* End Function:_RME_Run_Swt *************************************************/

/* Function:_RME_Prc_Boot_Crt *************************************************
Description : Create a process. A process is in fact a protection domain
              associated with a set of capabilities.
              This function does not require a kernel memory capability, and is
              only used to create the first process at boot-time.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Crt - The capability to the capability table to
                                      put this process capability in.
                                      2-Level.
              rme_cid_t Cap_Prc - The capability slot that you want this newly
                                  created process capability to be in.
                                  1-Level.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this process.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability to the page table to use for
                                  this process.
                                  2-Level.
              rme_ptr_t Raw_Pgt - Alternate user-supplied page table physical
                                  address.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Prc_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt_Crt,
                            rme_cid_t Cap_Prc,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Pgt)
#else
rme_ret_t _RME_Prc_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt_Crt,
                            rme_cid_t Cap_Prc,
                            rme_cid_t Cap_Cpt,
                            rme_ptr_t Raw_Pgt)
#endif
{
    struct RME_Cap_Cpt* Cpt_Crt;
    struct RME_Cap_Cpt* Cpt_Op;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Pgt_Op;
#endif
    struct RME_Cap_Prc* Prc_Crt;
    struct RME_Cap_Cpt* Prc_Cpt;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Prc_Pgt;
#endif
    rme_ptr_t Type_Stat;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Crt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Crt,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
#if(RME_PGT_RAW_USER==0U)
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);
#endif
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Crt,RME_CPT_FLAG_CRT);
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_PRC_CRT);
#if(RME_PGT_RAW_USER==0U)
    RME_CAP_CHECK(Pgt_Op,RME_PGT_FLAG_PRC_CRT);
#endif
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Crt,Cap_Prc,struct RME_Cap_Prc*,Prc_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Prc_Crt);

    /* Header init */
    Prc_Crt->Head.Root_Ref=1U;
    Prc_Crt->Head.Object=0U;
    Prc_Crt->Head.Flag=RME_PRC_FLAG_INV|RME_PRC_FLAG_THD;

    /* Info init */
    Prc_Cpt=RME_CAP_CONV_ROOT(Cpt_Op,struct RME_Cap_Cpt*);
#if(RME_PGT_RAW_USER==0U)
    Prc_Pgt=RME_CAP_CONV_ROOT(Pgt_Op,struct RME_Cap_Pgt*);
#endif
    Prc_Crt->Cpt=Prc_Cpt;
#if(RME_PGT_RAW_USER==0U)
    Prc_Crt->Pgt=Prc_Pgt;
#else
    Prc_Crt->Pgt=Raw_Pgt;
#endif
    
    /* Reference objects */
    RME_FETCH_ADD(&(Prc_Cpt->Head.Root_Ref),1U);
#if(RME_PGT_RAW_USER==0U)
    RME_FETCH_ADD(&(Prc_Pgt->Head.Root_Ref),1U);
#endif

    /* Establish cap */
    RME_WRITE_RELEASE(&(Prc_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_PRC,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Prc_Boot_Crt ********************************************/

/* Function:_RME_Prc_Crt ******************************************************
Description : Create a process. A process is in fact a protection domain
              associated with a set of capabilities.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt_Crt - The capability to the capability table to
                                      put this process capability in.
                                      2-Level.
              rme_cid_t Cap_Prc - The capability slot that you want this newly
                                  created process capability to be in.
                                  1-Level.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this process.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability to the page table to use for
                                  this process.
                                  2-Level.
              rme_ptr_t Raw_Pgt - Alternate user-supplied page table physical
                                  address.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Prc_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Crt,
                       rme_cid_t Cap_Prc,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Pgt)
#else
rme_ret_t _RME_Prc_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt_Crt,
                       rme_cid_t Cap_Prc,
                       rme_cid_t Cap_Cpt,
                       rme_ptr_t Raw_Pgt)
#endif
{
    struct RME_Cap_Cpt* Cpt_Crt;
    struct RME_Cap_Cpt* Cpt_Op;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Pgt_Op;
#endif
    struct RME_Cap_Prc* Prc_Crt;
    struct RME_Cap_Cpt* Prc_Cpt;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Prc_Pgt;
#endif
    rme_ptr_t Type_Stat;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt_Crt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Crt,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
#if(RME_PGT_RAW_USER==0U)
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_Op,Type_Stat);
#endif
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Crt,RME_CPT_FLAG_CRT);
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_PRC_CRT);
#if(RME_PGT_RAW_USER==0U)
    RME_CAP_CHECK(Pgt_Op,RME_PGT_FLAG_PRC_CRT);
#endif
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Crt,Cap_Prc,struct RME_Cap_Prc*,Prc_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Prc_Crt);
    
    /* Header init */
    Prc_Crt->Head.Root_Ref=0U;
    Prc_Crt->Head.Object=0U;
    Prc_Crt->Head.Flag=RME_PRC_FLAG_ALL;
    
    /* Info init */
    Prc_Cpt=RME_CAP_CONV_ROOT(Cpt_Op,struct RME_Cap_Cpt*);
#if(RME_PGT_RAW_USER==0U)
    Prc_Pgt=RME_CAP_CONV_ROOT(Pgt_Op,struct RME_Cap_Pgt*);
#endif
    Prc_Crt->Cpt=Prc_Cpt;
#if(RME_PGT_RAW_USER==0U)
    Prc_Crt->Pgt=Prc_Pgt;
#else
    Prc_Crt->Pgt=Raw_Pgt;
#endif
    
    /* Reference objects */
    RME_FETCH_ADD(&(Prc_Cpt->Head.Root_Ref),1U);
#if(RME_PGT_RAW_USER==0U)
    RME_FETCH_ADD(&(Prc_Pgt->Head.Root_Ref),1U);
#endif

    /* Establish cap */
    RME_WRITE_RELEASE(&(Prc_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_PRC,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Prc_Crt *************************************************/

/* Function:_RME_Prc_Del ******************************************************
Description : Delete a process.
Input       : struct RME_Cap_Cpt* Cpt - The capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table.
                                  2-Level.
              rme_cid_t Cap_Prc - The capability to the process.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Prc_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Prc)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Prc* Prc_Del;
    rme_ptr_t Type_Stat;
    /* For deletion use */
    struct RME_Cap_Cpt* Prc_Cpt;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Prc_Pgt;
#endif

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);

    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Prc,struct RME_Cap_Prc*,Prc_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Prc_Del,Type_Stat,RME_CAP_TYPE_PRC);

    /* Remember for deletion */
    Prc_Cpt=Prc_Del->Cpt;
#if(RME_PGT_RAW_USER==0U)
    Prc_Pgt=Prc_Del->Pgt;
#endif

    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Prc_Del,Type_Stat);

    /* Dereference caps */
    RME_FETCH_ADD(&(Prc_Cpt->Head.Root_Ref),-1);
#if(RME_PGT_RAW_USER==0U)
    RME_FETCH_ADD(&(Prc_Pgt->Head.Root_Ref),-1);
#endif
    
    return 0;
}
/* End Function:_RME_Prc_Del *************************************************/

/* Function:_RME_Prc_Cpt ******************************************************
Description : Change a process's capability table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Prc - The capability to the process that have been
                                  created already.
                                  2-Level.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this process.
                                  2-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Prc_Cpt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Prc,
                       rme_cid_t Cap_Cpt)
{
    struct RME_Cap_Prc* Prc_Op;
    struct RME_Cap_Cpt* Cpt_New;
    struct RME_Cap_Cpt* Cpt_Old;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Prc,RME_CAP_TYPE_PRC,
                   struct RME_Cap_Prc*,Prc_Op,Type_Stat); 
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_New,Type_Stat);     
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Prc_Op,RME_PRC_FLAG_CPT);
    RME_CAP_CHECK(Cpt_New,RME_CPT_FLAG_PRC_CPT);
    
    /* Convert to root */
    Cpt_New=RME_CAP_CONV_ROOT(Cpt_New,struct RME_Cap_Cpt*);
    
    /* Commit the change */
    Cpt_Old=Prc_Op->Cpt;
    if(RME_COMP_SWAP((rme_ptr_t*)(&(Prc_Op->Cpt)),
                     (rme_ptr_t)Cpt_Old,(rme_ptr_t)Cpt_New)==RME_CASFAIL)
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PTH_CONFLICT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Reference new table and dereference the old table */
    RME_FETCH_ADD(&(Cpt_New->Head.Root_Ref),1);
    RME_FETCH_ADD(&(Cpt_Old->Head.Root_Ref),-1);

    return 0;
}
/* End Function:_RME_Prc_Cpt *************************************************/

/* Function:_RME_Prc_Pgt ******************************************************
Description : Change a process's page table.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Prc - The capability to the process that have been
                                  created already.
                                  2-Level.
              rme_cid_t Cap_Pgt - The capability to the page table to use for
                                  this process.
                                  2-Level.
              rme_ptr_t Raw_Pgt - Alternate user-supplied page table physical
                                  address.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
#if(RME_PGT_RAW_USER==0U)
rme_ret_t _RME_Prc_Pgt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Prc,
                       rme_cid_t Cap_Pgt)
#else
rme_ret_t _RME_Prc_Pgt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Prc,
                       rme_ptr_t Raw_Pgt)
#endif
{
    struct RME_Cap_Prc* Prc_Op;
#if(RME_PGT_RAW_USER==0U)
    struct RME_Cap_Pgt* Pgt_New;
    struct RME_Cap_Pgt* Pgt_Old;
#else
    rme_ptr_t Pgt_Old;
#endif
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Prc,RME_CAP_TYPE_PRC,
                   struct RME_Cap_Prc*,Prc_Op,Type_Stat); 
#if(RME_PGT_RAW_USER==0U)
    RME_CPT_GETCAP(Cpt,Cap_Pgt,RME_CAP_TYPE_PGT,
                   struct RME_Cap_Pgt*,Pgt_New,Type_Stat);
#endif
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Prc_Op,RME_PRC_FLAG_PGT);
#if(RME_PGT_RAW_USER==0U)
    RME_CAP_CHECK(Pgt_New,RME_PGT_FLAG_PRC_PGT);
#endif
    
    Pgt_Old=Prc_Op->Pgt;
    
#if(RME_PGT_RAW_USER==0U)
    /* Convert to root */
    Pgt_New=RME_CAP_CONV_ROOT(Pgt_New, struct RME_Cap_Pgt*);
    /* Actually commit the change */
    if(RME_COMP_SWAP((rme_ptr_t*)(&(Prc_Op->Pgt)),
                     (rme_ptr_t)Pgt_Old,
                     (rme_ptr_t)Pgt_New)==RME_CASFAIL)
#else
    /* Actually commit the change */
    if(RME_COMP_SWAP((rme_ptr_t*)(&(Prc_Op->Pgt)),
                     (rme_ptr_t)Pgt_Old,
                     Raw_Pgt)==RME_CASFAIL)
#endif
    {
        RME_COVERAGE_MARKER();
        
        return RME_ERR_PTH_CONFLICT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Reference new table and dereference the old table */
#if(RME_PGT_RAW_USER==0U)
    RME_FETCH_ADD(&(Pgt_New->Head.Root_Ref),1);
    RME_FETCH_ADD(&(Pgt_Old->Head.Root_Ref),-1);
#endif
    
    return 0;
}
/* End Function:_RME_Prc_Pgt *************************************************/

/* Function:_RME_Thd_Boot_Crt *************************************************
Description : Create a boot-time thread. The boot-time thread is per-core, and
              will have infinite budget, and has no parent. This function
              allows creation of a thread on behalf of other processors,
              by passing a CPUID parameter. Therefore, it is recommended to
              create the threads sequentially during boot-up; if you create
              threads in parallel, be sure to only create the thread on your
              local core.
              This function does not require a kernel memory capability, and 
              the initial threads' maximum priority will be set by the system.
              This thread is a basic thread and hence always have attribute 0.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table.
                                  2-Level.
              rme_cid_t Cap_Thd - The capability slot that you want this newly
                                  created thread capability to be in.
                                  1-Level.
              rme_cid_t Cap_Prc - The capability to the process that it is in.
                                  2-Level.
              rme_ptr_t Vaddr - The virtual address to store the thread.
              rme_ptr_t Prio - The priority level of the thread.
              struct RME_CPU_Local* Local - The CPU-local data structure of the
                                            CPU to bind the thread to.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Thd,
                            rme_cid_t Cap_Prc,
                            rme_ptr_t Vaddr,
                            rme_ptr_t Prio,
                            struct RME_CPU_Local* Local)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Prc* Prc_Op;
    struct RME_Cap_Thd* Thd_Crt;
    struct RME_Cap_Prc* Prc_Root;
    struct RME_Thd_Struct* Thread;
    rme_ptr_t Type_Stat;
    
    /* Check whether the priority level is allowed */
    if(Prio>=RME_PREEMPT_PRIO_NUM)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_PRIO;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat); 
    RME_CPT_GETCAP(Cpt,Cap_Prc,RME_CAP_TYPE_PRC,
                   struct RME_Cap_Prc*,Prc_Op,Type_Stat);   
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    RME_CAP_CHECK(Prc_Op,RME_PRC_FLAG_THD);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Thd_Crt);
     
    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr,RME_THD_SIZE(0U))!=0)
    {
        RME_COVERAGE_MARKER();

        RME_WRITE_RELEASE(&(Thd_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Object init */
    Thread=(struct RME_Thd_Struct*)Vaddr;
    /* The TID of these threads are by default taken care of by the kernel */
    Thread->Sched.TID=0U;
    Thread->Sched.Slice=RME_THD_INIT_TIME;
    Thread->Sched.State=RME_THD_RUNNING;
    Prc_Root=RME_CAP_CONV_ROOT(Prc_Op,struct RME_Cap_Prc*);
    Thread->Sched.Prc=Prc_Root;
    Thread->Sched.Signal=0U;
    Thread->Sched.Prio=Prio;
    Thread->Sched.Prio_Max=RME_PREEMPT_PRIO_NUM-1U;
    /* Set scheduler reference to 1 so cannot be free */
    Thread->Sched.Sched_Ref=1U;
    Thread->Sched.Sched_Sig=0U;
    /* Bind the thread to the current CPU */
    Thread->Sched.Local=Local;
    /* This is a marking that this thread haven't sent any notifications */
    _RME_List_Crt(&(Thread->Sched.Notif));
    _RME_List_Crt(&(Thread->Sched.Event));
    /* Point its pointer to itself - this will never be a hypervisor thread */
    Thread->Ctx.Hyp_Attr=0U;
    Thread->Ctx.Reg=(struct RME_Thd_Reg*)(Vaddr+RME_HYP_SIZE);
    /* Initialize the invocation stack */
    _RME_List_Crt(&(Thread->Ctx.Invstk));
    
    /* Info init */
    Thd_Crt->Head.Root_Ref=1U;
    Thd_Crt->Head.Object=Vaddr;
    /* This can only be a parent, and not a child, and cannot be freed. Additionally,
     * this should not be blocked on any endpoint. Any attempt to block this thread will fail.
     * Setting execution information for this is also prohibited. */
    Thd_Crt->Head.Flag=RME_THD_FLAG_SCHED_PRIO|RME_THD_FLAG_SCHED_PARENT|
                       RME_THD_FLAG_XFER_DST|RME_THD_FLAG_XFER_SRC|
                       RME_THD_FLAG_SCHED_RCV|RME_THD_FLAG_SWT;

    /* Referece process */
    RME_FETCH_ADD(&(Prc_Root->Head.Root_Ref), 1U);
    
    /* Insert this into the runqueue, and set current thread to it */
    _RME_Run_Ins(Thread);
    Local->Thd_Cur=Thread;

    /* Establish cap */
    RME_WRITE_RELEASE(&(Thd_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_THD,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Thd_Boot_Crt ********************************************/

/* Function:_RME_Thd_Crt ******************************************************
Description : Create a thread. A thread is the minimal kernel-level execution
              unit.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table.
                                  2-Level.
              rme_cid_t Cap_Kom - The kernel memory capability.
                                  2-Level.
              rme_cid_t Cap_Thd - The capability slot that you want this newly
                                  created thread capability to be in.
                                  1-Level.
              rme_cid_t Cap_Prc - The capability to the process that it is in.
                                  2-Level.
              rme_ptr_t Prio_Max - The maximum priority allowed for this
                                   thread. Once set, this cannot be changed.
              rme_ptr_t Raddr - The relative virtual address to store the
                                thread kernel object.
              rme_ptr_t Attr - The context attributes.
              rme_ptr_t Is_Hyp - Whether this is a hypervisor thread.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Kom,
                       rme_cid_t Cap_Thd,
                       rme_cid_t Cap_Prc,
                       rme_ptr_t Prio_Max,
                       rme_ptr_t Raddr,
                       rme_ptr_t Attr,
                       rme_ptr_t Is_Hyp)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Prc* Prc_Op;
    struct RME_Cap_Kom* Kom_Op;
    struct RME_Cap_Thd* Thd_Crt;
    struct RME_Cap_Prc* Prc_Root;
    struct RME_Thd_Struct* Thread;
    rme_ptr_t Type_Stat;
    rme_ptr_t Vaddr;
    rme_ptr_t Size;
    
    /* See if the maximum priority relationship is correct - a thread
     * can never create a thread with higher maximum priority */
    if((RME_CPU_LOCAL()->Thd_Cur)->Sched.Prio_Max<Prio_Max)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_PRIO;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat); 
    RME_CPT_GETCAP(Cpt,Cap_Kom,RME_CAP_TYPE_KOM,
                   struct RME_Cap_Kom*,Kom_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Prc,RME_CAP_TYPE_PRC,
                   struct RME_Cap_Prc*,Prc_Op,Type_Stat);
    /* Check if the target caps is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    RME_CAP_CHECK(Prc_Op,RME_PRC_FLAG_THD);
    /* See if the creation is valid for this kmem range */
    if(Is_Hyp==0U)
    {
        RME_COVERAGE_MARKER();
        
        Size=RME_THD_SIZE(Attr);
    }
    else
    {
        RME_COVERAGE_MARKER();
        
#if(RME_HYP_VA_SIZE==0U)
        return RME_ERR_PTH_HADDR;
#else
        Size=RME_HYP_SIZE;
#endif
    }
    
    RME_KOM_CHECK(Kom_Op,RME_KOM_FLAG_THD,Raddr,Vaddr,Size);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Thd_Crt);
     
    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr,Size)<0)
    {
        RME_COVERAGE_MARKER();

        RME_WRITE_RELEASE(&(Thd_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Object init */
    Thread=(struct RME_Thd_Struct*)Vaddr;
    /* These thread's TID default to 0 */
    Thread->Sched.TID=0U;
    Thread->Sched.Slice=0U;
    Thread->Sched.State=RME_THD_TIMEOUT;
    Prc_Root=RME_CAP_CONV_ROOT(Prc_Op,struct RME_Cap_Prc*);
    Thread->Sched.Prc=Prc_Root;
    Thread->Sched.Signal=0U;
    Thread->Sched.Prio_Max=Prio_Max;
    Thread->Sched.Sched_Ref=0U;
    Thread->Sched.Sched_Sig=0U;
    /* Currently the thread is not bound to any particular CPU */
    Thread->Sched.Local=RME_THD_FREE;
    /* This is a marking that this thread haven't sent any notifications */
    _RME_List_Crt(&(Thread->Sched.Notif));
    _RME_List_Crt(&(Thread->Sched.Event));
    /* Point its pointer to itself - this is not a hypervisor thread yet */
    if(Is_Hyp==0U)
    {
        RME_COVERAGE_MARKER();
        
        Thread->Ctx.Hyp_Attr=Attr;
        Thread->Ctx.Reg=(struct RME_Thd_Reg*)(Vaddr+RME_HYP_SIZE);
    }
    /* Default to HYP_VA_BASE for all created hypervisor threads */
    else
    {
        RME_COVERAGE_MARKER();
        
        Thread->Ctx.Hyp_Attr=Attr|RME_THD_HYP_FLAG;
        Thread->Ctx.Reg=RME_HYP_VA_BASE;
    }
    /* Initialize the invocation stack */
    _RME_List_Crt(&(Thread->Ctx.Invstk));

    /* Header init */
    Thd_Crt->Head.Root_Ref=0U;
    Thd_Crt->Head.Object=Vaddr;
    Thd_Crt->Head.Flag=RME_THD_FLAG_ALL;

    /* Reference process */
    RME_FETCH_ADD(&(Prc_Root->Head.Root_Ref), 1U);
    
    /* Establish cap */
    RME_WRITE_RELEASE(&(Thd_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_THD,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Thd_Crt *************************************************/

/* Function:_RME_Thd_Del ******************************************************
Description : Delete a thread.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table. 
              rme_cid_t Cap_Cpt - The capability to the capability table.
                                  2-Level.
              rme_cid_t Cap_Thd - The capability to the thread in the captbl.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Thd)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Thd* Thd_Del;
    rme_ptr_t Type_Stat;
    /* These are for deletion */
    struct RME_Thd_Struct* Thread;
    struct RME_Inv_Struct* Invocation;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Thd,struct RME_Cap_Thd*,Thd_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Thd_Del,Type_Stat,RME_CAP_TYPE_THD);
    
    /* Get the thread */
    Thread=RME_CAP_GETOBJ(Thd_Del,struct RME_Thd_Struct*);
    
    /* See if the thread is free. If still bound, we cannot proceed to deletion */
    if(Thread->Sched.Local!=RME_THD_FREE)
    {
        RME_COVERAGE_MARKER();

        RME_CAP_DEFROST(Thd_Del,Type_Stat);
        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Thd_Del,Type_Stat);
    
    /* Is the thread using any invocation? If yes, just pop the invocation
     * stack to empty, and free all the invocation stubs. This can be virtually
     * unbounded if the invocation stack is just too deep. This is left to the
     * user; if this is what he or she wants, be our guest. */
    while(Thread->Ctx.Invstk.Next!=&(Thread->Ctx.Invstk))
    {
        Invocation=(struct RME_Inv_Struct*)(Thread->Ctx.Invstk.Next);
        _RME_List_Del(Invocation->Head.Prev,Invocation->Head.Next);
        Invocation->Thd_Act=0U;
    }
    
    /* Dereference the process */
    RME_FETCH_ADD(&(Thread->Sched.Prc->Head.Root_Ref), -1);
    
    /* Try to depopulate the area - this must be successful */
    if((Thread->Ctx.Hyp_Attr&RME_THD_HYP_FLAG)==0U)
    {
        RME_COVERAGE_MARKER();

        RME_ASSERT(_RME_Kot_Erase((rme_ptr_t)Thread,
                                  RME_THD_SIZE(Thread->Ctx.Hyp_Attr))==0);
    }
    else
    {
        RME_COVERAGE_MARKER();

        RME_ASSERT(_RME_Kot_Erase((rme_ptr_t)Thread,
                   RME_HYP_SIZE)==0);
    }
    
    return 0;
}
/* End Function:_RME_Thd_Del *************************************************/

/* Function:_RME_Thd_Exec_Set *************************************************
Description : Set a thread's entry point and stack. The registers will be
              initialized with these contents.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Thd - The capability to the thread.
                                  2-Level.
              rme_ptr_t Entry - The entry address of the thread.
              rme_ptr_t Stack - The stack address to use for execution.
              rme_ptr_t Param - The parameter to pass to the thread.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Exec_Set(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Thd,
                            rme_ptr_t Entry,
                            rme_ptr_t Stack,
                            rme_ptr_t Param)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thread;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_EXEC_SET);
    
    /* See if the target thread is already bound. If no or incorrect, we just quit */
    Thread=RME_CAP_GETOBJ(Thd_Op,struct RME_Thd_Struct*);
    if(Thread->Sched.Local!=RME_CPU_LOCAL())
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if there is a fault pending. If yes, we clear it */
    if(Thread->Sched.State==RME_THD_EXC_PEND)
    {
        RME_COVERAGE_MARKER();

        Thread->Sched.State=RME_THD_TIMEOUT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Commit the change if both values are non-zero. If both are zero we are just
     * clearing the error flag and continue execution from where it faulted */
    if((Entry!=RME_NULL)&&(Stack!=RME_NULL))
    {
        RME_COVERAGE_MARKER();

        __RME_Thd_Reg_Init(RME_THD_ATTR(Thread->Ctx.Hyp_Attr),
                           Entry,Stack,Param,&(Thread->Ctx.Reg->Reg));
#if(RME_COP_NUM!=0U)
        __RME_Thd_Cop_Init(RME_THD_ATTR(Thread->Ctx.Hyp_Attr), 
                           &(Thread->Ctx.Reg->Reg),&(Thread->Ctx.Reg->Cop));
#endif
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    return 0;
}
/* End Function:_RME_Thd_Exec_Set ********************************************/

/* Function:_RME_Thd_Sched_Bind ***********************************************
Description : Set a thread's priority level, and its scheduler thread. When
              there are any state changes on this thread, a notification will
              be sent to its scheduler thread. If the state of the thread
              changes for multiple times, then only the most recent state will
              be reflected in the scheduler's receive queue.
              The scheduler and the threads that it schedule must be on the
              same core. When a thread wants to go from one core to another,
              its notification to the scheduler must all be processed, and it
              must have no scheduler notifications in itself. 
              This must be called on the same core with the Cap_Thd_Sched, and
              the Cap_Thd itself must be free.
              It is impossible to set a thread's priority beyond its maximum
              priority.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Thd - The capability to the thread.
                                  2-Level.
              rme_cid_t Cap_Thd_Sched - The scheduler thread.
                                        2-Level.
              rme_cid_t Cap_Sig - The signal endpoint for scheduler
                                  notifications. This signal endpoint will be
                                  sent to whenever this thread has a fault, or
                                  timeouts. This is purely optional; if it is
                                  not needed, pass in RME_CID_NULL.
              rme_tid_t TID - The thread ID. This is user-supplied, and the
                              kernel will not check whether there are two
                              threads that have the same TID.
              rme_ptr_t Prio - The priority level, higher is more critical.
              rme_ptr_t Haddr - The kernel-accessible virtual memory address
                                for this thread's register sets, only used by
                                hypervisor-managed threads. For other threads,
                                please pass in NULL instead.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Sched_Bind(struct RME_Cap_Cpt* Cpt,
                              rme_cid_t Cap_Thd,
                              rme_cid_t Cap_Thd_Sched,
                              rme_cid_t Cap_Sig,
                              rme_tid_t TID,
                              rme_ptr_t Prio,
                              rme_ptr_t Haddr)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Cap_Thd* Thd_Sched;
    struct RME_Cap_Sig* Sig_Op;
    struct RME_Thd_Struct* Thread;
    struct RME_Thd_Struct* Scheduler;
    struct RME_CPU_Local* Local_Old;
    struct RME_CPU_Local* Local_New;
    rme_ptr_t Type_Stat;
    rme_ptr_t Hyp_Attr;
    rme_ptr_t End;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Thd_Sched,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Sched,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_CHILD);
    RME_CAP_CHECK(Thd_Sched,RME_THD_FLAG_SCHED_PARENT);
    
    /* See if we need the signal endpoint for this operation */
    if(Cap_Sig<RME_CID_NULL)
    {
        RME_COVERAGE_MARKER();

        RME_CPT_GETCAP(Cpt,Cap_Sig,RME_CAP_TYPE_SIG,
                       struct RME_Cap_Sig*,Sig_Op,Type_Stat);
        RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_SCHED);
    }
    else
    {
        RME_COVERAGE_MARKER();

        Sig_Op=RME_NULL;
    }

    /* See if the target thread is already bound. If yes, we just quit */
    Thread=RME_CAP_GETOBJ(Thd_Op,struct RME_Thd_Struct*);
    Local_Old=Thread->Sched.Local;
    if(Local_Old!=RME_THD_FREE)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the parent thread is on the same core with the current processor */
    Local_New=RME_CPU_LOCAL();
    Scheduler=RME_CAP_GETOBJ(Thd_Sched,struct RME_Thd_Struct*);
    if(Scheduler->Sched.Local!=Local_New)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* See if we are trying to bind to ourself. This is prohibited */
    if(Thread==Scheduler)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_NOTIF;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the priority relationship is correct */
    if(Scheduler->Sched.Prio_Max<Prio)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_PRIO;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Check whether the coprocessor context attribute is compatible with this core */
    Hyp_Attr=Thread->Ctx.Hyp_Attr;
#if(RME_COP_NUM!=0U)
    if(__RME_Thd_Cop_Check(RME_THD_ATTR(Hyp_Attr))<0)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
#else
    if(RME_THD_ATTR(Hyp_Attr)!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
#endif
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Check the hypervisor context buffer passed in to see whether it is good */
    if((Haddr!=RME_NULL)&&((Hyp_Attr&RME_THD_HYP_FLAG)!=0U))
    {
        RME_COVERAGE_MARKER();
        
        /* Register save area must be aligned to word boundary */
        if(RME_NOT_ALIGNED(Haddr))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_HADDR;
        }
        else
        {
            RME_COVERAGE_MARKER();
            
            /* It needs to be safely accessible to the kernel as well */
#if(RME_HYP_VA_BASE!=0U)
            if(Haddr<RME_HYP_VA_BASE)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_PTH_HADDR;
            }
            else
            {
#endif
                End=Haddr+RME_REG_SIZE(RME_THD_ATTR(Thread->Ctx.Hyp_Attr));
                if((End<=Haddr)||(End>(RME_HYP_VA_BASE+RME_HYP_VA_SIZE)))
                {
                    RME_COVERAGE_MARKER();

                    return RME_ERR_PTH_HADDR;
                }
                else
                {
                    RME_COVERAGE_MARKER();
                }
#if(RME_HYP_VA_BASE!=0U)
            }
#endif
        }
    }
    /* We don't allow setting HYP addr for normal threads, nor do we allow
     * setting HYP addr to NULL for hypervisor-managed threads. */
    else if(((Haddr!=RME_NULL)&&((Hyp_Attr&RME_THD_HYP_FLAG)==0U))||
            ((Haddr==RME_NULL)&&((Hyp_Attr&RME_THD_HYP_FLAG)!=0U)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_HADDR;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Try to bind the thread */
    if(RME_COMP_SWAP((rme_ptr_t*)&(Thread->Sched.Local),
                     (rme_ptr_t)Local_Old,
                     (rme_ptr_t)Local_New)==RME_CASFAIL)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_CONFLICT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Increase the reference count of the scheduler thread struct - same core */
    Scheduler->Sched.Sched_Ref++;
    
    /* Bind successful. Do operations to finish this. No need to worry about other cores'
     * operations on this thread because this thread is already bound to this core */
    Thread->Sched.Sched_Thd=Scheduler;
    Thread->Sched.Prio=Prio;
    Thread->Sched.TID=(rme_ptr_t)TID;

    /* Tie the signal endpoint to it if not zero */
    if(Sig_Op==0U)
    {
        RME_COVERAGE_MARKER();

        Thread->Sched.Sched_Sig=0U;
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* Convert to root cap */
        Thread->Sched.Sched_Sig=RME_CAP_CONV_ROOT(Sig_Op,struct RME_Cap_Sig*);
        
        /* Increase refcnt */
        RME_FETCH_ADD(&(Thread->Sched.Sched_Sig->Head.Root_Ref),1U);
    }
    
    /* Set hypervisor context address if we're hypervisor-managed */
    if((Thread->Ctx.Hyp_Attr&RME_THD_HYP_FLAG)!=0U)
    {
        RME_COVERAGE_MARKER();
        Thread->Ctx.Reg=(struct RME_Thd_Reg*)Haddr;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    return 0;
}
/* End Function:_RME_Thd_Sched_Bind ******************************************/

/* Function:_RME_Thd_Sched_Prio ***********************************************
Description : Change a thread's priority level. This can only be called from
              the core that have the thread bound. To facilitate scheduling,
              this system call allows up to 3 thread's priority changes per
              call. This system call can cause a potential context switch.
              It is impossible to set a thread's priority beyond its maximum
              priority. 
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Number - The number of threads to adjust priority.
                                 Allowed values are 1, 2 and 3.
              rme_cid_t Cap_Thd0 - The capability to the first thread.
                                   2-Level.
              rme_ptr_t Prio0 - The priority level, higher is more critical.
              rme_cid_t Cap_Thd1 - The capability to the second thread.
                                   2-Level.
              rme_ptr_t Prio1 - The priority level, higher is more critical.
              rme_cid_t Cap_Thd2 - The capability to the third thread.
                                   2-Level.
              rme_ptr_t Prio2 - The priority level, higher is more critical.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Sched_Prio(struct RME_Cap_Cpt* Cpt,
                              struct RME_Reg_Struct* Reg,
                              rme_ptr_t Number,
                              rme_cid_t Cap_Thd0,
                              rme_ptr_t Prio0,
                              rme_cid_t Cap_Thd1,
                              rme_ptr_t Prio1,
                              rme_cid_t Cap_Thd2,
                              rme_ptr_t Prio2)
{
    rme_ptr_t Count;
    rme_cid_t Cap_Thd[3];
    rme_ptr_t Prio[3];
    struct RME_Cap_Thd* Thd_Op[3];
    struct RME_Thd_Struct* Thread[3];
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Thd_Struct* Thd_New;
    rme_ptr_t Type_Stat;
    
    /* Check parameter validity */
    if((Number==0U)||(Number>3U))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    
    /* We'll use arrays in the next */
    Cap_Thd[0]=Cap_Thd0;
    Cap_Thd[1]=Cap_Thd1;
    Cap_Thd[2]=Cap_Thd2;
    Prio[0]=Prio0;
    Prio[1]=Prio1;
    Prio[2]=Prio2;

    Local=RME_CPU_LOCAL();
    for(Count=0U;Count<Number;Count++)
    {
        /* Get the capability slot */
        RME_CPT_GETCAP(Cpt,Cap_Thd[Count],RME_CAP_TYPE_THD,
                       struct RME_Cap_Thd*,Thd_Op[Count],Type_Stat);
        /* Check if the target cap is not frozen and allows such operations */
        RME_CAP_CHECK(Thd_Op[Count],RME_THD_FLAG_SCHED_PRIO);
        
        /* See if the target thread is already bound to this core. If no, we just quit */
        Thread[Count]=(struct RME_Thd_Struct*)(Thd_Op[Count]->Head.Object);
        if(Thread[Count]->Sched.Local!=Local)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_INVSTATE;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* See if the priority relationship is correct */
        if(Thread[Count]->Sched.Prio_Max<Prio[Count])
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_PRIO;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    
    /* Now save the system call return value to the caller stack. We're now sure that all
     * the context switches may be performed without ano possibility of a failure. */
    __RME_Svc_Retval_Set(Reg,0);
    
    /* Do the scheduling for each thread, and we'll switch to the real winner after all
     * these scheduling. This can help remove the excessive overhead. */
    for(Count=0U;Count<Number;Count++)
    {
        /* See if this thread is currently running, or is runnable. If yes, it must be
         * in the run queue. Remove it from there and change priority, after changing
         * priority, put it back, and see if we need a reschedule. If the thread*/
        if((Thread[Count]->Sched.State==RME_THD_RUNNING)||
           (Thread[Count]->Sched.State==RME_THD_READY))
        {
            RME_COVERAGE_MARKER();

            _RME_Run_Del(Thread[Count]);
            Thread[Count]->Sched.Prio=Prio[Count];
            _RME_Run_Ins(Thread[Count]);
        }
        else
        {
            RME_COVERAGE_MARKER();

            Thread[Count]->Sched.Prio=Prio[Count];
        }
    }
    
    /* Get the current highest-priority running thread */
    Thd_New=_RME_Run_High(Local);
    Thd_Cur=Local->Thd_Cur;
    RME_ASSERT(Thd_New->Sched.Prio>=Thd_Cur->Sched.Prio);
    
    /* See if we need a context switch */
    if(Thd_New->Sched.Prio>Thd_Cur->Sched.Prio)
    {
        RME_COVERAGE_MARKER();

        /* This will cause a solid context switch - The current thread will be set
         * to ready, and we will set the thread that we switch to to be running. */
        _RME_Run_Swt(Reg, Thd_Cur, Thd_New);
        Thd_Cur->Sched.State=RME_THD_READY;
        Thd_New->Sched.State=RME_THD_RUNNING;
        Local->Thd_Cur=Thd_New;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    return 0;
}
/* End Function:_RME_Thd_Sched_Prio ******************************************/

/* Function:_RME_Thd_Sched_Free ***********************************************
Description : Free a thread from its current binding. This function can only be
              executed from the same core on with the thread.
              This system call can cause a potential context switch.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              volatile struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Thd - The capability to the thread.
                                  2-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Sched_Free(struct RME_Cap_Cpt* Cpt, 
                              struct RME_Reg_Struct* Reg,
                              rme_cid_t Cap_Thd)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Thread;
    /* These are used to free the thread */
    struct RME_CPU_Local* Local;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_FREE);
    
    /* See if the target thread is already bound. If no or bound to other cores, we just quit */
    Local=RME_CPU_LOCAL();
    Thread=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Thread->Sched.Local!=Local)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Am I referenced by someone as a scheduler? If yes, we cannot unbind. Because
     * boot-time thread's refcnt will never be 0, thus they will never pass this checking */
    if(Thread->Sched.Sched_Ref!=0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_REFCNT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Decrease the parent's reference count - on the same core */
    Thread->Sched.Sched_Thd->Sched.Sched_Ref--;

    /* See if we have any events sent to the parent. If yes, remove that event */
    if(Thread->Sched.Notif.Next!=&(Thread->Sched.Notif))
    {
        RME_COVERAGE_MARKER();

        _RME_List_Del(Thread->Sched.Notif.Prev, Thread->Sched.Notif.Next);
        _RME_List_Crt(&(Thread->Sched.Notif));
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* If we have an scheduler event endpoint, release it */
    if(Thread->Sched.Sched_Sig!=RME_NULL)
    {
        RME_COVERAGE_MARKER();

        RME_FETCH_ADD(&(Thread->Sched.Sched_Sig->Head.Root_Ref),-1);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Now save the system call return value to the caller stack */
    __RME_Svc_Retval_Set(Reg,0);  
    
    /* If the thread is running, or ready to run, kick it out of the run queue.
     * If it is blocked on some endpoint, end the blocking and set the return
     * value to RME_ERR_SIV_FREE. If the thread is killed due to a fault, we will
     * not clear the fault here, and we will wait for the Exec_Set to clear it. */
    if(Thread->Sched.State!=RME_THD_BLOCKED)
    {
        RME_COVERAGE_MARKER();

        if((Thread->Sched.State==RME_THD_RUNNING)||
           (Thread->Sched.State==RME_THD_READY))
        {
            RME_COVERAGE_MARKER();

            _RME_Run_Del(Thread);
            Thread->Sched.State=RME_THD_TIMEOUT;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        /* If it got here, the thread that is operated on cannot be the current thread, so
         * we are not overwriting the return value of the caller thread */
        __RME_Svc_Retval_Set(&(Thread->Ctx.Reg->Reg),RME_ERR_SIV_FREE);
        Thread->Sched.Signal->Thd=RME_NULL;
        Thread->Sched.Signal=RME_NULL;
        Thread->Sched.State=RME_THD_TIMEOUT;
    }
    /* Delete all slices on it */
    Thread->Sched.Slice=0U;
    
    /* See if this thread is the current thread. If yes, then there will be a context switch */
    if(Local->Thd_Cur==Thread)
    {
        RME_COVERAGE_MARKER();

        Local->Thd_Cur=_RME_Run_High(Local);
        _RME_Run_Ins(Local->Thd_Cur);
        Local->Thd_Cur->Sched.State=RME_THD_RUNNING;
        _RME_Run_Swt(Reg, Thread, Local->Thd_Cur);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Set the state to free so other cores can bind */
    RME_WRITE_RELEASE((rme_ptr_t*)&(Thread->Sched.Local),
                      (rme_ptr_t)RME_THD_FREE);

    return 0;
}
/* End Function:_RME_Thd_Sched_Free ******************************************/

/* Function:_RME_Thd_Sched_Rcv ************************************************
Description : Try to receive a notification from the scheduler queue. This
              can only be called from the same core the thread is on.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Thd - The capability to the scheduler thread. We
                                  are going to get timeout or exception
                                  notifications for the threads that it is
                                  responsible for scheduling. This capability
                                  must point to a thread on the same core.
                                  2-Level.
Output      : None.
Return      : rme_ret_t - If successful, the thread ID; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Sched_Rcv(struct RME_Cap_Cpt* Cpt,
                             rme_cid_t Cap_Thd)
{
    struct RME_Cap_Thd* Thd_Op;
    struct RME_Thd_Struct* Scheduler;
    struct RME_Thd_Struct* Thread;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Op,RME_THD_FLAG_SCHED_RCV);
    
    /* Check if the CPUID is correct. Only if yes can we proceed */
    Scheduler=(struct RME_Thd_Struct*)Thd_Op->Head.Object;
    if(Scheduler->Sched.Local!=RME_CPU_LOCAL())
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Are there any notifications? */
    if(Scheduler->Sched.Event.Next==&(Scheduler->Sched.Event))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_NOTIF;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Return one notification and delete it from the notification list */
    Thread=(struct RME_Thd_Struct*)(Scheduler->Sched.Event.Next-1U);
    _RME_List_Del(Thread->Sched.Notif.Prev,Thread->Sched.Notif.Next);
    /* We need to do this because we are using this to detect whether the notification is sent */
    _RME_List_Crt(&(Thread->Sched.Notif));
    
    /* See if the child has an exception. If yes, we return an exception flag with its TID */
    if(Thread->Sched.State==RME_THD_EXC_PEND)
    {
        RME_COVERAGE_MARKER();
        
        return (rme_ret_t)(Thread->Sched.TID|RME_THD_EXC_FLAG);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Return the notification TID, which means that it is just a timeout */
    return (rme_ret_t)(Thread->Sched.TID);
}
/* End Function:_RME_Thd_Sched_Rcv *******************************************/

/* Function:_RME_Thd_Time_Xfer ************************************************
Description : Transfer time from one thread to another. This can only be called
              from the core that the thread is on, and the the two threads
              involved in the time transfer must be on the same core.
              If the time transfered is more than or equal to what the source
              have, the source will be out of time or blocked. If the source is
              both out of time and blocked, we do not send the notification;
              Instead, we send the notification when the receive endpoint
              actually receive something.
              It is possible to transfer time to threads have a lower priority,
              and it is also possible to transfer time to threads that have a
              higher priority. In the latter case, and if the source is
              currently running, a preemption will occur. However, it is not
              allowed to transfer time to a thread with higher maximum priority,
              and this guarantees that the quality of time can only degrade.
              There are 3 kinds of threads in the system:
              1. Init threads - They are created at boot-time and have infinite
                                budget.
              2. Infinite threads - They are created later but have infinite
                                    budget.
              3. Normal threads - They are created later and have a finite
                                  budget.
              There are 3 kinds of transfer in the system:
              1. Normal transfers - They transfer a finite budget.
              2. Infinite transfers - They attempt to transfer an infinite
                                      budget but will not revoke the timeslices
                                      of the source if the source have infinite
                                      budget.
              3. Revoking transfers - They attempt to transfer an infinite
                                      budget but will revoke the timeslices of
                                      the source if the source is an infinite
                                      thread.
              Normal budget transferring rules:
              -----------------------------------------------------------------
                Nom   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      --      |      T-
              -----------------------------------------------------------------
                      | Infinite |      --      |      --      |      T-
              -----------------------------------------------------------------
                      |  Normal  |      -A      |      TA      |      TA
              -----------------------------------------------------------------
              Infinite budget transferring rules:
              -----------------------------------------------------------------
                Inf   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      --      |      S-
              -----------------------------------------------------------------
                      | Infinite |      --      |      --      |      S-
              -----------------------------------------------------------------
                      |  Normal  |      -I      |      -I      |      TA
              -----------------------------------------------------------------
              Revoking budget transferring rules:
              -----------------------------------------------------------------
                Rev   |   From   |     Init     |   Infinite   |    Normal
              -----------------------------------------------------------------
                 To   |   Init   |      --      |      S-      |      S-
              -----------------------------------------------------------------
                      | Infinite |      --      |      S-      |      S-
              -----------------------------------------------------------------
                      |  Normal  |      -I      |      SI      |      TA
              -----------------------------------------------------------------
              Notations:
              -:Nothing will happen on source/destination.
              T:Source timeout if all transferred.
              S:Source will definitely timeout.
              A:Destination accept if not overflow.
              I:Destination will definitely become an infinite thread.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Thd_Dst - The destination thread.
                                      2-Level.
              rme_cid_t Cap_Thd_Src - The source thread.
                                      2-Level.
              rme_ptr_t Time - The time to transfer, in slices.
                               Use RME_THD_INIT_TIME for revoking transfer.
                               Use RME_THD_INF_TIME for infinite trasnfer.
Output      : None.
Return      : rme_ret_t - If successful, the destination time amount; or an
                          error code.
******************************************************************************/
rme_ret_t _RME_Thd_Time_Xfer(struct RME_Cap_Cpt* Cpt,
                             struct RME_Reg_Struct* Reg,
                             rme_cid_t Cap_Thd_Dst,
                             rme_cid_t Cap_Thd_Src,
                             rme_ptr_t Time)
{
    struct RME_Cap_Thd* Thd_Dst_Op;
    struct RME_Cap_Thd* Thd_Src_Op;
    struct RME_Thd_Struct* Thd_Dst;
    struct RME_Thd_Struct* Thd_Src;
    struct RME_CPU_Local* Local;
    rme_ptr_t Time_Xfer;
    rme_ptr_t Type_Stat;
    
    /* We may allow transferring infinite time here */
    if(Time==0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Thd_Dst,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Dst_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Thd_Src,RME_CAP_TYPE_THD,
                   struct RME_Cap_Thd*,Thd_Src_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Thd_Dst_Op,RME_THD_FLAG_XFER_DST);
    RME_CAP_CHECK(Thd_Src_Op,RME_THD_FLAG_XFER_SRC);

    /* Check if the two threads are on the core that is accordance with what we are on */
    Local=RME_CPU_LOCAL();
    Thd_Src=RME_CAP_GETOBJ(Thd_Src_Op,struct RME_Thd_Struct*);
    if(Thd_Src->Sched.Local!=Local)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Do we have slices to transfer? - slices == 0 implies TIMEOUT, or BLOCKED, or even EXC_PEND */
    if(Thd_Src->Sched.Slice==0U)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    Thd_Dst=RME_CAP_GETOBJ(Thd_Dst_Op,struct RME_Thd_Struct*);
    
    if(Thd_Dst->Sched.Local!=Local)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_INVSTATE;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* The destination must never have higher maximum priority than the source,
     * unless it is a init thread which could be used as a black hole */
    if((Thd_Src->Sched.Prio_Max<Thd_Dst->Sched.Prio_Max)&&
       (Thd_Dst->Sched.Slice!=RME_THD_INIT_TIME))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_PRIO;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* See if the destination is in a fault. If yes, cancel the transfer */
    if(Thd_Dst->Sched.State==RME_THD_EXC_PEND)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_PTH_EXC;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Delegating from a normal thread */
    if(Thd_Src->Sched.Slice<RME_THD_INF_TIME)
    {
        RME_COVERAGE_MARKER();

        /* Delegate all our time */
        if(Time>=RME_THD_INF_TIME)
        {
            RME_COVERAGE_MARKER();

            Time_Xfer=Thd_Src->Sched.Slice;
        }
        /* Delegate some time, if not sufficient, clean up the source time */
        else
        {
            RME_COVERAGE_MARKER();
            
            if(Thd_Src->Sched.Slice>Time)
            {
                RME_COVERAGE_MARKER();

                Time_Xfer=Time;
            }
            else
            {
                RME_COVERAGE_MARKER();

                Time_Xfer=Thd_Src->Sched.Slice;
            }
        }
        
        /* See if we are transferring to an infinite budget thread. If yes, we
         * are revoking timeslices; If not, this is a finite transfer */
        if(Thd_Dst->Sched.Slice<RME_THD_INF_TIME)
        {
            RME_COVERAGE_MARKER();
            
            RME_TIME_CHECK(Thd_Dst->Sched.Slice,Time_Xfer);
            Thd_Dst->Sched.Slice+=Time_Xfer;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        Thd_Src->Sched.Slice-=Time_Xfer;
    }
    /* Delegating from init or infinite thread */
    else
    {
        RME_COVERAGE_MARKER();

        /* Infinite transfer to the destination */
        if(Time>=RME_THD_INF_TIME)
        {
            RME_COVERAGE_MARKER();

            /* This transfer will revoke the infinite budget */
            if(Time==RME_THD_INIT_TIME)
            {
                RME_COVERAGE_MARKER();
                
                /* Will not revoke, source is an init thread */
                if(Thd_Src->Sched.Slice!=RME_THD_INIT_TIME)
                {
                    RME_COVERAGE_MARKER();
                    
                    Thd_Src->Sched.Slice=0U;
                }
                else
                {
                    RME_COVERAGE_MARKER();
                }
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
            
            /* Set destination to infinite if it is not an init thread */
            if(Thd_Dst->Sched.Slice<RME_THD_INF_TIME)
            {
                RME_COVERAGE_MARKER();
                
                Thd_Dst->Sched.Slice=RME_THD_INF_TIME;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* Just increase the budget of the other thread - check first */
            RME_TIME_CHECK(Thd_Dst->Sched.Slice,Time);
            Thd_Dst->Sched.Slice+=Time;
        }
    }

    /* Is the source time used up? If yes, delete it from the run queue, and notify its 
     * parent. If it is not in the run queue, The state of the source must be BLOCKED. */
    if(Thd_Src->Sched.Slice==0U)
    {
        RME_COVERAGE_MARKER();
        
        /* If it is blocked, we do not change its state, and only sends the scheduler notification */
        if((Thd_Src->Sched.State==RME_THD_RUNNING)||
           (Thd_Src->Sched.State==RME_THD_READY))
        {
            RME_COVERAGE_MARKER();
            
            _RME_Run_Del(Thd_Src);
            Thd_Src->Sched.State=RME_THD_TIMEOUT;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }

        /* Notify the parent about this */
        _RME_Run_Notif(Thd_Src);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Now save the system call return value to the caller 
     * stack - how much time the destination have now */
    __RME_Svc_Retval_Set(Reg,(rme_ret_t)(Thd_Dst->Sched.Slice));

    /* See what was the state of the destination thread. If it is timeout,
     * then activate it. If it is other state, then leave it alone */
    if(Thd_Dst->Sched.State==RME_THD_TIMEOUT)
    {
        RME_COVERAGE_MARKER();
        
        Thd_Dst->Sched.State=RME_THD_READY;
        _RME_Run_Ins(Thd_Dst);
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* All possible kernel send (scheduler notifications) done,
     * now pick the highest priority thread to run */
    _RME_Kern_High(Reg,Local);

    return 0;
}
/* End Function:_RME_Thd_Time_Xfer *******************************************/

/* Function:_RME_Thd_Swt ******************************************************
Description : Switch to another thread. The thread to switch to must have the same
              preemptive priority as this thread, and have time, and not blocked.
              If trying to switch to a higher priority thread, this is impossible
              because whenever a thread with higher priority exists in the system,
              the kernel wiull let it preempt the current thread. 
              If trying to switch to a lower priority thread, this is impossible
              because the current thread just preempts it after the thread switch.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table. 
              volatile struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Thd - The capability to the thread. If this is -1,
                                  the kernel will pickup whatever thread that
                                  has the highest priority and time to run. 
                                  2-Level. 
              rme_ptr_t Is_Yield - This is a flag to indicate whether this
                                   is a full yield. If it is, the kernel will
                                   discard all the time alloted on this
                                   thread. This only works for threads that
                                   have a finite budget.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Thd_Swt(struct RME_Cap_Cpt* Cpt,
                       struct RME_Reg_Struct* Reg,
                       rme_cid_t Cap_Thd,
                       rme_ptr_t Is_Yield)
{
    struct RME_Cap_Thd* Thd_Cap_New;
    struct RME_Thd_Struct* Thd_New;
    struct RME_Thd_Struct* Thd_High;
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    rme_ptr_t Type_Stat;

    Local=RME_CPU_LOCAL();
    Thd_Cur=Local->Thd_Cur;
    
    /* See if the scheduler is given the right to pick a thread to run */
    if(Cap_Thd<RME_CID_NULL)
    {
        RME_COVERAGE_MARKER();
        
        RME_CPT_GETCAP(Cpt,Cap_Thd,RME_CAP_TYPE_THD,
                       struct RME_Cap_Thd*,Thd_Cap_New,Type_Stat);
        /* Check if the target cap is not frozen and allows such operations */
        RME_CAP_CHECK(Thd_Cap_New,RME_THD_FLAG_SWT);
        /* See if we can do operation on this core */
        Thd_New=RME_CAP_GETOBJ(Thd_Cap_New,struct RME_Thd_Struct*);
        if(Thd_New->Sched.Local!=Local)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_INVSTATE;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
            
        /* See if we can yield to the thread */
        if(Thd_Cur->Sched.Prio!=Thd_New->Sched.Prio)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_PRIO;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
            
        /* See if the state will allow us to do this */
        if((Thd_New->Sched.State==RME_THD_BLOCKED)||
           (Thd_New->Sched.State==RME_THD_TIMEOUT))
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_INVSTATE;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
            
        /* See if the target is in a fault state */
        if(Thd_New->Sched.State==RME_THD_EXC_PEND)
        {
            RME_COVERAGE_MARKER();

            return RME_ERR_PTH_EXC;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* See if we need to give up all our timeslices in this yield */
        if((Is_Yield!=0U)&&(Thd_Cur->Sched.Slice<RME_THD_INF_TIME))
        {
            RME_COVERAGE_MARKER();
            
            _RME_Run_Del(Thd_Cur);
            Thd_Cur->Sched.Slice=0U;
            Thd_Cur->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this. This function includes kernel send
             * as well, but we don't need to call _RME_Kern_High after this 
             * because we are using optimized logic for this context switch.
             * For all other cases, _RME_Kern_High must be called. */
            _RME_Run_Notif(Thd_Cur);
            /* Because we have sent a notification, we could have poked a thread
             * at higher priority. Additionally, if the new thread is the current
             * thread, we are forced to switch to someone else, because our
             * timeslice have certainly exhausted. */
            Thd_High=_RME_Run_High(Local);
            if((Thd_High->Sched.Prio>Thd_New->Sched.Prio)||(Thd_Cur==Thd_New))
            {
                RME_COVERAGE_MARKER();

                Thd_New=Thd_High;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
        }
        else
        {
            RME_COVERAGE_MARKER();
            
            Thd_Cur->Sched.State=RME_THD_READY;
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
        
        /* See if we need to give up all our timeslices in this yield */
        if((Is_Yield!=0U)&&(Thd_Cur->Sched.Slice<RME_THD_INF_TIME))
        {
            RME_COVERAGE_MARKER();
            
            _RME_Run_Del(Thd_Cur);
            Thd_Cur->Sched.Slice=0U;
            Thd_Cur->Sched.State=RME_THD_TIMEOUT;
            /* Notify the parent about this */
            _RME_Run_Notif(Thd_Cur);
        }
        else
        {
            RME_COVERAGE_MARKER();
            
            /* This operation is just to make sure that if there are any other
             * thread at the same priority level, we're not switching to ourself */
            _RME_Run_Del(Thd_Cur);
            _RME_Run_Ins(Thd_Cur);
            Thd_Cur->Sched.State=RME_THD_READY;
        }
        
        Thd_New=_RME_Run_High(Local);
    }
    
    /* Now that we are successful, save the return value to the caller stack */
    __RME_Svc_Retval_Set(Reg,0);

    /* Set the new thread's state first */
    Thd_New->Sched.State=RME_THD_RUNNING;
    /* Here we do not need to call _RME_Kern_High because we have picked the
     * highest priority thread according to the logic above. We just check if
     * it happens to be ourself so we can return from the fast path. */
    if(Thd_Cur==Thd_New)
    {
        RME_COVERAGE_MARKER();
        
        return 0;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
            
    /* We have a solid context switch */
    _RME_Run_Swt(Reg,Thd_Cur,Thd_New);
    Local->Thd_Cur=Thd_New;

    return 0;
}
/* End Function:_RME_Thd_Swt *************************************************/

/* Function:_RME_Sig_Boot_Crt *************************************************
Description : Create a boot-time kernel signal endpoint. This is only used at
              boot-time to create endpoints that are related directly to 
              hardware interrupts.
              This function does not require a kernel memory capability.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this signal.
                                  2-Level.
              rme_cid_t Cap_Sig - The capability slot that you want this newly
                                  created signal capability to be in.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Sig_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Sig)
{
    struct RME_Cap_Cpt* Cpt_Crt;
    struct RME_Cap_Sig* Sig_Crt;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Crt,Type_Stat);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Crt,RME_CPT_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Crt,Cap_Sig,struct RME_Cap_Sig*,Sig_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Sig_Crt);

    /* Header init */
    Sig_Crt->Head.Root_Ref=1U;
    Sig_Crt->Head.Object=0U;
    Sig_Crt->Head.Flag=RME_SIG_FLAG_ALL;
    
    /* Info init */
    Sig_Crt->Sig_Num=0U;
    Sig_Crt->Thd=RME_NULL;

    /* Establish cap */
    RME_WRITE_RELEASE(&(Sig_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_SIG,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Sig_Boot_Crt ********************************************/

/* Function:_RME_Sig_Crt ******************************************************
Description : Create a signal endpoint.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this signal.
                                  2-Level.
              rme_cid_t Cap_Sig - The capability slot that you want this newly
                                  created signal capability to be in.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Sig_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Sig)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Sig* Sig_Crt;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Sig,struct RME_Cap_Sig*,Sig_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Sig_Crt);

    /* Header init */
    Sig_Crt->Head.Root_Ref=0U;
    Sig_Crt->Head.Object=0U;
    Sig_Crt->Head.Flag=RME_SIG_FLAG_ALL;
    
    /* Info init */
    Sig_Crt->Sig_Num=0U;
    Sig_Crt->Thd=0U;
    
    /* Establish cap */
    RME_WRITE_RELEASE(&(Sig_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_SIG,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Sig_Crt *************************************************/

/* Function:_RME_Sig_Del ******************************************************
Description : Delete a signal endpoint.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table to
                                  delete from.
                                  2-Level.
              rme_cid_t Cap_Sig - The capability to the signal.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Sig_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Sig)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Sig* Sig_Del;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Sig,struct RME_Cap_Sig*,Sig_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Sig_Del,Type_Stat,RME_CAP_TYPE_SIG);

    /* See if the signal endpoint is currently used. If yes, we cannot delete it */
    if(Sig_Del->Thd!=0U)
    {
        RME_COVERAGE_MARKER();

        RME_CAP_DEFROST(Sig_Del,Type_Stat);
        return RME_ERR_SIV_ACT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Sig_Del,Type_Stat);
    
    return 0;
}
/* End Function:_RME_Sig_Del *************************************************/

/* Function:_RME_Kern_High ****************************************************
Description : Pick the thread with the highest priority to run. Always call
              this after you finish all your kernel sending stuff in the
              interrupt handler, or the kernel send will not be correct.
Input       : struct RME_Reg_Struct* Reg - The register set.
              struct RME_CPU_Local* Local - The CPU-local data structure.
Output      : volatile struct RME_Reg_Struct* Reg - The updated register set.
Return      : None.
******************************************************************************/
void _RME_Kern_High(struct RME_Reg_Struct* Reg,
                    struct RME_CPU_Local* Local)
{
    struct RME_Thd_Struct* Thd_New;
    struct RME_Thd_Struct* Thd_Cur;

    Thd_New=_RME_Run_High(Local);
    RME_ASSERT(Thd_New!=RME_NULL);
    Thd_Cur=Local->Thd_Cur;

    /* Are these two threads the same? */
    if(Thd_New==Thd_Cur)
    {
        RME_COVERAGE_MARKER();

        return;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Is the current thread running or ready? */
    if((Thd_Cur->Sched.State==RME_THD_RUNNING)||
       (Thd_Cur->Sched.State==RME_THD_READY))
    {
        RME_COVERAGE_MARKER();

        /* Yes, compare the priority to see if we need to do it */
        if(Thd_New->Sched.Prio<=Thd_Cur->Sched.Prio)
        {
            RME_COVERAGE_MARKER();

            return;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* We will have a solid context switch on this point */
    if(Thd_Cur->Sched.State==RME_THD_RUNNING)
    {
        RME_COVERAGE_MARKER();

        Thd_Cur->Sched.State=RME_THD_READY;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    _RME_Run_Swt(Reg,Thd_Cur,Thd_New);
    Thd_New->Sched.State=RME_THD_RUNNING;
    Local->Thd_Cur=Thd_New;
}
/* End Function:_RME_Kern_High ***********************************************/

/* Function:_RME_Kern_Snd *****************************************************
Description : Try to send a signal to an endpoint from kernel. This is intended
              to be called in the interrupt routines in the kernel, and this is
              not a system call. The capability passed in must be the root
              capability, and this function will not check whether it really is.
Input       : struct RME_Cap_Sig* Cap_Sig - The signal root capability.
Output      : None.
Return      : rme_ret_t - If successful, 0, or an error code.
******************************************************************************/
rme_ret_t _RME_Kern_Snd(struct RME_Cap_Sig* Cap_Sig)
{
    rme_ptr_t Unblock;
    struct RME_Thd_Struct* Thd_Sig;
    
    Thd_Sig=Cap_Sig->Thd;
    
    /* If and only if we are calling from the same core as the blocked thread do
     * we actually unblock. Use an intermediate variable Unblock to avoid optimizations */
    if(Thd_Sig!=RME_NULL)
    {
        RME_COVERAGE_MARKER();

        if(Thd_Sig->Sched.Local==RME_CPU_LOCAL())
        {
            RME_COVERAGE_MARKER();

            Unblock=1U;
        }
        else
        {
            RME_COVERAGE_MARKER();

            Unblock=0U;
        }
    }
    else
    {
        RME_COVERAGE_MARKER();

        Unblock=0U;
    }

    if(Unblock!=0U)
    {
        RME_COVERAGE_MARKER();

        /* The thread is blocked, and it is on our core. Unblock it, and
         * set the return value to one as always, Even if we were specifying
         * multi-receive. This is because other cores may reduce the count
         * to zero while we are doing this */
        __RME_Svc_Retval_Set(&(Thd_Sig->Ctx.Reg->Reg),1);
        /* See if the thread still have time left */
        if(Thd_Sig->Sched.Slice!=0U)
        {
            RME_COVERAGE_MARKER();

            /* Put this into the runqueue and just set it to ready. We will not
             * switch to it immediately; this is because we may send to a myriad
             * of endpoints in one interrupt, and we hope to perform the context
             * switch only once when exiting that handler. We can save many 
             * register push/pops! */
            _RME_Run_Ins(Thd_Sig);
            Thd_Sig->Sched.State=RME_THD_READY;
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* No slices left. The only possible reason is because we delegated
             * all of its time to someone else. We will not notify its parent again
             * here because we will have notified it when we transferred all the
             * timeslices away. We just silently change the state of this thread
             * to TIMEOUT. Same for the next function. */
            Thd_Sig->Sched.State=RME_THD_TIMEOUT;
        }
        
        /* Clear the blocking status of the endpoint up - we don't need a write
         * release barrier here because even if this is reordered to happen
         * earlier it is still fine. */
        Cap_Sig->Thd=RME_NULL;
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* The guy who blocked on it is not on our core, or nobody blocked.
         * We just faa the counter value and return. */
        if(RME_FETCH_ADD(&(Cap_Sig->Sig_Num),1U)>=RME_MAX_SIG_NUM)
        {
            RME_COVERAGE_MARKER();

            RME_FETCH_ADD(&(Cap_Sig->Sig_Num),-1);
            return RME_ERR_SIV_FULL;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
    }

    return 0;
}
/* End Function:_RME_Kern_Snd ************************************************/

/* Function:_RME_Sig_Snd ******************************************************
Description : Try to send to a signal endpoint. This system call can cause
              a potential context switch.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Sig - The capability to the signal.
                                  2-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0, or an error code.
******************************************************************************/
rme_ret_t _RME_Sig_Snd(struct RME_Cap_Cpt* Cpt, 
                       struct RME_Reg_Struct* Reg,
                       rme_cid_t Cap_Sig)
{
    struct RME_Cap_Sig* Sig_Op;
    struct RME_Cap_Sig* Sig_Root;
    struct RME_Thd_Struct* Thd_Rcv;
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    rme_ptr_t Unblock;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Sig,RME_CAP_TYPE_SIG,
                   struct RME_Cap_Sig*,Sig_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_SND);
    
    Local=RME_CPU_LOCAL();
    Thd_Cur=Local->Thd_Cur;
    Sig_Root=RME_CAP_CONV_ROOT(Sig_Op,struct RME_Cap_Sig*);
    Thd_Rcv=Sig_Root->Thd;

    /* If and only if we are calling from the same core as the blocked thread do
     * we actually unblock. Use an intermediate variable Unblock to avoid optimizations */
    if(Thd_Rcv!=RME_NULL)
    {
        RME_COVERAGE_MARKER();

        if(Thd_Rcv->Sched.Local==Local)
        {
            RME_COVERAGE_MARKER();

            Unblock=1U;
        }
        else
        {
            RME_COVERAGE_MARKER();

            Unblock=0U;
        }
    }
    else
    {
        RME_COVERAGE_MARKER();

        Unblock=0U;
    }
    
    if(Unblock!=0U)
    {
        RME_COVERAGE_MARKER();

        /* Now save the system call return value to the caller stack */
        __RME_Svc_Retval_Set(Reg,0);
        /* The thread is blocked, and it is on our core. Unblock it, and
         * set the return value to one as always, Even if we were specifying
         * multi-receive. This is because other cores may reduce the count
         * to zero while we are doing this */
        __RME_Svc_Retval_Set(&(Thd_Rcv->Ctx.Reg->Reg),1);
        /* See if the thread still have time left */
        if(Thd_Rcv->Sched.Slice!=0U)
        {
            RME_COVERAGE_MARKER();

            /* Put the waiting one into the runqueue */
            _RME_Run_Ins(Thd_Rcv);
            /* See if it will preempt us */
            if(Thd_Rcv->Sched.Prio>Thd_Cur->Sched.Prio)
            {
                RME_COVERAGE_MARKER();

                /* Yes. Do a context switch */
                _RME_Run_Swt(Reg,Thd_Cur,Thd_Rcv);
                Thd_Cur->Sched.State=RME_THD_READY;
                Thd_Rcv->Sched.State=RME_THD_RUNNING;
                Local->Thd_Cur=Thd_Rcv;
            }
            else
            {
                RME_COVERAGE_MARKER();

                Thd_Rcv->Sched.State=RME_THD_READY;
            }
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* Silently change state to timeout */
            Thd_Rcv->Sched.State=RME_THD_TIMEOUT;
        }
        
        /* Clear the blocking status of the endpoint up - we don't need a write release barrier
         * here because even if this is reordered to happen earlier it is still fine. */
        Sig_Root->Thd=RME_NULL;
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* The guy who blocked on it is not on our core, we just faa and return */
        if(RME_FETCH_ADD(&(Sig_Root->Sig_Num),1U)>=RME_MAX_SIG_NUM)
        {
            RME_COVERAGE_MARKER();

            RME_FETCH_ADD(&(Sig_Root->Sig_Num),-1);
            return RME_ERR_SIV_FULL;
        }
        else
        {
            RME_COVERAGE_MARKER();
        }
        
        /* Now save the system call return value to the caller stack */
        __RME_Svc_Retval_Set(Reg,0);
    }

    return 0;
}
/* End Function:_RME_Sig_Snd *************************************************/

/* Function:_RME_Sig_Rcv ******************************************************
Description : Try to receive from a signal endpoint. The rules for signal
              endpoint receive is:
              1.If a receive endpoint have many send endpoints, everyone can
                send to it, and sending to it will increase the count by 1.
              2.If some thread blocks on a receive endpoint, the wakeup is only
                possible from the same core that thread is on.
              3.It is not recommended to let 2 cores operate on the rcv endpoint
                simutaneously.
              This system call can potentially trigger a context switch.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              volatile struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Sig - The capability to the signal.
                                  2-Level.
              rme_ptr_t Option - The receive option.
Output      : None.
Return      : rme_ret_t - If successful, a non-negative number containing the 
                          number of signals received; or an error code.
******************************************************************************/
rme_ret_t _RME_Sig_Rcv(struct RME_Cap_Cpt* Cpt,
                       struct RME_Reg_Struct* Reg,
                       rme_cid_t Cap_Sig,
                       rme_ptr_t Option)
{
    struct RME_Cap_Sig* Sig_Op;
    struct RME_Cap_Sig* Sig_Root;
    struct RME_CPU_Local* Local;
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Thd_Struct* Thd_New;
    rme_ptr_t Old_Value;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Sig,RME_CAP_TYPE_SIG,
                   struct RME_Cap_Sig*,Sig_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    switch(Option)
    {
        case RME_RCV_BS:
        {
            RME_COVERAGE_MARKER();

            RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_RCV_BS);
            break;
        }
        case RME_RCV_BM:
        {
            RME_COVERAGE_MARKER();
            
            RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_RCV_BM);
            break;
        }
        case RME_RCV_NS:
        {
            RME_COVERAGE_MARKER();
            
            RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_RCV_NS);
            break;
        }
        case RME_RCV_NM:
        {
            RME_COVERAGE_MARKER();
            
            RME_CAP_CHECK(Sig_Op,RME_SIG_FLAG_RCV_NM);
            break;
        }
        default:
        {
            RME_COVERAGE_MARKER();
            
            return RME_ERR_SIV_ACT;
        }
    }
    
    /* Convert to root cap */
    Sig_Root=RME_CAP_CONV_ROOT(Sig_Op,struct RME_Cap_Sig*);
    
    /* See if we can receive on that endpoint - if someone blocks, we must
     * wait for it to unblock before we can proceed */
    if(Sig_Root->Thd!=RME_NULL)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_ACT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    Local=RME_CPU_LOCAL();
    Thd_Cur=Local->Thd_Cur;
    
    /* Are we trying to let a boot-time thread block on a signal? This is NOT allowed.
     * Additionally, if the current thread have no timeslice left (which shouldn't happen
     * under whatever circumstances), we assert and die */
    RME_ASSERT(Thd_Cur->Sched.Slice!=0U);
    if(Thd_Cur->Sched.Slice==RME_THD_INIT_TIME)
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_BOOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Are there any counts available? If yes, just take one and return. We cannot
     * use faa here because we don't know if we will get it below zero */
    Old_Value=Sig_Root->Sig_Num;
    if(Old_Value>0U)
    {
        RME_COVERAGE_MARKER();

        /* We can't use fetch-and-add because we don't know if other cores will reduce count to zero */
        if((Option==RME_RCV_BS)||(Option==RME_RCV_NS))
        {
            RME_COVERAGE_MARKER();

            /* Try to take one */
            if(RME_COMP_SWAP(&(Sig_Root->Sig_Num),
                             Old_Value,
                             Old_Value-1U)==RME_CASFAIL)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_SIV_CONFLICT;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
            
            /* We have taken it, now return what we have taken */
            __RME_Svc_Retval_Set(Reg,1);
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* Try to take all */
            if(RME_COMP_SWAP(&(Sig_Root->Sig_Num),
                             Old_Value,
                             0U)==RME_CASFAIL)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_SIV_CONFLICT;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }
            
            /* We have taken all, now return what we have taken */
            __RME_Svc_Retval_Set(Reg,(rme_ret_t)Old_Value);
        }
        
        return 0;
    }
    else
    {
        RME_COVERAGE_MARKER();

        /* There's no value, try to block */
        if((Option==RME_RCV_BS)||(Option==RME_RCV_BM))
        {
            RME_COVERAGE_MARKER();

            if(RME_COMP_SWAP((rme_ptr_t*)&(Sig_Root->Thd),
                             RME_NULL,
                             (rme_ptr_t)Thd_Cur)==RME_CASFAIL)
            {
                RME_COVERAGE_MARKER();

                return RME_ERR_SIV_CONFLICT;
            }
            else
            {
                RME_COVERAGE_MARKER();
            }

            /* Now we block our current thread. No need to set any return value
             * to the register set here, because we do not yet know how many
             * signals will be there when the thread unblocks. The unblocking
             * does not need an option so we don't keep that; we always treat
             * it as single receive when we unblock anyway. */
            Thd_Cur->Sched.State=RME_THD_BLOCKED;
            Thd_Cur->Sched.Signal=Sig_Root;
            _RME_Run_Del(Thd_Cur);
            Thd_New=_RME_Run_High(Local);
            _RME_Run_Swt(Reg, Thd_Cur, Thd_New);
            Thd_New->Sched.State=RME_THD_RUNNING;
            Local->Thd_Cur=Thd_New;
        }
        else
        {
            RME_COVERAGE_MARKER();

            /* We have taken nothing but the system call is successful anyway */
            __RME_Svc_Retval_Set(Reg,0);
        }
    }
    
    return 0;
}
/* End Function:_RME_Sig_Rcv *************************************************/

/* Function:_RME_Inv_Crt ******************************************************
Description : Create an invocation stub.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table to use
                                  for this process.
                                  2-Level.
              rme_cid_t Cap_Inv - The capability slot that you want this newly
                                  created invocation capability to be in.
                                  1-Level.
              rme_cid_t Cap_Prc - The capability to the process that it is in.
                                  2-Level.
              rme_ptr_t Raddr - The relative virtual address to store the
                                invocation port kernel object.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Inv_Crt(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Kom,
                       rme_cid_t Cap_Inv,
                       rme_cid_t Cap_Prc,
                       rme_ptr_t Raddr)
{
    struct RME_Cap_Cpt* Cpt_Op;
    struct RME_Cap_Prc* Prc_Op;
    struct RME_Cap_Kom* Kom_Op;
    volatile struct RME_Cap_Inv* Inv_Crt;
    struct RME_Cap_Prc* Prc_Root;
    struct RME_Inv_Struct* Invocation;
    rme_ptr_t Type_Stat;
    rme_ptr_t Vaddr;
    
    /* Get the capability slots */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Prc,RME_CAP_TYPE_PRC,
                   struct RME_Cap_Prc*,Prc_Op,Type_Stat);
    RME_CPT_GETCAP(Cpt,Cap_Kom,RME_CAP_TYPE_KOM,
                   struct RME_Cap_Kom*,Kom_Op,Type_Stat);
    /* Check if the captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    RME_CAP_CHECK(Prc_Op,RME_PRC_FLAG_INV);
    /* See if the creation is valid for this kmem range */
    RME_KOM_CHECK(Kom_Op,RME_KOM_FLAG_INV,Raddr,Vaddr,RME_INV_SIZE);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Inv,struct RME_Cap_Inv*,Inv_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Inv_Crt);
    
    /* Try to populate the area */
    if(_RME_Kot_Mark(Vaddr,RME_INV_SIZE)!=0)
    {
        RME_COVERAGE_MARKER();

        RME_WRITE_RELEASE(&(Inv_Crt->Head.Type_Stat),0U);
        return RME_ERR_CPT_KOT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Object init */
    Invocation=(struct RME_Inv_Struct*)Vaddr;
    Prc_Root=RME_CAP_CONV_ROOT(Prc_Op,struct RME_Cap_Prc*);
    Invocation->Prc=Prc_Root;
    Invocation->Thd_Act=RME_NULL;
    /* By default we do not return on exception */
    Invocation->Is_Exc_Ret=0U;
    
    /* Header init */
    Inv_Crt->Head.Root_Ref=0U;
    Inv_Crt->Head.Object=Vaddr;
    Inv_Crt->Head.Flag=RME_INV_FLAG_ALL;
    
    /* Reference object */
    RME_FETCH_ADD(&(Prc_Root->Head.Root_Ref),1U);
    
    /* Establish cap */
    RME_WRITE_RELEASE(&(Inv_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_INV,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Inv_Crt *************************************************/

/* Function:_RME_Inv_Del ******************************************************
Description : Delete an invocation stub.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the capability table to
                                  delete from.
                                  2-Level.
              rme_cid_t Cap_Inv - The capability to the invocation stub.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Inv_Del(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Cpt,
                       rme_cid_t Cap_Inv)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Inv* Inv_Del;
    rme_ptr_t Type_Stat;
    /* These are for deletion */
    struct RME_Inv_Struct* Invocation;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);    
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_DEL);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Inv,struct RME_Cap_Inv*,Inv_Del);
    /* Delete check */
    RME_CAP_DEL_CHECK(Inv_Del,Type_Stat,RME_CAP_TYPE_INV);
    
    /* Get the invocation */
    Invocation=RME_CAP_GETOBJ(Inv_Del,struct RME_Inv_Struct*);
    
    /* See if the invocation is currently being used. If yes, we cannot delete it */
    if(Invocation->Thd_Act!=RME_NULL)
    {
        RME_COVERAGE_MARKER();

        RME_CAP_DEFROST(Inv_Del,Type_Stat);
        return RME_ERR_SIV_ACT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }
    
    /* Now we can safely delete the cap */
    RME_CAP_DELETE(Inv_Del,Type_Stat);
    
    /* Dereference the process */
    RME_FETCH_ADD(&(Invocation->Prc->Head.Root_Ref), -1);
    
    /* Try to clear the area - this must be successful */
    RME_ASSERT(_RME_Kot_Erase((rme_ptr_t)Invocation,RME_INV_SIZE)==0);
    
    return 0;
}
/* End Function:_RME_Inv_Del *************************************************/

/* Function:_RME_Inv_Set ******************************************************
Description : Set an invocation stub's entry point and stack. The registers will
              be initialized with these contents.
Input       : struct RME_Cap_Cpt* Cpt - The capability table.
              rme_cid_t Cap_Inv - The capability to the invocation stub.
                                  2-Level.
              rme_ptr_t Entry - The entry of the thread.
              rme_ptr_t Stack - The stack address to use for execution.
              rme_ptr_t Is_Exc_Ret - If there is an exception in this
                                     invocation, return immediately, or wait
                                     for fault handling?
                                     If 1, we return directly on fault.
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Inv_Set(struct RME_Cap_Cpt* Cpt,
                       rme_cid_t Cap_Inv,
                       rme_ptr_t Entry,
                       rme_ptr_t Stack,
                       rme_ptr_t Is_Exc_Ret)
{
    struct RME_Cap_Inv* Inv_Op;
    volatile struct RME_Inv_Struct* Invocation;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Inv,RME_CAP_TYPE_INV,
                   struct RME_Cap_Inv*,Inv_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Inv_Op,RME_INV_FLAG_SET);
    
    /* Commit the change - we do not care if the invocation is in use */
    Invocation=RME_CAP_GETOBJ(Inv_Op,struct RME_Inv_Struct*);
    Invocation->Entry=Entry;
    Invocation->Stack=Stack;
    Invocation->Is_Exc_Ret=Is_Exc_Ret;
    
    return 0;
}
/* End Function:_RME_Inv_Set *************************************************/

/* Function:_RME_Inv_Act ******************************************************
Description : Call the invocation stub. One parameter is guaranteed; however, 
              some platforms may provide more than that.
Input       : struct RME_Cap_Cpt* Cpt - The capability table.
              struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Inv - The invocation stub.
                                  2-Level.
              rme_ptr_t Param - The parameter for the call.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Inv_Act(struct RME_Cap_Cpt* Cpt, 
                       struct RME_Reg_Struct* Reg,
                       rme_cid_t Cap_Inv,
                       rme_ptr_t Param)
{
    struct RME_Cap_Inv* Inv_Op;
    struct RME_Inv_Struct* Invocation;
    struct RME_Thd_Struct* Thd_Cur;
    struct RME_Thd_Struct* Thd_Act;
    rme_ptr_t Type_Stat;

    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Inv,RME_CAP_TYPE_INV,
                   struct RME_Cap_Inv*,Inv_Op,Type_Stat);
    /* Check if the target cap is not frozen and allows such operations */
    RME_CAP_CHECK(Inv_Op,RME_INV_FLAG_ACT);

    /* Get the invocation struct */
    Invocation=RME_CAP_GETOBJ(Inv_Op,struct RME_Inv_Struct*);
    /* See if we are currently active - If yes, we can't activate it again */
    Thd_Act=Invocation->Thd_Act;
    if(RME_UNLIKELY(Thd_Act!=0U))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_ACT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Push this invocation stub capability into the current thread's invocation stack */
    Thd_Cur=RME_CPU_LOCAL()->Thd_Cur;
    /* Try to do CAS and activate it */
    if(RME_UNLIKELY(RME_COMP_SWAP((volatile rme_ptr_t*)&(Invocation->Thd_Act),
                                  (rme_ptr_t)Thd_Act,
                                  (rme_ptr_t)Thd_Cur)==RME_CASFAIL))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_ACT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Save whatever is needed to return to the point - normally only SP and IP needed
     * because all other registers, including the coprocessor registers, are saved at
     * user-level. We do not set the return value because it will be set by Inv_Ret.
     * The coprocessor state will be consistent across the call */
    __RME_Inv_Reg_Save(&(Invocation->Ret),Reg);
    /* Push this into the stack: insert after the thread list header */
    _RME_List_Ins(&(Invocation->Head),
                  &(Thd_Cur->Ctx.Invstk),
                  Thd_Cur->Ctx.Invstk.Next);
    /* Setup the register contents, and do the invocation */
    __RME_Thd_Reg_Init(RME_THD_ATTR(Thd_Cur->Ctx.Hyp_Attr),
                       Invocation->Entry,
                       Invocation->Stack,
                       Param,Reg);
    
    /* We are assuming that we are always invoking into a new process (why use synchronous
     * invocation if you don't do so?). So we always switch page tables regardless. */
#if(RME_PGT_RAW_USER==0U)
    RME_ASSERT(RME_CAP_IS_ROOT(Invocation->Prc->Pgt)!=0U);
#endif
    __RME_Pgt_Set(Invocation->Prc->Pgt);
    
    return 0;
}
/* End Function:_RME_Inv_Act *************************************************/

/* Function:_RME_Inv_Ret ******************************************************
Description : Return from the invocation function, and set the return value to
              the old register set. This function does not need a capability
              table to work.
Input       : struct RME_Reg_Struct* Reg - The register set.
              rme_ptr_t Retval - The return value of this synchronous invocation.
              rme_ptr_t Is_Exc - Are we attempting a return from exception?
Output      : None.
Return      : rme_ret_t - If successful, 0; or an error code.
******************************************************************************/
rme_ret_t _RME_Inv_Ret(struct RME_Reg_Struct* Reg,
                       rme_ptr_t Retval,
                       rme_ptr_t Is_Exc)
{
    struct RME_Thd_Struct* Thread;
    struct RME_Inv_Struct* Invocation;

    /* See if we can return; If we can, get the structure */
    Thread=RME_CPU_LOCAL()->Thd_Cur;
    Invocation=RME_INVSTK_TOP(Thread);
    if(RME_UNLIKELY(Invocation==RME_NULL))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_EMPTY;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Is this return forced by a fault? If yes, check if we allow that */
    if(RME_UNLIKELY((Is_Exc!=0U)&&(Invocation->Is_Exc_Ret==0U)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_SIV_FAULT;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Pop it from the stack */
    _RME_List_Del(Invocation->Head.Prev,Invocation->Head.Next);

    /* Restore the register contents, and set return value. We need to set
     * the return value of the invocation system call itself as well */
    __RME_Inv_Reg_Restore(Reg,&(Invocation->Ret));
    __RME_Inv_Retval_Set(Reg,(rme_ret_t)Retval);

    /* We have successfully returned, set the invocation as inactive. We need
     * a barrier here to avoid potential destruction of the return value. */
    RME_WRITE_RELEASE(&(Invocation->Thd_Act),0U);

    /* Decide the system call's return value */
    if(RME_UNLIKELY(Is_Exc!=0U))
    {
        RME_COVERAGE_MARKER();

        __RME_Svc_Retval_Set(Reg, RME_ERR_SIV_FAULT);
    }
    else
    {
        RME_COVERAGE_MARKER();

        __RME_Svc_Retval_Set(Reg,0);
    }

    /* Same assumptions as in invocation activation */
    Invocation=RME_INVSTK_TOP(Thread);
    if(Invocation!=RME_NULL)
    {
        RME_COVERAGE_MARKER();
        
#if(RME_PGT_RAW_USER==0U)
        RME_ASSERT(RME_CAP_IS_ROOT(Invocation->Prc->Pgt)!=0U);
#endif
        __RME_Pgt_Set(Invocation->Prc->Pgt);
    }
    else
    {
        RME_COVERAGE_MARKER();
        
#if(RME_PGT_RAW_USER==0U)
        RME_ASSERT(RME_CAP_IS_ROOT(Thread->Sched.Prc->Pgt)!=0U);
#endif
        __RME_Pgt_Set(Thread->Sched.Prc->Pgt);
    }
    
    return 0;
}
/* End Function:_RME_Inv_Ret *************************************************/

/* Function:_RME_Kfn_Boot_Crt *************************************************
Description : This function is used to create boot-time kernel call capability.
              This kind of capability that does not have a kernel object.
              Kernel function capabilities allow you to execute user-defined 
              functions in kernel mode. These functions must be defined in the
              platform extensions.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              rme_cid_t Cap_Cpt - The capability to the captbl that may contain
                                  the cap to kernel function.
                                  2-Level.
              rme_cid_t Cap_Kfn - The capability to the kernel function.
                                  1-Level.
Output      : None.
Return      : rme_ret_t - If successful, 0; or error code.
******************************************************************************/
rme_ret_t _RME_Kfn_Boot_Crt(struct RME_Cap_Cpt* Cpt,
                            rme_cid_t Cap_Cpt,
                            rme_cid_t Cap_Kfn)
{
    struct RME_Cap_Cpt* Cpt_Op;
    volatile struct RME_Cap_Kfn* Kfn_Crt;
    rme_ptr_t Type_Stat;
    
    /* Get the cap location that we care about */
    RME_CPT_GETCAP(Cpt,Cap_Cpt,RME_CAP_TYPE_CPT,
                   struct RME_Cap_Cpt*,Cpt_Op,Type_Stat);
    /* Check if the target captbl is not frozen and allows such operations */
    RME_CAP_CHECK(Cpt_Op,RME_CPT_FLAG_CRT);
    
    /* Get the cap slot */
    RME_CPT_GETSLOT(Cpt_Op,Cap_Kfn,struct RME_Cap_Kfn*,Kfn_Crt);
    /* Take the slot if possible */
    RME_CPT_OCCUPY(Kfn_Crt);
    
    /* Header init */
    Kfn_Crt->Head.Root_Ref=1U;
    Kfn_Crt->Head.Object=0U;
    Kfn_Crt->Head.Flag=RME_KFN_FLAG_FULL_RANGE;
    
    /* Establish cap */
    RME_WRITE_RELEASE(&(Kfn_Crt->Head.Type_Stat),
                      RME_CAP_TYPE_STAT(RME_CAP_TYPE_KFN,
                                        RME_CAP_STAT_VALID,
                                        RME_CAP_ATTR_ROOT));

    return 0;
}
/* End Function:_RME_Kfn_Boot_Crt ********************************************/

/* Function:_RME_Kfn_Act ******************************************************
Description : Activate a kernel function.
Input       : struct RME_Cap_Cpt* Cpt - The master capability table.
              struct RME_Reg_Struct* Reg - The register set.
              rme_cid_t Cap_Kfn - The capability to the kernel capability.
                                  2-Level.
              rme_ptr_t Func_ID - The function ID to invoke.
              rme_ptr_t Sub_ID - The subfunction ID to invoke.
              rme_ptr_t Param1 - The first parameter.
              rme_ptr_t Param2 - The second parameter.
Output      : None.
Return      : rme_ret_t - If the call is successful, it will return whatever
                          the 
                          function returned (It is expected that they shall
                          never return an negative value); or an error code.
                          If the kernel function ever causes a context switch,
                          it is responsible for setting the return value. On 
                          failure, no context switch is allowed.
******************************************************************************/
rme_ret_t _RME_Kfn_Act(struct RME_Cap_Cpt* Cpt,
                       struct RME_Reg_Struct* Reg,
                       rme_cid_t Cap_Kfn,
                       rme_ptr_t Func_ID,
                       rme_ptr_t Sub_ID,
                       rme_ptr_t Param1,
                       rme_ptr_t Param2)
{
    struct RME_Cap_Kfn* Kfn_Op;
    rme_ptr_t Type_Stat;
    
    /* Get the capability slot */
    RME_CPT_GETCAP(Cpt,Cap_Kfn,RME_CAP_TYPE_KFN,
                   struct RME_Cap_Kfn*,Kfn_Op,Type_Stat);    

    /* Check if the range of calling is allowed - This is kernel function specific */
    if((Func_ID>RME_KFN_FLAG_HIGH(Kfn_Op->Head.Flag))||
       (Func_ID<RME_KFN_FLAG_LOW(Kfn_Op->Head.Flag)))
    {
        RME_COVERAGE_MARKER();

        return RME_ERR_CPT_FLAG;
    }
    else
    {
        RME_COVERAGE_MARKER();
    }

    /* Return whatever the function returns */
    return __RME_Kfn_Handler(Cpt,Reg,Func_ID,Sub_ID,Param1,Param2);
}
/* End Function:_RME_Kfn_Act *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
