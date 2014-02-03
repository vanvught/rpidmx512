/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <bcm2835.h>
#include <bcm2835_vc.h>

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

extern void fb_init(void);

extern void cpu_info(void);
extern void mem_info(void);

int notmain(uint32_t boot_dev, uint32_t arm_m_type, uint32_t atags)
{
	fb_init();

    printf("Compiled on %s at %s\n\n", __DATE__, __TIME__);

    uint64_t ts = bcm2835_st_read();

    mem_info();

    printf("\n%ld usec elapsed\n\n", (long int)(bcm2835_st_read() - ts));

	cpu_info();

	printf("\n");

	printf("EMMC Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_EMMC));
	printf("UART Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_UART));
	printf("ARM  Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_ARM));
	printf("CORE Clock rate (Hz): %ld\n", bcm2835_vc_get_clock_rate(BCM2835_MAILBOX_CLOCK_ID_CORE));

    printf("\nProgram end\n");

    return 0;
}
