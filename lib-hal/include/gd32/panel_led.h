/**
 * @file panel_led.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "gd32.h"

#if !defined (PANELLED_595_COUNT)
# if !defined (UNUSED)
#  define UNUSED  __attribute__((unused))
# endif
#else
# if !defined (UNUSED)
#  define UNUSED
# endif
#endif

namespace hal {
namespace panelled {
extern uint32_t g_nData;
}  // namespace panelled

inline void panel_led_spi(const UNUSED uint32_t nData) {
#if defined(PANELLED_595_COUNT)
	GPIO_BC(PANELLED_595_CS_GPIOx) = PANELLED_595_CS_GPIO_PINx;

#if (PANELLED_595_COUNT >= 1)
	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_TBE))
		;

	SPI_DATA(SPI_PERIPH) = (nData & 0xFF);

	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_RBNE))
		;

	static_cast<void>(SPI_DATA(SPI_PERIPH));
#endif
#if (PANELLED_595_COUNT >= 2)
	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_TBE))
		;

	SPI_DATA(SPI_PERIPH) = ((nData >> 8) & 0xFF);

	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_RBNE))
		;

	static_cast<void>(SPI_DATA(SPI_PERIPH));
#endif
#if (PANELLED_595_COUNT >= 3)
	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_TBE))
		;

	SPI_DATA(SPI_PERIPH) = ((nData >> 16) & 0xFF);

	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_RBNE))
		;

	static_cast<void>(SPI_DATA(SPI_PERIPH));
#endif
#if (PANELLED_595_COUNT == 4)
	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_TBE))
		;

	SPI_DATA(SPI_PERIPH) = ((nData >> 24) & 0xFF);

	while (RESET == (SPI_STAT(SPI_PERIPH) & SPI_FLAG_RBNE))
		;

	static_cast<void>(SPI_DATA(SPI_PERIPH));
#endif

	GPIO_BOP(PANELLED_595_CS_GPIOx) = PANELLED_595_CS_GPIO_PINx;
#endif
}

inline void panel_led_init() {
#if defined(PANELLED_595_COUNT)

#endif
}

inline void panel_led_on(uint32_t UNUSED on) {
#if defined(PANELLED_595_COUNT)
	if (panelled::g_nData  == (panelled::g_nData | on)) {
		return;
	}

	panelled::g_nData |= on;

	panel_led_spi(panelled::g_nData);
#endif
}

inline void panel_led_off(uint32_t UNUSED off) {
#if defined(PANELLED_595_COUNT)
	if (panelled::g_nData  == (panelled::g_nData & ~off)) {
		return;
	}

	panelled::g_nData &= ~off;

	panel_led_spi(panelled::g_nData);
#endif
}

inline void panel_led_run() {
#if defined(PANELLED_595_COUNT)

#endif
}

}  // namespace hal

#endif /* GD32_PANEL_LED_H_ */
