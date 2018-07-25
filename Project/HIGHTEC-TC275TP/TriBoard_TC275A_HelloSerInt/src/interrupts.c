/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Handling of interrupts on TC27x
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#include <machine/wdtcon.h>
#include <machine/intrinsics.h>

#ifdef TRIBOARD_TC275B
#include "tc27xb/IfxCpu_reg.h"
#include "tc27xb/IfxCpu_bf.h"
#include "tc27xb/IfxSrc_reg.h"
#else
#if defined(APPKIT_TC275TU_C) || defined(TRIBOARD_TC275C)
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"
#include "tc27xc/IfxSrc_reg.h"
#else
#include "tc27xa/IfxCpu_reg.h"
#include "tc27xa/IfxCpu_bf.h"
#include "tc27xa/IfxSrc_reg.h"
#endif /* APPKIT_TC275TU_C || TRIBOARD_TC275C */
#endif /* TRIBOARD_TC275B */
#include "interrupts.h"


extern void _init_vectab(void);
extern int _install_int_handler(int intno, void (*handler)(int), int arg);


/* Service Request Control register */
typedef union _Ifx_SRC_t
{
	volatile unsigned int R;
	struct _bits
	{
		volatile unsigned int SRPN   : 8;	/* [7:0] Service Request Priority Number (rw) */
		volatile unsigned int        : 2;
		volatile unsigned int SRE    : 1;	/* [10:10] Service Request Enable (rw) */
		volatile unsigned int TOS    : 2;	/* [12:11] Type of Service Control (rw) */
		volatile unsigned int        : 3;
		volatile unsigned int ECC    : 6;	/* [21:16] ECC (rwh) */
		volatile unsigned int        : 2;
		volatile unsigned int SRR    : 1;	/* [24:24] Service Request Flag (rh) */
		volatile unsigned int CLRR   : 1;	/* [25:25] Request Clear Bit (w) */
		volatile unsigned int SETR   : 1;	/* [26:26] Request Set Bit (w) */
		volatile unsigned int IOV    : 1;	/* [27:27] Interrupt Trigger Overflow Bit (rh) */
		volatile unsigned int IOVCLR : 1;	/* [28:28] Interrupt Trigger Overflow Clear Bit (w) */
		volatile unsigned int SWS    : 1;	/* [29:29] SW Sticky Bit (rh) */
		volatile unsigned int SWSCLR : 1;	/* [30:30] SW Sticky Clear Bit (w) */
		volatile unsigned int        : 1;
	} B;
} Ifx_SRC_t;


static Ifx_SRC_t * const tabSRC = (Ifx_SRC_t *)&MODULE_SRC;


/*---------------------------------------------------------------------
	Function:	InterruptInit
	Purpose:	Initialisation of interrupt handling
	Arguments:	void
	Return:		void
---------------------------------------------------------------------*/
void InterruptInit(void)
{
	/* basic initialisation of vector tables */
	_init_vectab();

	/* enable external interrupts */
	_enable();
}

/*---------------------------------------------------------------------
	Function:	InterruptInstall
	Purpose:	Install a service handler for an interrupt
	Arguments:	int irqNum       - number of interrupt
				isrhnd_t isrProc - pointer to service routine
				int prio         - priority (1-255)
				int arg          - argument for service routine
	Return:		void
---------------------------------------------------------------------*/
void InterruptInstall(int irqNum, isrhnd_t isrProc, int prio, int arg)
{
	unsigned int coreId = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;

	if ((irqNum < 0) || (IRQ_ID_MAX_NUM <= irqNum))
	{
		return;
	}

	/* install the service routine */
	_install_int_handler(prio, isrProc, arg);

	/* set processor and priority values */
	tabSRC[irqNum].B.TOS  = coreId;
	tabSRC[irqNum].B.SRPN = prio;
	/* ... and enable it */
	tabSRC[irqNum].B.SRE = 1;
}
