/*
 * smp_printf.c
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdio.h>
#include <stdarg.h>

#include "arm/spinlock.h"
#include "h3.h"

unsigned int output_mutex = spin_unlocked;

int smp_printf(const char *format, ...) {
    /* Wait until the output spin lock is acquired */
	spin_lock(&output_mutex);

#if 0
	va_list arp;

	va_start(arp, format);

	int i = printf(format, arp);

	va_end(arp);
#else
	volatile uint32_t nGPIO = H3_PIO_PORTA->DAT;
	H3_PIO_PORTA->DAT = nGPIO;
#endif

    /* Leave critical section - release output spin lock */
	spin_unlock(&output_mutex);

	return 0;
}
