/*
 *    Extended system control API implementation for TC27x
 */

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>
#ifdef USE_IRQ
#include "interrupts.h"
#endif /* USE_IRQ */

#include "bspconfig.h"
#include "system_tc2x.h"

#ifdef TRIBOARD_TC275B
#include "tc27xb/IfxScu_reg.h"
#include "tc27xb/IfxScu_bf.h"
#include "tc27xb/IfxCpu_reg.h"
#include "tc27xb/IfxCpu_bf.h"
#else
#if defined(APPKIT_TC275TU_C) || defined(TRIBOARD_TC275C)
#include "tc27xc/IfxScu_reg.h"
#include "tc27xc/IfxScu_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"
#else
#include "tc27xa/IfxScu_reg.h"
#include "tc27xa/IfxScu_bf.h"
#include "tc27xa/IfxCpu_reg.h"
#include "tc27xa/IfxCpu_bf.h"
#endif /* APPKIT_TC275TU_C || TRIBOARD_TC275C */
#endif /* TRIBOARD_TC275B */


typedef struct tagPllInitValue
{
	unsigned int uiOSCCON;
	unsigned int uiPLLCON0;
	unsigned int uiPLLCON1;
	unsigned int uiCCUCON0;
	unsigned int uiCCUCON1;
	unsigned int uiCCUCON2;
} TPllInitValue, *PPllInitValue;

extern const TPllInitValue g_PllInitValue_200_100;
#define PLL_VALUE_200_100 ((const PPllInitValue)(&g_PllInitValue_200_100))

extern const TPllInitValue g_PllInitValue_100_50;
#define PLL_VALUE_100_50  ((const PPllInitValue)(&g_PllInitValue_100_50))


#ifndef EXTCLK
  #define EXTCLK		(20000000)	/* external oscillator clock (20MHz) */
#endif



#pragma section ".rodata"
/* PLL settings for 20MHz ext. clock */
#if (20000000 == EXTCLK)

/* 200/100 MHz @ 20MHz ext. clock */
const TPllInitValue g_PllInitValue_200_100 =
{
	/* OSCCON,	PLLCON0,	PLLCON1,	CCUCON0,	CCUCON1,	CCUCON2 */
#if defined(TRIBOARD_TC275A) || defined(APPKIT_TC275TU)
	0x0007001C, 0x01017600, 0x00020002, 0x52220101, 0x50012211, 0x40000202
#else
	0x0007001C, 0x01017600, 0x00020002, 0x52120112, 0x50012211, 0x40000002
#endif /* TRIBOARD_TC275A || APPKIT_TC275TU */
};

/* 100/50 MHz @ 20MHz ext. clock */
const TPllInitValue g_PllInitValue_100_50 =
{
	/* OSCCON,	PLLCON0,	PLLCON1,	CCUCON0,	CCUCON1,	CCUCON2 */
#if defined(TRIBOARD_TC275A) || defined(APPKIT_TC275TU)
	0x0007001C, 0x01018a00, 0x00020006, 0x52220101, 0x50012211, 0x40000202
#else
	0x0007001C, 0x01018a00, 0x00020006, 0x52120112, 0x50012211, 0x40000002
#endif /* TRIBOARD_TC275A || APPKIT_TC275TU */
};
#else
#error "ERROR: UNSUPPORTED EXTERNAL CLOCK!"
#endif /* PLL settings for 20MHz ext. clock */
#pragma section


static Ifx_SCU * const pSCU = (Ifx_SCU *)&MODULE_SCU;


