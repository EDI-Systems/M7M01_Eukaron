/*====================================================================
* Project:  Board Support Package (BSP) examples
* Function: simplified formatted output into buffer (sprintf)
*
* Copyright HighTec EDV-Systeme GmbH 1982-2013
*====================================================================*/

#include <stdarg.h>
#include <string.h>

#include "usr_sprintf.h"

#ifndef FALSE
#define FALSE	0
#endif /* FALSE */
#ifndef TRUE
#define TRUE	1
#endif /* TRUE */

#define is_digit(c)		((c >= '0') && (c <= '9'))

static int _cvt(unsigned long val, char *buf, long radix, char *digits)
{
	char temp[80];
	char *cp = temp;
	int length = 0;

	if (val == 0)
	{
		/* Special case */
		*cp++ = '0';
	}
	else
	{
		while (val)
		{
			*cp++ = digits[val % radix];
			val /= radix;
		}
	}
	while (cp != temp)
	{
		*buf++ = *--cp;
		length++;
	}
	*buf = '\0';
	return length;
}

int usr_vsprintf(char *dest, const char *fmt, va_list ap)
{
	char c, sign, *cp, *dp = dest;
	int left_prec, right_prec, zero_fill, length, pad, pad_on_right;
	char buf[32];
	long val;

	while ((c = *fmt++) != 0)
	{
		cp = buf;
		length = 0;
		if (c == '%')
		{
			c = *fmt++;
			left_prec = right_prec = pad_on_right = 0;
			if (c == '-')
			{
				c = *fmt++;
				pad_on_right++;
			}
			if (c == '0')
			{
				zero_fill = TRUE;
				c = *fmt++;
			}
			else
			{
				zero_fill = FALSE;
			}
			while (is_digit(c))
			{
				left_prec = (left_prec * 10) + (c - '0');
				c = *fmt++;
			}
			if (c == '.')
			{
				c = *fmt++;
				zero_fill++;
				while (is_digit(c))
				{
					right_prec = (right_prec * 10) + (c - '0');
					c = *fmt++;
				}
			}
			else
			{
				right_prec = left_prec;
			}
			sign = '\0';
			/* handle type modifier */
			if (c == 'l' || c == 'h')
			{
				c = *fmt++;
			}
			switch (c)
			{
				case 'd' :
				case 'u' :
				case 'x' :
				case 'X' :
					val = va_arg(ap, long);
					switch (c)
					{
						case 'd' :
							if (val < 0)
							{
								sign = '-';
								val = -val;
							}
							/* fall through */
						case 'u' :
							length = _cvt(val, buf, 10, "0123456789");
							break;
						case 'x' :
							length = _cvt(val, buf, 16, "0123456789abcdef");
							break;
						case 'X' :
							length = _cvt(val, buf, 16, "0123456789ABCDEF");
							break;
					}
					break;
				case 's' :
					cp = va_arg(ap, char *);
					length = strlen(cp);
					break;
				case 'c' :
					c = (char)va_arg(ap, long);
					*dp++ = c;
					continue;
				default:
					*dp++ = '?';
			}
			pad = left_prec - length;
			if (sign != '\0')
			{
				pad--;
			}
			if (zero_fill)
			{
				c = '0';
				if (sign != '\0')
				{
					*dp++ = sign;
					sign = '\0';
				}
			}
			else
			{
				c = ' ';
			}
			if (!pad_on_right)
			{
				while (pad-- > 0)
				{
					*dp++ = c;
				}
			}
			if (sign != '\0')
			{
				*dp++ = sign;
			}
			while (length-- > 0)
			{
				c = *cp++;
				if (c == '\n')
				{
					*dp++ = '\r';
				}
				*dp++ = c;
			}
			if (pad_on_right)
			{
				while (pad-- > 0)
				{
					*dp++= ' ';
				}
			}
		}
		else
		{
			if (c == '\n')
			{
				*dp++= '\r';
			}
			*dp++ = c;
		}
	}

	*dp = '\0';

	return ((int)dp - (int)dest);
}

int usr_sprintf(char *buf, char const *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = usr_vsprintf(buf, fmt, ap);
	va_end(ap);

	return ret;
}
