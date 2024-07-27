/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line
*           (polling variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/
#define MODULE_UART_POLL

#include "bspconfig.h"
#include "uart_poll.h"

/* Check the serial line if a character has been received.
   returns 1 and the character in *chr if there is one
   else 0
 */
int _poll_uart(unsigned char *chr)
{
	unsigned char ret;
	int res = 0;

	if (RX_READY(UARTBASE))
	{
		ret = (unsigned char)GET_CHAR(UARTBASE);
		/* acknowledge receive */
		RX_CLEAR(UARTBASE);
		/* check for error condition */
		if (GET_ERROR_STATUS(UARTBASE))
		{
			/* reset error flags */
			RESET_ERROR(UARTBASE);
			/* ignore this character */
		}
		else
		{
			/* this is a valid character */
			*chr = ret;
			res = 1;
		}
	}

	return res;
}


/* Receive (and wait for) a character from the serial line */
unsigned char _in_uart(void)
{
	unsigned char ch;

	/* wait for a new character */
	while (_poll_uart(&ch) == 0)
		;

	return ch;
}


/* Send character CHR via the serial line */
void _out_uart(const unsigned char chr)
{
	/* wait until space is available in the FIFO */
	while (!TX_READY(UARTBASE))
		;

	TX_CLEAR(UARTBASE);

	/* send the character */
	PUT_CHAR(UARTBASE, chr);
}
