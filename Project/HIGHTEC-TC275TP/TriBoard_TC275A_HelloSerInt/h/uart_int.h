/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line
*           (interrupt variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#ifndef __UART_INT_H__
#define __UART_INT_H__

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

/* Initialise asynchronous interface to operate at BAUDRATE,8,n,1 */
void _init_uart(int baudrate);

/* send a buffer of given size <len> over serial line */
int _uart_send(const char *buffer, int len);

/* send a string over serial line */
int _uart_puts(const char *str);

/* get a character from serial line */
int _uart_getchar(char *c);

/* test UARTs sending state */
int _uart_sending(void);

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* __UART_INT_H__ */
