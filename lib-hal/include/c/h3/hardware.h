/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef H3_HARDWARE_H_
#define H3_HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>

#include "h3_hs_timer.h"
#include "h3_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);

inline static uint32_t hardware_micros(void) {
	return h3_hs_timer_lo_us();
}

inline static void hardware_watchdog_init(void) {
	h3_watchdog_enable();
}

inline static void hardware_watchdog_feed(void) {
	h3_watchdog_restart();
}

void hardware_watchdog_stop(void);

bool hardware_is_pwr_button_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* H3_HARDWARE_H_ */
