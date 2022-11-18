/*! \file system_tc2x.h
 *  \brief Extended system control API for TC2x definition
 *
 *  \autor TGL
 *
 *  \version
 *    12.09.2011  initial version
 *
 */

#ifndef __SYSTEM_TC2X_H__
#define __SYSTEM_TC2X_H__

#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*! \brief Check if cache is enabled
 */
int SYSTEM_IsCacheEnabled(void);

/*! \brief Enable/disable cache
 */
void SYSTEM_EnaDisCache(int Enable);

/*   0,1,2 ... core WDT
 *   3     ... sec WDT
 */
void SYSTEM_EnableProtectionExt(int Sel);
void SYSTEM_DisableProtectionExt(int Sel);

void SYSTEM_EnableSecProtection(void);
void SYSTEM_DisableSecProtection(void);

unsigned long SYSTEM_GetStmClock(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYSTEM_TC2X_H__ */
