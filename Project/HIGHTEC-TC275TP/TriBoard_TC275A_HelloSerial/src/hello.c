/*====================================================================
* Project:  Board Support Package (BSP) examples
* Function: example using a serial line (polling mode).
*
* Copyright HighTec EDV-Systeme GmbH 1982-2013
*====================================================================*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "led.h"
#include "uart_poll.h"

#ifdef __TC161__
#include "system_tc2x.h"
#endif /* __TC161__ */

#ifdef MINIMAL_CODE
#include "usr_sprintf.h"
#define SPRINTF		usr_sprintf
#define VSPRINTF	usr_vsprintf
#else
#define SPRINTF		sprintf
#define VSPRINTF	vsprintf
#endif /* MINIMAL_CODE */

#define BUFSIZE		128

#ifndef BAUDRATE
#define BAUDRATE	38400
#endif /* BAUDRATE */

static const char *my_str = "Hello world!";

static void my_puts(const char *str)
{
	char buffer[BUFSIZE];
	char *ptr;

	SPRINTF(buffer, "%s\r\n", str);

	for (ptr = buffer; *ptr; ++ptr)
		_out_uart((const unsigned char) *ptr);
}

static void my_printf(const char *fmt, ...)
{
	char buffer[BUFSIZE];
	char *ptr;
	va_list ap;

	va_start(ap, fmt);
	VSPRINTF(buffer, fmt, ap);
	va_end(ap);

	for (ptr = buffer; *ptr; ++ptr)
		_out_uart((const unsigned char) *ptr);
}

int main(void)
{
	unsigned char c;
	int quit = 0;

#ifdef __TC161__
	SYSTEM_Init();
#endif /* __TC161__ */

	_init_uart(BAUDRATE);
	InitLED();

	my_puts(my_str);
	my_puts("Your choice please");

	while (!quit)
	{
		c = _in_uart();

		switch (c)
		{
			case '0' :
				LEDOFF(0);
				my_puts("LED switched to OFF");
				break;
			case '1' :
				LEDON(0);
				my_puts("LED switched to ON");
				break;
			case '2' :
				my_puts(my_str);
				break;
			case 'E' :
				quit = 1;
				my_puts("Bye bye!");
				break;
			case '\n' :
			case '\r' :
				/* do nothing -- ignore it */
				break;
			default :
				my_printf("Command '%c' not supported\r\n", c);
				break;
		}
	}

	return EXIT_SUCCESS;
}
