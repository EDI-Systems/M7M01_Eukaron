/******************************************************************************
Filename    : siginv.h
Author      : pry
Date        : 14/06/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of signal and invocation management facilities.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __SIGINV_H_DEFS__
#define __SIGINV_H_DEFS__
/*****************************************************************************/
/* The maximum number of signals on an endpoint */
#define RME_MAX_SIG_NUM           (((ptr_t)(-1))>>1)

/* The kernel object sizes */
#define RME_INV_SIZE          sizeof(struct RME_Inv_Struct)
#define RME_SIG_SIZE          sizeof(struct RME_Sig_Struct)
/*****************************************************************************/
/* __SIGINV_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __SIGINV_H_STRUCTS__
#define __SIGINV_H_STRUCTS__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/* The signal structure */
struct RME_Sig_Struct
{
    /* Is this a kernel signal endpoint? */
    ptr_t Kernel_Flag;
    /* The number of signals sent to here */
    ptr_t Signal_Num;
    /* What thread blocked on this one */
    struct RME_Thd_Struct* Thd;
};

/* The signal capability */
struct RME_Cap_Sig
{
    struct RME_Cap_Head Head;
    ptr_t Info[3];
};

struct RME_Inv_Struct
{
    /* This will be inserted into a thread structure */
    struct RME_List Head;
    /* The process pointer */
    struct RME_Proc_Struct* Proc;
    /* Is the invocation currently active? If yes, we cannot delete */
    ptr_t Active;
    /* The register set settings for invocation */
    struct RME_Reg_Struct Reg;
    /* The co-processor/peripheral settings for invocation */
    struct RME_Cop_Struct Cop_Reg;
    /* Below are used to hold the context, if one invocation is interrupted before it exits */
    struct RME_Reg_Struct Inv_Reg;
    struct RME_Cop_Struct Inv_Cop_Reg;
};

/* The synchronous invocation capability */
struct RME_Cap_Inv
{
    struct RME_Cap_Head Head;
    ptr_t Info[3];
};
/*****************************************************************************/

/*****************************************************************************/
/* __SIGINV_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __SIGINV_MEMBERS__
#define __SIGINV_MEMBERS__

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
__EXTERN__ ret_t _RME_Sig_Boot_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                                   cid_t Cap_Sig, ptr_t Vaddr);
__EXTERN__ ret_t _RME_Sig_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                              cid_t Cap_Kmem, cid_t Cap_Sig, ptr_t Vaddr);
__EXTERN__ ret_t _RME_Sig_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Sig);
__EXTERN__ ret_t _RME_Kern_Snd(struct RME_Reg_Struct* Reg, struct RME_Sig_Struct* Sig);
__EXTERN__ ret_t _RME_Sig_Snd(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg, cid_t Cap_Sig);
__EXTERN__ ret_t _RME_Sig_Rcv(struct RME_Cap_Captbl* Captbl, struct RME_Reg_Struct* Reg, cid_t Cap_Sig);

__EXTERN__ ret_t _RME_Inv_Crt(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl,
                              cid_t Cap_Kmem, cid_t Cap_Inv, cid_t Cap_Proc, ptr_t Vaddr);
__EXTERN__ ret_t _RME_Inv_Del(struct RME_Cap_Captbl* Captbl, cid_t Cap_Captbl, cid_t Cap_Inv);
__EXTERN__ ret_t _RME_Inv_Set(struct RME_Cap_Captbl* Captbl, cid_t Cap_Inv, ptr_t Entry, ptr_t Stack);
__EXTERN__ ret_t _RME_Inv_Act(struct RME_Cap_Captbl* Captbl, 
                              struct RME_Reg_Struct* Reg,
                              cid_t Cap_Inv, ptr_t Param);
__EXTERN__ ret_t _RME_Inv_Ret(struct RME_Reg_Struct* Reg);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __SIGINV_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
