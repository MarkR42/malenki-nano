#include <avr/io.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// Write a string to the USART0.
// This assumes the hardware is already configured.
void diag_puts(const char *str)
{
	const char *p=str;
	while (*p != 0) {
		// Wait for uart0 to be un-busy.
		while (! (USART0.STATUS & USART_DREIF_bm)) {
			// Sleep
		}
		USART0.TXDATAL = *p;
		++p ;
	}
}

#define DIAG_BUFSIZE 80
void diag_printf(const char * fmt, ...)
{
	static char buf[DIAG_BUFSIZE];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	buf[DIAG_BUFSIZE -1] = '\0'; // ensure null terminated
	diag_puts(buf);
}
