/**
 * @file panel_led
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_PANEL_LED_H_
#define GD32_PANEL_LED_H_

#include "gd32_bitbanging595.h"

namespace hal {
#if defined(BOARD_GD32F207VC) || defined(BOARD_GD32F450VE)
inline static void panel_led_on(uint32_t on) {
	const uint32_t nDMX = on & 0xFFFF;

	if (nDMX != 0) {
		const uint32_t nShift =  (__CLZ(nDMX) - 23) * 2;
		on &= ~(0xFFFF);
		on |= (1 << nShift);
	}

	BitBanging595::Get()->SetOn(on);
}

inline static void panel_led_off(uint32_t off) {
	const uint32_t nDMX = off & 0xFFFF;

	if (nDMX != 0) {
		const uint32_t nShift =  (__CLZ(nDMX) - 23) * 2;
		off &= ~(0xFFFF);
		off |= (1 << nShift);

	}

	BitBanging595::Get()->SetOff(off);
}
#else
inline static void panel_led_on(uint32_t __attribute__((unused)) on) {
}

inline static void panel_led_off(uint32_t __attribute__((unused)) off) {
}
#endif
}  // namespace hal

#endif /* GD32_PANEL_LED_H_ */
