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

#include <bcm2835.h>

void led_set(int state) {
	bcm2835_gpio_write(16, !state);
}

void led_init(void) {
	bcm2835_gpio_fsel(16, BCM2835_GPIO_FSEL_OUTP);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

// this is derived from:

/*
 *  Broadcom BCM2708 watchdog driver.
 *
 *  (c) Copyright 2010 Broadcom Europe Ltd
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *      BCM2708 watchdog driver. Loosely based on wdt driver.
 */

//------------------------------------------------------------------------

#define BCM2708_PERI_BASE           0x20000000
#define PM_BASE                     (BCM2708_PERI_BASE + 0x100000) /* Power Management, Reset controller and Watchdog registers */
#define PM_RSTC                     (PM_BASE+0x1c)
#define PM_WDOG                     (PM_BASE+0x24)
#define PM_WDOG_RESET               0000000000
#define PM_PASSWORD                 0x5a000000
#define PM_WDOG_TIME_SET            0x000fffff
#define PM_RSTC_WRCFG_CLR           0xffffffcf
#define PM_RSTC_WRCFG_SET           0x00000030
#define PM_RSTC_WRCFG_FULL_RESET    0x00000020
#define PM_RSTC_RESET               0x00000102

//------------------------------------------------------------------------
static void inline wdog_start ( unsigned int timeout )
{
    unsigned int pm_rstc,pm_wdog;
    /* Setup watchdog for reset */
    pm_rstc = GET32(PM_RSTC);
    pm_wdog = PM_PASSWORD | (timeout & PM_WDOG_TIME_SET); // watchdog timer = timer clock / 16; need password (31:16) + value (11:0)
    pm_rstc = PM_PASSWORD | (pm_rstc & PM_RSTC_WRCFG_CLR) | PM_RSTC_WRCFG_FULL_RESET;
    PUT32(PM_WDOG,pm_wdog);
    PUT32(PM_RSTC,pm_rstc);
}
//------------------------------------------------------------------------
void wdog_stop ( void )
{
    PUT32(PM_RSTC,PM_PASSWORD | PM_RSTC_RESET);
}

#define WDOG_TIMEOUT 0x0FFFF

void watchdog_init(void) {
	wdog_start(WDOG_TIMEOUT);
}

void watchdog_feed(void) {
	wdog_start(WDOG_TIMEOUT);
}
