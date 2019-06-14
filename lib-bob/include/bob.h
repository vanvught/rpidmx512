/**
 * @file bob.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BOB_H_
#define BOB_H_

#include <string.h>

#if defined(__linux__)
 #include "bcm2835.h"
#elif defined(H3)
 #include "h3_board.h"
 #include "h3_gpio.h"
 #include "h3_hs_timer.h"
 #include "h3_spi.h"

 #define bcm2835_aux_spi_begin()					(void)0
 #define bcm2835_aux_spi_setClockDivider(__p)		(void)0
 #define bcm2835_aux_spi_write(__p)					(void)0
 #define bcm2835_aux_spi_writenb(__p1,__p2)			(void)0
 #define bcm2835_aux_spi_transfern(__p1,__p2)		(void)0
 #define bcm2835_aux_spi_transfernb(__p1,__p2,__p3)	(void)0
 static inline uint32_t bcm2835_aux_spi_CalcClockDivider(uint32_t __p) { return 0;}
 //
 #define bcm2835_gpio_clr	h3_gpio_clr
 #define bcm2835_gpio_set	h3_gpio_set
 #define bcm2835_gpio_fsel	h3_gpio_fsel
 //
 #define BCM2835_GPIO_FSEL_OUTP	GPIO_FSEL_OUTPUT
 //
 #define bcm2835_delay(x)		udelay(x * 1000)
 //
#else
 #include "bcm2835.h"
 #include "bcm2835_spi.h"
 #include "bcm2835_aux_spi.h"
#endif

#include "i2c.h"

#if defined(__linux__) || defined(__circle__)
  #define udelay bcm2835_delayMicroseconds
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#if defined(H3)
 	#define FUNC_PREFIX(x) h3_##x
#else
 	#define FUNC_PREFIX(x) bcm2835_##x
#endif

#include "device_info.h"

#endif
