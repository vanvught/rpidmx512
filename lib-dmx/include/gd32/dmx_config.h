/**
 * @file dmx_config.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_DMX_CONFIG_H_
#define GD32_DMX_CONFIG_H_

#include "gd32.h"

namespace dmxmulti {
namespace config {
#if defined (BOARD_GD32F207C_EVAL)
# include "board_gd32f207c_eval.h"
#elif defined (BOARD_GD32F207R_ETH)
# include "board_gd32f207r_eth.h"
#elif defined (BOARD_GD32F103R)
# include "board_gd32f103r.h"
#else
# error
#endif
}  // namespace config
}  // namespace dmxmulti

namespace dmx {
namespace buffer {
static constexpr auto SIZE = 514;	// multiple of uint16_t
}  // namespace buffer
}  // namespace dmx

/**
 * PORTs check
 */
static_assert(DMX_MAX_PORTS <= dmxmulti::config::max::IN, "IN: DMX_MAX_PORTS");
static_assert(DMX_MAX_PORTS <= dmxmulti::config::max::OUT, "OUT: DMX_MAX_PORTS");

/**
 * DMA channel check
 */
#if defined (GD32F20X_CL)
# if defined (DMX_USE_UART4) && defined (DMX_USE_UART7)
# error DMA1 Channel 3
# endif
# if defined (DMX_USE_UART3) && defined (DMX_USE_UART6)
# error DMA1 Channel 4
# endif
#endif

#endif /* GD32_DMX_CONFIG_H_ */
