/**
 * @file logic_analyzer.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef BOARD_LOGIC_ANALYZER_H_
#define BOARD_LOGIC_ANALYZER_H_

#include "h3_board.h"

#if defined(ORANGE_PI_ONE)
#else
// PIXELDMXSTARTSTOP_GPIO	GPIO_EXT_12
// DISPLAYTIMEOUT_GPIO		GPIO_EXT_15
// EXTERNAL_LED 			GPIO_EXT_16
// SPI_CS1					GPIO_EXT_26
# define LOGIC_ANALYZER_CH0_GPIO_PINx	GPIO_EXT_22
# define LOGIC_ANALYZER_CH1_GPIO_PINx	GPIO_EXT_18
# define LOGIC_ANALYZER_CH2_GPIO_PINx	GPIO_EXT_16
//# define LOGIC_ANALYZER_CH3_GPIO_PINx	GPIO_EXT_7
#endif

#endif /* BOARD_LOGIC_ANALYZER_H_ */
