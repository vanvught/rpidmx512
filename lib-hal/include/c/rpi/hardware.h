/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RPI_HARDWARE_H_
#define RPI_HARDWARE_H_

#include <stdint.h>

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"
#include "bcm2835_rng.h"

#ifdef __cplusplus
extern "C" {
#endif

/*@unused@*/inline static uint32_t hardware_micros(void) {
	return BCM2835_ST->CLO;
}

/*@unused@*/inline static int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address) {
	return bcm2835_vc_get_board_mac_address(mac_address);
}

/*@unused@*/inline static void hardware_watchdog_init(void) {
	bcm2835_watchdog_init();
}

/*@unused@*/inline static void hardware_watchdog_feed(void) {
	bcm2835_watchdog_feed();
}

/*@unused@*/inline static void hardware_watchdog_stop(void) {
	bcm2835_watchdog_stop();
}

#ifdef __cplusplus
}
#endif

#endif /* RPI_HARDWARE_H_ */
