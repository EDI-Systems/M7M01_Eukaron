/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Hardware-dependent module providing a time base
*           by programming a system timer (TriCore TC27xx version).
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#include <machine/wdtcon.h>
#include <machine/intrinsics.h>
#include "interrupts.h"
#include "system_tc2x.h"

#ifdef TRIBOARD_TC275B
#include "tc27xb/IfxStm_reg.h"
#include "tc27xb/IfxStm_bf.h"
#include "tc27xb/IfxCpu_reg.h"
#include "tc27xb/IfxCpu_bf.h"
#else
#if defined(APPKIT_TC275TU_C) || defined(TRIBOARD_TC275C)
#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"
#else
#include "tc27xa/IfxStm_reg.h"
#include "tc27xa/IfxStm_bf.h"
#include "tc27xa/IfxCpu_reg.h"
#include "tc27xa/IfxCpu_bf.h"
#endif /* APPKIT_TC275TU_C || TRIBOARD_TC275C */
#endif /* TRIBOARD_TC275B */


#define SYSTIME_ISR_PRIO	2

#define STM0_BASE			((Ifx_STM *)&MODULE_STM0)
#define STM1_BASE			((Ifx_STM *)&MODULE_STM1)
#define STM2_BASE			((Ifx_STM *)&MODULE_STM2)


/* type of an Interrupt Service Routine (ISR) handler */
typedef void (*PFV)(void);

void TimerInit(unsigned int hz);
void TimerSetHandler(PFV handler);


/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value = 0;

static PFV user_handler = (PFV)0;

static __inline Ifx_STM *systime_GetStmBase(void)
{
	switch (_mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK)
	{
		default :
		case 0 : return STM0_BASE;
		case 1 : return STM1_BASE;
		case 2 : return STM2_BASE;
	}
}

/* timer interrupt routine */
static void tick_irq(int reload_value)
{
	Ifx_STM *StmBase = systime_GetStmBase();

	/* set new compare value */
	StmBase->CMP[0].U += (unsigned int)reload_value;
	if (user_handler)
	{
		user_handler();
	}
}

void TimerInit(unsigned int hz)
{
	unsigned int CoreID = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	Ifx_STM *StmBase = systime_GetStmBase();
	unsigned int frequency = SYSTEM_GetStmClock();
	int irqId;

	reload_value = frequency / hz;

	switch (CoreID)
	{
		default :
		case 0 :
			irqId = SRC_ID_STM0SR0;
			break;
		case 1 :
			irqId = SRC_ID_STM1SR0;
			break;
		case 2 :
			irqId = SRC_ID_STM2SR0;
			break;
	}

	/* install handler for timer interrupt */
	InterruptInstall(irqId, tick_irq, SYSTIME_ISR_PRIO, (int)reload_value);

	/* reset interrupt flag */
	StmBase->ISCR.U = (IFX_STM_ISCR_CMP0IRR_MSK << IFX_STM_ISCR_CMP0IRR_OFF);
	/* prepare compare register */
	StmBase->CMP[0].U = StmBase->TIM0.U + reload_value;
	StmBase->CMCON.U  = 31;
	StmBase->ICR.B.CMP0EN = 1;
}

void TimerSetHandler(PFV handler)
{
	user_handler = handler;
}
