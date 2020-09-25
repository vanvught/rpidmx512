/*
 * smp_printf.c
 */


#include <stdio.h>
#include <stdarg.h>

#include "h3_spinlock.h"

int smp_printf(const char *format, ...) {
	h3_spinlock_lock(1);

	va_list arp;

	va_start(arp, format);

	int i = printf(format, arp);

	va_end(arp);

	h3_spinlock_unlock(1);

	return i;
}
