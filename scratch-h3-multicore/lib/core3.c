/*
 * core3.c
 *
 */

#include "h3.h"
#include "h3_hs_timer.h"

extern int smp_printf(const char *format, ...);

void core3(void) {
	for (;;) {
		smp_printf("Core 3 [%u]\n", h3_hs_timer_lo_us());
		udelay(3000000);
	}
}
