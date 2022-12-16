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

#include <cstdint>
#include <cassert>

#include "gd32_bitbanging595.h"

namespace hal {
#if defined(BOARD_GD32F207VC) || defined(BOARD_GD32F450VE)

static constexpr uint32_t led_map[16] = {
		0x4000, 	// A_TX
		0x1000, 	// B_TX
		0x0400, 	// C_TX
		0x0100, 	// D_TX
		0x0080, 	// E_TX
		0x0020, 	// F_TX
		0x0008,		// G_TX
		0x0002,		// H_TX
		0x8000,		// A_RX
		0x2000,		// B_RX
		0x0800,		// C_RX
		0x0200,		// D_RX
		0x0040,		// E_RX
		0x0010,		// F_RX
		0x0004,		// G_RX
		0x0001		// H_RX
};

inline static void panel_led_on(uint32_t on) {
	const uint32_t nDMX = on & 0xFFFF;

	if (nDMX != 0) {
		on &= ~(0xFFFF);
		const auto nIndex =  31 - __CLZ(nDMX);
		assert(nIndex < 16);
		on |= (led_map[nIndex]);
	}

	BitBanging595::Get()->SetOn(on);
}

inline static void panel_led_off(uint32_t off) {
	const uint32_t nDMX = off & 0xFFFF;

	if (nDMX != 0) {
		off &= ~(0xFFFF);
		const auto nIndex = 31 - __CLZ(nDMX);
		assert(nIndex < 16);
		off |= (led_map[nIndex]);
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
