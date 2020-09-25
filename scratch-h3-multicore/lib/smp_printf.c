/*
 * smp_printf.c
 */

#include <stdio.h>
#include <stdarg.h>

#include "arm/spinlock.h"

unsigned int output_mutex = spin_unlocked;

int smp_printf(const char *format, ...) {
    /* Wait until the output spin lock is acquired */
	spin_lock(&output_mutex);

	va_list arp;

	va_start(arp, format);

	int i = printf(format, arp);

	va_end(arp);

    /* Leave critical section - release output spin lock */
	spin_unlock(&output_mutex);

	return i;
}
