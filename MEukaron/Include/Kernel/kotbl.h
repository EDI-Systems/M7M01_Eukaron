/******************************************************************************
Filename    : kotbl.h
Author      : pry
Date        : 01/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of kernel object table.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __KOTBL_H_DEFS__
#define __KOTBL_H_DEFS__
/*****************************************************************************/
/* Bitmap reference error */
#define RME_ERR_KOT_BMP  (-1)

/* Number of slots, and size of each slot */
#define RME_KOTBL_SLOT_NUM     (RME_KMEM_SIZE>>RME_KMEM_SLOT_ORDER)
#define RME_KOTBL_SLOT_SIZE    RME_POW2(RME_KMEM_SLOT_ORDER)
#define RME_KOTBL_WORD_NUM     (RME_KOTBL_SLOT_NUM>>RME_WORD_ORDER)
/* Round the kernel object size to the entry slot size */
#define RME_KOTBL_ROUND(X)     RME_ROUND_UP(X,RME_KMEM_SLOT_ORDER)
/*****************************************************************************/
/* __KOTBL_H_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Structs *******************************************************************/
#ifdef __HDR_STRUCTS__
#ifndef __KOTBL_H_STRUCTS__
#define __KOTBL_H_STRUCTS__
/* We used structs in the header */

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __KOTBL_H_STRUCTS__ */
#endif
/* __HDR_STRUCTS__ */
#endif
/* End Structs ***************************************************************/

/* Private Global Variables **************************************************/
#if(!(defined __HDR_DEFS__||defined __HDR_STRUCTS__))
#ifndef __KOTBL_MEMBERS__
#define __KOTBL_MEMBERS__

/* In this way we can use the data structures and definitions in the headers */
#define __HDR_DEFS__

#undef __HDR_DEFS__

#define __HDR_STRUCTS__

#undef __HDR_STRUCTS__

/* If the header is not used in the public mode */
#ifndef __HDR_PUBLIC_MEMBERS__
/*****************************************************************************/
static ptr_t RME_Kotbl[RME_KOTBL_WORD_NUM];
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
__EXTERN__ ret_t _RME_Kotbl_Init(ptr_t Words);
__EXTERN__ ret_t _RME_Kotbl_Mark(ptr_t Kaddr, ptr_t Size);
__EXTERN__ ret_t _RME_Kotbl_Erase(ptr_t Kaddr, ptr_t Size);
/*****************************************************************************/
/* Undefine "__EXTERN__" to avoid redefinition */
#undef __EXTERN__
/* __KOTBL_MEMBERS__ */
#endif
/* !(defined __HDR_DEFS__||defined __HDR_STRUCTS__) */
#endif
/* End Public C Function Prototypes ******************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
