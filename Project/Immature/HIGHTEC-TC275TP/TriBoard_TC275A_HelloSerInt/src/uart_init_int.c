/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via TriCore's serial line
*           (Infineon controller TC27xx)
*           (interrupt variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/
#define MODULE_UART_INT

#include "bspconfig.h"

#include <machine/wdtcon.h>

/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
void _uart_init_bsp(int baudrate, void (*uart_rx_isr)(int arg), void (*uart_tx_isr)(int arg))
{
	unsigned int numerator;
	unsigned int denominator;

	/* install handlers for transmit and receive interrupts */
	InterruptInstall(SRC_ID_ASCLIN0TX, uart_tx_isr, XMIT_INTERRUPT, 0);
	InterruptInstall(SRC_ID_ASCLIN0RX, uart_rx_isr, RECV_INTERRUPT, 0);

#if defined(APPKIT_TC275TU) || defined(APPKIT_TC275TU_C)
	/* on board wiggler is connected to ASCLIN0 */
	/* ARX0A/P14.1 (RXD), ATX0/P14.0 (TXD) */
	/* Set TXD/P14.0 to "output" and "high" */
	port->IOCR0.B.PC0 = OUT_PPALT2;
	port->OMR.B.PS0 = 1;
#else
	/* on board wiggler is connected to ASCLIN0 */
	/* ARX0B/P15.3 (RXD), ATX0/P15.2 (TXD) */
	/* Set TXD/P15.2 to "output" and "high" */
	port->IOCR0.B.PC2 = OUT_PPALT2;
	port->OMR.B.PS2 = 1;
#endif /* APPKIT_TC275TU || APPKIT_TC275TU_C */

	/* baudrate values at 100 MHz */
	denominator = 3125;

	switch (baudrate)
	{
		case   9600 : numerator =   BAUD_9600; break;
		case  19200 : numerator =  BAUD_19200; break;
		case  57600 : numerator =  BAUD_57600; break;
		case 115200 : numerator = BAUD_115200; break;
		default     : numerator =  BAUD_38400; break;
	}

	/* Enable ASCn */
	unlock_wdtcon();
	UARTBASE->CLC.U = 0;
	lock_wdtcon();
	/* read back for activating module */
	(void)UARTBASE->CLC.U;

#if defined(APPKIT_TC275TU) || defined(APPKIT_TC275TU_C)
	/* select ARX0A/P14.1 as input pin */
	UARTBASE->IOCR.B.ALTI = 0;
#else
	/* select ARX0B/P15.3 as input pin */
	UARTBASE->IOCR.B.ALTI = 1;
#endif /* APPKIT_TC275TU || APPKIT_TC275TU_C */

	/* Program ASC0 */
	UARTBASE->CSR.U = 0;

	/* configure TX and RX FIFOs */
	UARTBASE->TXFIFOCON.U = (1 << 6)	/* INW: (1 == 1 byte) */
						  | (1 << 1)	/* ENO */
						  | (1 << 0);	/* FLUSH */
	UARTBASE->RXFIFOCON.U = (1 << 6)	/* OUTW: (1 == 1 byte) */
						  | (1 << 1)	/* ENI */
						  | (1 << 0);	/* FLUSH */

	UARTBASE->BITCON.U = ( 9 << 0)		/* PRESCALER: 10 */
					   | (15 << 16)		/* OVERSAMPLING: 16 */
					   | ( 9 << 24)		/* SAMPLEPOINT: position 7,8,9 */
					   | (1u << 31);	/* SM: 3 samples per bit */

	/* data format: 8N1 */
	UARTBASE->FRAMECON.U = (1 << 9)		/* STOP: 1 bit */
						 | (0 << 16)	/* MODE: Init */
						 | (0 << 30);	/* PEN: no parity */
	UARTBASE->DATCON.U = (7 << 0);		/* DATLEN: 8 bit */

	/* set baudrate value */
	UARTBASE->BRG.U = (denominator << 0)	/* DENOMINATOR */
					| (numerator << 16);	/* NUMERATOR */

	UARTBASE->FRAMECON.B.MODE = 1;			/* ASC mode */
	UARTBASE->FLAGSENABLE.U = (1u << 28);	/* FLAGSENABLE.RFLE */
	UARTBASE->CSR.U = 1;					/* select CLC as clock source */
}
