/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Transmit and receive characters via serial line
*           (polling variant)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#ifndef __UART_POLL_H__
#define __UART_POLL_H__

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

/* Initialise asynchronous interface to operate at baudrate,8,n,1 */
void _init_uart(int baudrate);

/* Send character CHR via the serial line */
void _out_uart(const unsigned char chr);

/* Receive (and return) a character from the serial line */
unsigned char _in_uart(void);

/* Check the serial line if a character has been received.
   returns 1 and the character in *chr if there is one
   else 0
 */
int _poll_uart(unsigned char *chr);

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* __UART_POLL_H__ */
