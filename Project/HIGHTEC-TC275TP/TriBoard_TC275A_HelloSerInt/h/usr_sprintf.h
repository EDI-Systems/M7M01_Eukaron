/*====================================================================
* Project:  Board Support Package (BSP) examples
* Function: simplified formatted output into buffer (sprintf)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2013
*====================================================================*/
#ifndef __USR_SPRINTF_H__
#define __USR_SPRINTF_H__

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

#include <stdarg.h>

int usr_vsprintf(char *dest, const char *fmt, va_list ap);

int usr_sprintf(char *buf, char const *fmt, ...);

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* __USR_SPRINTF_H__ */
