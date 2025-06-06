/******************************************************************************
Filename    : init.c
Author      : pry
Date        : 11/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The init process of MMU-based UVM systems. This process is currently
              just a benchmark, and will only output the performance figures. The kernel
              is not responsible for parsing the .elf files, so the header of this file is
              pretty much fixed.
******************************************************************************/

/* Includes ******************************************************************/
#include "rme.h"

#define __HDR_DEFS__
#include "Platform/UVM_platform.h"
#include "Init/syssvc.h"
#include "Init/init.h"
#undef __HDR_DEFS__

#define __HDR_STRUCTS__
#include "Platform/UVM_platform.h"
#include "Init/syssvc.h"
#include "Init/init.h"
#undef __HDR_STRUCTS__

/* Private include */
#include "Init/init.h"

#define __HDR_PUBLIC_MEMBERS__
#include "Platform/UVM_platform.h"
#include "Init/syssvc.h"
#undef __HDR_PUBLIC_MEMBERS__
/*End Includes **************************************************************/

/* Begin Function:UVM_Clear ***************************************************
Description : Memset a memory area to zero.
Input       : void* Addr - The address to clear.
              ptr_t Size - The size to clear.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_Clear(void* Addr, ptr_t Size)
{
    ptr_t* Word_Inc;
    u8* Byte_Inc;
    ptr_t Words;
    ptr_t Bytes;
    
    /* On processors not that fast, copy by word is really important */
    Word_Inc=(ptr_t*)Addr;
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    {
        *Word_Inc=0;
        Word_Inc++;
    }
    
    /* Get the final bytes */
    Byte_Inc=(u8*)Word_Inc;
    for(Bytes=Size%sizeof(ptr_t);Bytes>0;Bytes--)
    {
        *Byte_Inc=0;
        Byte_Inc++;
    }
}
/* End Function:UVM_Clear ****************************************************/
/*The base address where we begin to place UVM kernel object,It is a relative address*/
#define UVM_OBJ_BASE   0xFFFF800010000000ULL-0xFFFF800001600000ULL;
#define TEST_THD_TBL      9
#define TEST_INV1        10
#define TEST_SIG1        11
#define TEST_SIG2        12
#define TEST_PROC_CAPTBL  13
#define TEST_PROCESS_PGT  14
#define TEST_PROCESS      15

/*The test thread slot*/
#define TEST_THD1        0
#define TEST_THD2        1
#define TEST_THD3        2
#define TEST_THD4        3

/*The test process page table slot*/
#define TEST_PROCESS_PML4   0
#define RME_TEST_PDP(X)      (TEST_PROCESS_PML4+1+(X))
#define RME_TEST_PDE(X)      (RME_TEST_PDP(16)+(X))

volatile ptr_t start;
volatile ptr_t middle;
volatile ptr_t end;
ptr_t sum;
ptr_t sumin;
ptr_t sumout;

/*This function is for tread switching test*/
void TEST_THD_FUNC1(void)
{
    cnt_t Cnt;
    sum=0;
    for(Cnt=0;Cnt<10000;Cnt++)
    {
        start=__UVM_X64_Read_TSC();
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD2),0);
        end=__UVM_X64_Read_TSC();
        sum+=end-start;
    }
    UVM_LOG_S("\r\nThread Switching takes clock cycles:");
    UVM_LOG_I(sum/10000);
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
}

/*This function is for tread switching test*/
void TEST_THD_FUNC2(void)
{
    while(1)
    {
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0);
    }
}

/*This function is for cross-process tread switching test*/
void TEST_THD_FUNC3(void)
{
    cnt_t Cnt;
    sum=0;
    for(Cnt=0;Cnt<10000;Cnt++)
    {
        start=__UVM_X64_Read_TSC();
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD4),0);
        end=__UVM_X64_Read_TSC();
        sum+=end-start;
    }
    UVM_LOG_S("\r\nCross-process thread Switching takes clock cycles:");
    UVM_LOG_I(sum/10000);
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
}

/*This function is for cross-process tread switching test*/
void TEST_THD_FUNC4(void)
{
    while(1)
    {
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0);
    }
}

/*This function is for signal sending-receiving test*/
void TEST_SIG_FUNC1(void)
{
    cnt_t Cnt;
    sum=0;
    for(Cnt=0;Cnt<10000;Cnt++)
    {
        start=__UVM_X64_Read_TSC();
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
        end=__UVM_X64_Read_TSC();
        sum+=end-start;
    }
    UVM_LOG_S("\r\nSignal sending-receiving takes clock cycles:");
    UVM_LOG_I(sum/10000);
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
}

