/*
 * core2.c
 *
 */

#include "h3.h"
#include "h3_hs_timer.h"

extern int smp_printf(const char *format, ...);

void core2(void) {
	for (;;) {
		smp_printf("Core 2 [%u]\n", h3_hs_timer_lo_us());
		udelay(1500000);
	}
}