#ifndef SYSTEM_DONT_SET_PLL
static void system_set_pll(const PPllInitValue pPllInitValue)
{
	unlock_safety_wdtcon();

	pSCU->OSCCON.U = pPllInitValue->uiOSCCON;

	while (pSCU->CCUCON1.B.LCK)
		;
	pSCU->CCUCON1.U = pPllInitValue->uiCCUCON1 | (1 << IFX_SCU_CCUCON1_UP_OFF);
	while (pSCU->CCUCON1.B.LCK)
		;
	pSCU->CCUCON1.U = pPllInitValue->uiCCUCON1;

	while (pSCU->CCUCON2.B.LCK)
		;
	pSCU->CCUCON2.U = pPllInitValue->uiCCUCON2 | (1 << IFX_SCU_CCUCON2_UP_OFF);
	while (pSCU->CCUCON2.B.LCK)
		;
	pSCU->CCUCON2.U = pPllInitValue->uiCCUCON2;

	pSCU->PLLCON0.U |= ((1 << IFX_SCU_PLLCON0_VCOBYP_OFF) | (1 << IFX_SCU_PLLCON0_SETFINDIS_OFF));
	pSCU->PLLCON1.U =  pPllInitValue->uiPLLCON1;				/* set K1,K2 divider */
	pSCU->PLLCON0.U =  pPllInitValue->uiPLLCON0					/* set P,N divider */
					| ((1 << IFX_SCU_PLLCON0_VCOBYP_OFF) | (1 << IFX_SCU_PLLCON0_CLRFINDIS_OFF));
	while (pSCU->CCUCON0.B.LCK)
		;
	pSCU->CCUCON0.U =  pPllInitValue->uiCCUCON0 | (1 << IFX_SCU_CCUCON0_UP_OFF);
	while (pSCU->CCUCON0.B.LCK)
		;
	pSCU->CCUCON0.U =  pPllInitValue->uiCCUCON0;
	lock_safety_wdtcon();

	if (0 == (pPllInitValue->uiPLLCON0 & (1 << IFX_SCU_PLLCON0_VCOBYP_OFF)))	/* no prescaler mode requested */
	{
#ifndef SYSTEM_PLL_HAS_NO_LOCK
		/* wait for PLL locked */
		while (0 == pSCU->PLLSTAT.B.VCOLOCK)
			;
#endif

		unlock_safety_wdtcon();
		pSCU->PLLCON0.B.VCOBYP = 0;			/* disable VCO bypass */
		lock_safety_wdtcon();
	}
}
#endif

/*! \brief System initialisation
 *  \param pPllInitValue ... address of PLL initialisation struct
 */
static void SYSTEM_InitExt(const PPllInitValue pPllInitValue)
{
#ifndef SYSTEM_DONT_SET_PLL
	/* initialise PLL (only done by CPU0) */
	if (0 == (_mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK))
		system_set_pll(pPllInitValue);
#endif

#ifdef USE_IRQ
	/* activate interrupt system */
	InterruptInit();
#endif /* USE_IRQ */
}

void SYSTEM_Init(void)
{
	SYSTEM_InitExt(DEFAULT_PLL_VALUE);
}

unsigned long SYSTEM_GetExtClock(void)
{
	return EXTCLK;
}

static unsigned long system_GetPllClock(void)
{
	unsigned int frequency = EXTCLK;	/* fOSC */

	Ifx_SCU_PLLSTAT pllstat = pSCU->PLLSTAT;
	Ifx_SCU_PLLCON0 pllcon0 = pSCU->PLLCON0;
	Ifx_SCU_PLLCON1 pllcon1 = pSCU->PLLCON1;

	if (0 == (pllstat.B.VCOBYST))
	{
		if (0 == (pllstat.B.FINDIS))
		{
			/* normal mode */
			frequency *= (pllcon0.B.NDIV + 1);		/* fOSC*N */
			frequency /= (pllcon0.B.PDIV + 1);		/* .../P  */
			frequency /= (pllcon1.B.K2DIV + 1);		/* .../K2 */
		}
		else	/* freerunning mode */
		{
			frequency = 800000000;		/* fVCOBASE 800 MHz (???) */
			frequency /= (pllcon1.B.K2DIV + 1);		/* .../K2 */
		}
	}
	else	/* prescaler mode */
	{
		frequency /= (pllcon1.B.K1DIV + 1);		/* fOSC/K1 */
	}

	return (unsigned long)frequency;
}

static unsigned long system_GetIntClock(void)
{
	unsigned long frequency = 0;
	switch (pSCU->CCUCON0.B.CLKSEL)
	{
		default:
		case 0:  /* back-up clock (typ. 100 MHz) */
			frequency = 100000000ul;
			break;
		case 1:	 /* fPLL */
			frequency = system_GetPllClock();
			break;
	}
	return frequency;
}

