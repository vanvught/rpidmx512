/**
 * @file hardware.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <bcm2835.h>
#include "bcm2835_led.h"
#include "bcm2835_wdog.h"

#include "sys_time.h"

extern void fb_init(void);

static volatile uint64_t hardware_init_startup_micros = 0;

void hardware_init(void)
{
	hardware_init_startup_micros = bcm2835_st_read();
	sys_time_init();
	fb_init();
	led_init();
	led_set(1);
}

void hardware_reboot(void)
{
	watchdog_init();
	for(;;);
}

uint64_t hardware_uptime_seconds(void)
{
	return (((bcm2835_st_read() - hardware_init_startup_micros) * 0x431bde83) >> (0x12 + 32));
}