/*This function is for Cross-process signal sending-receiving test*/
void TEST_SIG_FUNC2(void)
{
    cnt_t Cnt;
    sum=0;
    for(Cnt=0;Cnt<10000;Cnt++)
    {
        start=__UVM_X64_Read_TSC();
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
        end=__UVM_X64_Read_TSC();
        sum+=end-start;
    }
    UVM_LOG_S("\r\nCross-process signal sending-receiving takes clock cycles:");
    UVM_LOG_I(sum/10000);
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
}

void TEST_INV1_FUNC(ptr_t param)
{
    middle=__UVM_X64_Read_TSC();
    UVM_Inv_Ret(param);
}

/* Begin Function:main ********************************************************
Description : The entry of the VMM's init thread. 
Input       : None.
Output      : None.
Return      : int - This function shall never return.
******************************************************************************/
int main(ptr_t CPUID)
{
    ptr_t Cur_Addr;
    cnt_t Count;
    cnt_t Count1;
    cnt_t Count2;
    UVM_LOG_S("........Booting RME system........");
    UVM_LOG_S("\r\nEnter user mode success!Welcome to RME system!");
    UVM_LOG_S("\r\nNow we are running init thread on cpu:");
    UVM_LOG_I(CPUID);
    if(CPUID==0) 
    {
        /*Empty test begins here*/
        sum=0;
        for(Count=0;Count<10000;Count++)
        {
            start=__UVM_X64_Read_TSC();
            end=__UVM_X64_Read_TSC();
            sum+=end-start;
        }
        UVM_LOG_S("\r\nEmpty test takes clock cycles:");
        UVM_LOG_I(sum/10000);
        /*Empty test ends here*/

        /*Empty system call test begins here*/
        sum=0;
        for(Count=0;Count<10000;Count++)
        {
            start=__UVM_X64_Read_TSC();
            UVM_Svc(-1,-1,-1,-1);
            end=__UVM_X64_Read_TSC();
            sum+=end-start;
        }
        UVM_LOG_S("\r\nEmpty system call takes clock cycles:");
        UVM_LOG_I(sum/10000);
        /*Empty system call test ends here*/

        /*Now we begin to create UVM kernel objects*/
        Cur_Addr=UVM_OBJ_BASE;
        
        /*Thread switching test begins here, We place the thread stack at 12MB */
        /*Create test thread capability table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD_TBL,Cur_Addr,16)>=0);
        Cur_Addr+=UVM_CAPTBL_SIZE(16);
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD1, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
        UVM_Clear((void*)(12*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_THD_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
        Cur_Addr+=UVM_THD_SIZE;
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_THD2, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
        UVM_Clear((void*)(13*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD2),(ptr_t)TEST_THD_FUNC2,13*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,1)>=0);
        Cur_Addr+=UVM_THD_SIZE;
        UVM_LOG_S("\r\nSwtching thread...");
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0);
        UVM_LOG_S("\r\nBack to main thread!");
        /*Thread switching test ends here*/
        
        /*Signal Sending-receiving test begins here*/
        /*create endpoint first*/
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG1, Cur_Addr)>=0);
        Cur_Addr+=UVM_SIG_SIZE;
        /*reset two threads*/
        UVM_Clear((void*)(12*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_SIG_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
        /*throw away thread1 infinity time slices,and add finity time slices to it*/
        //UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(UVM_BOOT_TBL_THD,0),TEST_THD1,UVM_THD_INIT_TIME)>=0);
        //UVM_ASSERT(UVM_Thd_Time_Xfer(TEST_THD1,UVM_CAPID(UVM_BOOT_TBL_THD,0),100)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),1)>=0);
        for(Count=0;Count<10000;Count++)
        {
            UVM_Sig_Snd(TEST_SIG1,1);
        }
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0)>=0);
        /*Signal Sending-receiving test ends here*/

        /*Cross-process thread switching test begins here*/

        /*Create test process capability table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROC_CAPTBL,Cur_Addr,16)>=0);
        Cur_Addr+=UVM_CAPTBL_SIZE(16);
        /*Create test process page table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PGT,Cur_Addr,1+16+8192)>=0);
        Cur_Addr+=UVM_CAPTBL_SIZE(1+16+8192);
        /*Create test process PML4*/
        Cur_Addr=UVM_ROUND_UP(Cur_Addr,12);
        UVM_ASSERT(UVM_Pgtbl_Crt(TEST_PROCESS_PGT,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PML4,Cur_Addr,0,1U,RME_PGT_SIZE_512G,RME_PGT_NUM_512)>=0);
        Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
        /* Create 16 PDPs*/
        for(Count=0;Count<16;Count++)
        {
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDP(Count),
                                           Cur_Addr, (ptr_t)UVM_POW2(RME_PGT_SIZE_512G)*Count, 0U, RME_PGT_SIZE_1G, RME_PGT_NUM_512)>=0);
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,TEST_PROCESS_PML4),Count,
                                                UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count)),RME_PGT_ALL_PERM)>=0);
        }
        UVM_LOG_S("\r\nCreate PDPs success!");
        /* Create 8192 PDEs*/
        for(Count=0;Count<8192;Count++)
        {
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDE(Count),
                                           Cur_Addr, (ptr_t)UVM_POW2(RME_PGT_SIZE_1G)*Count, 0U,  RME_PGT_SIZE_2M, RME_PGT_NUM_512)>=0);
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count>>9)),Count&0x1FF,
                                       UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDE(Count)),RME_PGT_ALL_PERM)>=0);
        }
        UVM_LOG_S("\r\nCreate PDEs success!");
        /*Add pages to PDEs*/
        for (Count1=0;Count1<4;Count1++)
        {
            for (Count2=0;Count2<512;Count2++)
            {
                UVM_ASSERT(UVM_Pgtbl_Add(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDE(Count1)),Count2,(RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE|RME_PGT_CACHE|RME_PGT_BUFFER),
                                    UVM_CAPID(UVM_BOOT_TBL_PGTBL,RME_TEST_PDE(Count1)),Count2,0)>=0);
            }
        }
        UVM_LOG_S("\r\nAdd pages success!");

        /*Create test process here*/
        UVM_ASSERT(UVM_Proc_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS,UVM_BOOT_CAPTBL,
                                        UVM_CAPID(TEST_PROCESS_PGT,TEST_PROCESS_PML4),Cur_Addr)>=0);
        Cur_Addr+=UVM_PROC_SIZE;
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD3, TEST_PROCESS, 10, Cur_Addr)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
        UVM_Clear((void*)(14*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_THD_FUNC3,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
        Cur_Addr+=UVM_THD_SIZE;

        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD4, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
        UVM_Clear((void*)(15*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD4),(ptr_t)TEST_THD_FUNC4,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
        Cur_Addr+=UVM_THD_SIZE;

        UVM_LOG_S("\r\nCross-process swtching thread...");
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0);
        UVM_LOG_S("\r\nBack to main thread!");

        /*Cross-process thread switching test ends here*/
       
        /*Cross-process signal Sending-receiving test begins here*/
        /*create endpoint first*/
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG2, Cur_Addr)>=0);
        Cur_Addr+=UVM_SIG_SIZE;
        UVM_Clear((void*)(14*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_SIG_FUNC2,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);

        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),1)>=0);
        for(Count=0;Count<10000;Count++)
        {
            UVM_Sig_Snd(TEST_SIG2,1);
        }
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0)>=0);

        /*Cross-process signal Sending-receiving test ends here*/

        /*Invocation stub test begins here*/
      
        UVM_ASSERT(UVM_Inv_Crt(UVM_BOOT_CAPTBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_INV1, TEST_PROCESS, Cur_Addr)>=0);
        Cur_Addr+=UVM_INV_SIZE;
        UVM_Clear((void*)(15*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Inv_Set(TEST_INV1,(ptr_t)TEST_INV1_FUNC,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
        sum=0;
        sumin=0;
        sumout=0;
        for(Count=0;Count<10000;Count++)
        {
            start=__UVM_X64_Read_TSC();
            UVM_Inv_Act(TEST_INV1,0,0);
            end=__UVM_X64_Read_TSC();
            sum+=end-start;
            sumin+=middle-start;
            sumout+=end-middle;
        }
        UVM_LOG_S("\r\nInvocation total takes clock cycles:");
        UVM_LOG_I(sum/10000);
        UVM_LOG_S("\r\nInvocation entry takes clock cycles:");
        UVM_LOG_I(sumin/10000);
        UVM_LOG_S("\r\nInvocation return takes clock cycles:");
        UVM_LOG_I(sumout/10000);
        
        /*Invocation stub test ends here*/

        /*Idle*/
        UVM_LOG_S("\r\nIdle......");
        while (1);
    }
    return 0;
}
/* End Function:main *********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
