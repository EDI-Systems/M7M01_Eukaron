/*====================================================================
* Project:  Board Support Package (BSP)
* Function: LEDs
*
* Copyright HighTec EDV-Systeme GmbH 1982-2013
*====================================================================*/

#ifndef __LED__
#define __LED__

#include "bspconfig.h"

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

#ifndef MAX_LED
#define MAX_LED 0
#endif

#if (MAX_LED > 0)
static __inline void LEDON(int nr)
{
	if (nr < MAX_LED)
	{
		LED_ON(nr);
	}
}

static __inline void LEDOFF(int nr)
{
	if (nr < MAX_LED)
	{
		LED_OFF(nr);
	}
}

static __inline void LEDTOGGLE(int nr)
{
	if (nr < MAX_LED)
	{
		LED_TOGGLE(nr);
	}
}
#else
/* no support for LEDs */
static __inline void LEDON(int nr)
{
	(void)nr;
}

static __inline void LEDOFF(int nr)
{
	(void)nr;
}

static __inline void LEDTOGGLE(int nr)
{
	(void)nr;
}

#ifndef INIT_LEDS
#define INIT_LEDS
#endif /* INIT_LEDS */
#endif /* LED_GREEN */

static __inline void InitLED(void)
{
	INIT_LEDS;
}

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* __LED__ */
