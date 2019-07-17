/******************************************************************************
Filename    : xml_conf.h
Author      : pry
Date        : 03/11/2018
Licence     : LGPL v3+; see COPYING for details.
Description : The configuration header for xml parsing library.
******************************************************************************/

/* Compiler keywords *********************************************************/
#define EXTERN                              extern
/* End Compiler keywords *****************************************************/

/* Configuration Typedefs ****************************************************/
typedef char                                xml_s8_t;
typedef unsigned long long                  xml_ptr_t;
typedef long long                           xml_ret_t;
typedef double                              xml_flt_t;
/* End Configuration Typedefs ************************************************/

/* Debugging Defines *********************************************************/
/* #define XML_ASSERT_CORRECT */
/* #define XML_LOG_FUNC_NAME                RMW_Log  */
/* End Debugging Defines *****************************************************/

/* Configuration Defines *****************************************************/
#define XML_STATIC

#define XML_MALLOC_FUNC_NAME                Malloc
#define XML_FREE_FUNC_NAME                  Free
#define XML_STRNCMP_FUNC_NAME               Strncmp
#define XML_MEMCPY_FUNC_NAME                Memcpy
#define XML_STRLEN_FUNC_NAME                Strlen

/* 
#define XML_MINIMAL
*/
/* End Configuration Defines *************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