unsigned long SYSTEM_GetCpuClock(void)
{
	unsigned long frequency = system_GetIntClock();
	unsigned long divider;
#if defined(TRIBOARD_TC275A) || defined(APPKIT_TC275TU)
	/* A Step devices */
	/* fCPU0 = fPLL / CPU0DIV */
	divider = pSCU->CCUCON0.B.CPU0DIV;
#else
	/* B + C Step devices */
	/* fCPU = fSRI */
	divider = pSCU->CCUCON0.B.SRIDIV;
#endif /* TRIBOARD_TC275A || APPKIT_TC275TU */
	if (0 == divider)
		return 0;
	return (frequency / divider);
}

unsigned long SYSTEM_GetSysClock(void)
{
	unsigned long frequency = system_GetIntClock();
	unsigned long divider = pSCU->CCUCON0.B.SPBDIV;
	if (0 == divider)
		return 0;
	return (frequency / divider);
}

unsigned long SYSTEM_GetStmClock(void)
{
	unsigned long frequency = system_GetIntClock();
	unsigned long divider = pSCU->CCUCON1.B.STMDIV;
	if (0 == divider)
		return 0;
	return (frequency / divider);
}

void SYSTEM_EnableInterrupts(void)
{
	_enable();
}

void SYSTEM_DisableInterrupts(void)
{
	_disable();
}

void SYSTEM_EnableProtection(void)
{
	lock_wdtcon();
}

void SYSTEM_DisableProtection(void)
{
	unlock_wdtcon();
}

void SYSTEM_EnableProtectionExt(int Sel)
{
	if (Sel < 0)
		SYSTEM_EnableProtection();
	else if (Sel < 3)
		lock_wdtcon();			/* CPU watchdog */
	else
		lock_safety_wdtcon();	/* security watchdog */
}

void SYSTEM_DisableProtectionExt(int Sel)
{
	if (Sel < 0)
		SYSTEM_DisableProtection();
	else if (Sel < 3)
		unlock_wdtcon();		/* CPU watchdog */
	else
		unlock_safety_wdtcon();	/* security watchdog */
}

void SYSTEM_EnableSecProtection(void)
{
	lock_safety_wdtcon();
}

void SYSTEM_DisableSecProtection(void)
{
	unlock_safety_wdtcon();
}


int SYSTEM_Reset(void)
{
	SYSTEM_DisableProtection();
	pSCU->SWRSTCON.B.SWRSTREQ = 1;
	while (1)
	{
	}
	return 0;
}


int SYSTEM_IdleExt(int CoreId)
{
	unlock_wdtcon();
	switch (CoreId)
	{
		case 0:
			pSCU->PMCSR[0].U = 1;
			break;
		case 1:
			pSCU->PMCSR[1].U = 1;
			break;
		case 2:
			pSCU->PMCSR[2].U = 1;
			break;
	}
	lock_wdtcon();
	return 0;
}

int SYSTEM_Idle(void)
{
	return SYSTEM_IdleExt(_mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK);
}

int SYSTEM_Sleep(void)
{
#if 0
	SCU *pScu=(SCU *)SCU_BASE;
	pScu->PMG_CSR = REQSLP_SLEEP;
	return 0;
#else
	return -1;	/* !!!!!!!!!!1 */
#endif
}


int SYSTEM_IsCacheEnabled(void)
{
	unsigned int ui = _mfcr(CPU_PCON0);
	if (ui & 2)
		return 0;	/* Cache is in bypass mode */
	ui = _mfcr(CPU_PCON2);
	if (0 == (ui & (0xFFFF<<16)))
		return 0;	/* Cache size is 0 */
	return 1;
}

void SYSTEM_EnaDisCache(int Enable)
{
	SYSTEM_DisableProtection();
	if (Enable)
	{
		_mtcr(CPU_PCON0, 0);
		_mtcr(CPU_DCON0, 0);
	}
	else	/* disable */
	{
		_mtcr(CPU_PCON0, 2);
		_mtcr(CPU_PCON1, 3);
		_mtcr(CPU_DCON0, 2);
	}
	SYSTEM_EnableProtection();
}

void SYSTEM_DbgBreak(void)
{
#ifdef DEBUG
	__asm volatile ("debug");
#else
	while (1)
		;
#endif
}
