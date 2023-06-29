/**
 * @file st7789.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ST7789_H
#define ST7789_H

#include <cstdint>

#include "spi/config.h"
#include "spi/st77xx.h"

namespace st7789 {
#if defined (SPI_LCD_240X240)
static constexpr auto ROTATION_0_SHIFT_X =  0;
static constexpr auto ROTATION_0_SHIFT_Y = 80;
static constexpr auto ROTATION_1_SHIFT_X = 80;
static constexpr auto ROTATION_1_SHIFT_Y =  0;
static constexpr auto ROTATION_2_SHIFT_X =  0;
static constexpr auto ROTATION_2_SHIFT_Y =  0;
static constexpr auto ROTATION_3_SHIFT_X =  0;
static constexpr auto ROTATION_3_SHIFT_Y =  0;
#elif defined (SPI_LCD_240X320)
static constexpr auto ROTATION_0_SHIFT_X =  0;
static constexpr auto ROTATION_0_SHIFT_Y =  0;
static constexpr auto ROTATION_1_SHIFT_X =  0;
static constexpr auto ROTATION_1_SHIFT_Y =  0;
static constexpr auto ROTATION_2_SHIFT_X =  0;
static constexpr auto ROTATION_2_SHIFT_Y =  0;
static constexpr auto ROTATION_3_SHIFT_X =  0;
static constexpr auto ROTATION_3_SHIFT_Y =  0;
#endif
}  // namespace st7789

class ST7789 : public ST77XX {
public:
	ST7789();
	~ST7789() override;

	void Init();

	void SetRotation(uint32_t nRotation);
};

#endif /* ST7789_H */
