/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Hardware-dependent module providing a time base
*           by programming a system timer
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#ifndef __TIMER__
#define __TIMER__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* type of a timer callback function */
typedef void (*PFV)(void);

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz);

/* Install <handler> as timer callback function */
void TimerSetHandler(PFV handler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIMER__ */
