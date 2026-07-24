/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3.h"
#include "h3_board.h"
#include "h3_gpio.h"
#include "firmware/debug/debug_debug.h"

#ifdef DEBUG_DISPLAY
#define DISPLAY_DEBUG_ENTRY() DEBUG_ENTRY()
#define DISPLAY_DEBUG_EXIT() DEBUG_EXIT()
#define DISPLAY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define DISPLAY_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define DISPLAY_DEBUG_ENTRY() \
    do {                      \
    } while (false)
#define DISPLAY_DEBUG_EXIT() \
    do {                     \
    } while (false)
#define DISPLAY_DEBUG_PRINTF(...) \
    do {                          \
    } while (false)
#define DISPLAY_DEBUG_PUTS(...) \
    do {                        \
    } while (false)
#endif

namespace display::timeout {
#define GPIO_PORTx (H3_GPIO_TO_PORT(DISPLAYTIMEOUT_GPIO))
#define INT_MASK (1U << H3_GPIO_TO_NUMBER(DISPLAYTIMEOUT_GPIO))

void irq_init() {
    DISPLAY_DEBUG_ENTRY();
    DISPLAY_DEBUG_PRINTF("GPIO_PORTx=%u, INT_MASK=0x%x", GPIO_PORTx, INT_MASK);
#if 0
		H3GpioFsel(DISPLAYTIMEOUT_GPIO, GPIO_FSEL_EINT);
		H3GpioIntCfg(DISPLAYTIMEOUT_GPIO, GPIO_INT_CFG_NEG_EDGE);

		if constexpr (GPIO_PORTx == H3_GPIO_PORTA) {
			H3_PIO_PA_INT->STA = INT_MASK;
			H3_PIO_PA_INT->CTL |= INT_MASK;
			H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7U << 4);
		} else if constexpr (GPIO_PORTx == H3_GPIO_PORTG) {
			H3_PIO_PG_INT->STA = INT_MASK;
			H3_PIO_PG_INT->CTL |= INT_MASK;
			H3_PIO_PG_INT->DEB = (0x0 << 0) | (0x7U << 4);
		} else {
			static_assert("IRQ is not available");
		}
#endif
    DISPLAY_DEBUG_EXIT();
}
} // namespace display::timeout
