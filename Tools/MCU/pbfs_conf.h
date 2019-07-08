/******************************************************************************
Filename    : pbfs_conf.h
Author      : pry
Date        : 03/11/2018
Licence     : LGPL v3+; see COPYING for details.
Description : The configuration header for piggyback filesystem library.
******************************************************************************/

/* Compiler keywords *********************************************************/
#define EXTERN                              extern
/* End Compiler keywords *****************************************************/

/* Configuration Typedefs ****************************************************/
typedef signed char                         pbfs_s8_t;
typedef unsigned char                       pbfs_u8_t;
typedef unsigned long long                  pbfs_ptr_t;
typedef long long                           pbfs_ret_t;
typedef unsigned int                        pbfs_u32_t;
/* End Configuration Typedefs ************************************************/

/* Debugging Defines *********************************************************/
/* #define PBFS_ASSERT_CORRECT */
/* #define PBFS_LOG_FUNC_NAME               RMW_Log  */
/* End Debugging Defines *****************************************************/

/* Configuration Defines *****************************************************/
#define PBFS_STATIC

#define PBFS_READ_FUNC_NAME                 PBFS_Read

/* 
#define PBFS_MINIMAL
*/
/* End Configuration Defines *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
