/**
 * @file led.h
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LED_H_
#define LED_H_

#include <stdint.h>

#define LED_BLINK_NORMAL	(uint32_t) (1E6 / 2)	///< 1Hz
#define LED_BLINK_IDENTIFY	(uint32_t) (1E6 / 4)	///< 2Hz
#define LED_BLINK_DMX_DATA	(uint32_t) (1E6 / 6)	///< 3Hz
#define LED_BLINK_RDM_DATA	(uint32_t) (1E6 / 8)	///< 4Hz

#ifdef __cplusplus
extern "C" {
#endif

extern void led_set_ticks_per_second(uint32_t);
extern uint32_t ticks_per_second_get(void);

extern void led_blink(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_H_ */
