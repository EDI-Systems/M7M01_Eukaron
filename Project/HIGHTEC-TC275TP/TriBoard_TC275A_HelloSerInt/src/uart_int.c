/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line.
*           (interrupt variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/
#define MODULE_UART_INT

#include <string.h>

#include "bspconfig.h"
#include "uart_int.h"


#ifndef RS232_RX_BUFSIZE
#define RS232_RX_BUFSIZE	0x100
#endif /* RS232_RX_BUFSIZE */

#ifndef RS232_TX_BUFSIZE
#define RS232_TX_BUFSIZE	0x200
#endif /* RS232_TX_BUFSIZE */

#ifndef RX_CLEAR
#define RX_CLEAR(u)			;
#endif

#ifndef TX_CLEAR
#define TX_CLEAR(u)			;
#endif

#if defined(__arm__)
#define ARM_ONLY(x)			x
#define NOT_ON_ARM(x)
#else
#define ARM_ONLY(x)
#define NOT_ON_ARM(x)		x
#endif

#if defined(__tricore__)
#define TRICORE_ONLY(x)		x
#define NOT_ON_TRICORE(x)
#else
#define NOT_ON_TRICORE(x)	x
#define TRICORE_ONLY(x)
#endif

#if defined(__tricore__)
#define ISR_ARGLIST		int arg
#else
#define ISR_ARGLIST		void
#endif


typedef struct
{
	unsigned int	head;
	unsigned int	tail;
	char			buf[RS232_RX_BUFSIZE];
} RxBuffer_t;


typedef struct
{
	unsigned int	head;
	unsigned int	tail;
	char			buf[RS232_TX_BUFSIZE];
} TxBuffer_t;


/* Circular send and receive buffers */
static TxBuffer_t sendBuf;
static RxBuffer_t recvBuf;


/* FIFO support */
static __inline int isEmptyTXFifo(void)
{
	return (sendBuf.tail == sendBuf.head);
}


static __inline int getFreeTXFifo(void)
{
	int used = (RS232_TX_BUFSIZE + sendBuf.head - sendBuf.tail) % RS232_TX_BUFSIZE;
	return (RS232_TX_BUFSIZE - 1 - used);
}


static __inline int readTXFifo(char *cp)
{
	int res = 0;
	if (sendBuf.tail != sendBuf.head)
	{
		unsigned int next = (sendBuf.tail + 1) % RS232_TX_BUFSIZE;
		*cp = sendBuf.buf[sendBuf.tail];
		sendBuf.tail = next;
		res = 1;
	}
	return res;
}


static __inline int writeTXFifo(char c)
{
	int res = 0;
	unsigned int next = (sendBuf.head + 1) % RS232_TX_BUFSIZE;
	if (next != sendBuf.tail)
	{
		sendBuf.buf[sendBuf.head] = c;
		sendBuf.head = next;
		res = 1;
	}
	return res;
}


static __inline int readRXFifo(char *cp)
{
	int res = 0;
	if (recvBuf.tail != recvBuf.head)
	{
		unsigned int next = (recvBuf.tail + 1) % RS232_RX_BUFSIZE;
		*cp = recvBuf.buf[recvBuf.tail];
		recvBuf.tail = next;
		res = 1;
	}
	return res;
}


static __inline int writeRXFifo(char c)
{
	int res = 0;
	unsigned int next = (recvBuf.head + 1) % RS232_RX_BUFSIZE;
	if (next != recvBuf.tail)
	{
		recvBuf.buf[recvBuf.head] = c;
		recvBuf.head = next;
		res = 1;
	}
	return res;
}


/* Receive (and return) a character from the serial line */
static __inline char _in_uart(void)
{
	char ch;

	/* read the character */
	/* (this also resets any error flags on some devices, e.g. on stm32p103) */
	ch = (char)GET_CHAR(UARTBASE);
	/* acknowledge receive */
	RX_CLEAR(UARTBASE);
	return ch;
}


/* Send character CHR via the serial line */
static __inline void _out_uart(const char chr)
{
	TX_CLEAR(UARTBASE);
	/* send the character */
	PUT_CHAR(UARTBASE, chr);
}



#if defined(__arm__)
#if defined(UART_TYPE_I)

#ifndef SEPARATE_ISR
#define SEPARATE_ISR
#endif

static int _uart_rx_handler(void);
static void _uart_tx_handler(void);

/* Interrupt Service Routines for RX and TX */
static void _uart_isr(void)
{
	unsigned state;

	do
	{
		state = GET_INT_STATUS(UARTBASE);
		/* RX interrupt */
		if (IS_RX_INT(state))
		{
			if (_uart_rx_handler() != 0) continue;
		}

		/* TX interrupt */
		if (IS_TX_INT(state))
		{
			_uart_tx_handler();
		}
	} while (NO_MORE_INT(state));	/* no more pending */
}

#elif defined(UART_TYPE_II)

#ifndef SEPARATE_ISR
#define SEPARATE_ISR
#endif

static int _uart_rx_handler(void);
static void _uart_tx_handler(void);

/* Interrupt Service Routines for RX and TX */
static void _uart_isr(void)
{
	unsigned state;

	while ((state = GET_INT_STATUS(UARTBASE)) != 0)
	{
		/* RX interrupt */
		if (IS_RX_INT(state))
		{
			if (_uart_rx_handler() != 0) continue;
		}

		/* TX interrupt */
		if (IS_TX_INT(state))
		{
			_uart_tx_handler();
		}
	}
}

#elif defined(UART_TYPE_III)

/* Interrupt Service Routines for RX and TX */
static void _uart_isr(void)
{
	unsigned int state;
	unsigned int mask = USART_SR_RXNE | USART_MASK_ERROR;

	if (UARTBASE->cr1 & USART_CR1_TXEIE)
	{
		mask |= USART_SR_TXE;
	}

	while ((state = (UARTBASE->sr & mask)) != 0)
	{
		/* RX interrupt */
		if (IS_RX_INT(state))
		{
			char c = _in_uart();
			/* check for error condition */
			if (GET_ERROR_STATUS(state))
			{
				/* reset error flags */
				RESET_ERROR(UARTBASE);
				/* ignore this character */
				continue;
			}
#ifdef LINBUS_MODE
			/* ignore echo from transmit action */
			if (0 == (mask & USART_SR_TXE))
			{
				writeRXFifo(c);
			}
#else
			writeRXFifo(c);
#endif /* LINBUS_MODE */
		}

		/* TX interrupt */
		if (IS_TX_INT(state))
		{
			char c;
			if (readTXFifo(&c))
			{
				_out_uart(c);
			}
			else
			{
				/* all done --> disable TX interrupt */
				UARTBASE->cr1 &= ~USART_CR1_TXEIE;
				mask &= ~USART_SR_TXE;
			}
		}
	}
}

#endif /* UART_TYPE_I */

#else /* __arm__ */

/* PowerPC and TriCore always uses separate ISRs for RX and TX */
#define SEPARATE_ISR

#endif /* __arm__ */


#if defined(SEPARATE_ISR)

/* Interrupt Service Routine for RX */
static ARM_ONLY(int) NOT_ON_ARM(void) _uart_rx_handler(ISR_ARGLIST)
{
	char c;
	TRICORE_ONLY((void)arg);
	NOT_ON_TRICORE(c = _in_uart());

	/* check for error condition */
	if (GET_ERROR_STATUS(UARTBASE))
	{
		/* ignore this character */
		TRICORE_ONLY(_in_uart());
		/* reset error flags */
		RESET_ERROR(UARTBASE);

		ARM_ONLY(return 1);
	}
	else
	{
		TRICORE_ONLY(c = _in_uart());
		writeRXFifo(c);
	}

	ARM_ONLY(return 0);
}


/* Interrupt Service Routine for TX */
static void _uart_tx_handler(ISR_ARGLIST)
{
	char c;
	TRICORE_ONLY((void)arg);

	if (readTXFifo(&c))
	{
		_out_uart(c);
	}
#ifndef KEEP_TX_INT_ENABLED
	else
	{
		/* all done --> disable TX interrupt */
		TX_STOP(UARTBASE);
	}
#endif /* KEEP_TX_INT_ENABLED */
}

#endif /* SEPARATE_ISR */


/* Externally visible functions */


/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
void _init_uart(int baudrate)
{
#if defined(__arm__)
    _uart_init_bsp(baudrate, _uart_isr);
#elif (defined(__PPC__) || defined(__tricore__))
    _uart_init_bsp(baudrate, _uart_rx_handler, _uart_tx_handler);
#else
#error Unknown architecture!
#endif
}


/* send a buffer of given size <len> over serial line */
int _uart_send(const char *buffer, int len)
{
	int ok;

	ok  = ((getFreeTXFifo() >= len) ? 1 : 0);

	if (ok && len)
	{
		int cnt = 0, trig;
		trig = isEmptyTXFifo();
		for (; cnt < len; ++cnt)
		{
			writeTXFifo(*buffer++);
		}
		if (trig
#ifdef __tricore__
				&& !TX_CHECK(UARTBASE)
#endif
			)
		{
#if defined(UART_TYPE_I)
			char c;
			if (readTXFifo(&c))
			{
				_out_uart(c);
#ifndef KEEP_TX_INT_ENABLED
				if (--len)
				{
					/* enable TX interrupt for sending */
					TX_START(UARTBASE);
				}
#endif /* KEEP_TX_INT_ENABLED */
			}
#else /* UART_TYPE_I */
			/* enable TX interrupt for sending */
			TX_START(UARTBASE);
#endif /* UART_TYPE_I */
		}
	}

	return ok;
}


/* send a string over serial line */
int _uart_puts(const char *str)
{
	int len = strlen(str);

	return _uart_send(str, len);
}


/* get a character from serial line */
int _uart_getchar(char *c)
{
	return readRXFifo(c);
}


/* test UARTs sending state */
int _uart_sending(void)
{
	int ret = (0 == isEmptyTXFifo());
#ifdef __tricore__
	if (0 == ret)
	{
		/* wait until last byte is sent */
		ret = TX_CHECK(UARTBASE);
	}
#endif
	return ret;
}
