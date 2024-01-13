/**
 * @file st77xx.cpp
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

#include <cstdint>
#include <cassert>

#include "spi/st77xx.h"
#include "spi/spi_lcd.h"

#include "hal_gpio.h"
#include "hal_spi.h"

#include "debug.h"

using namespace spi::lcd;

ST77XX::ST77XX() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

ST77XX::~ST77XX() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ST77XX::EnableDisplay(bool bEnable) {
	Write_Command(bEnable ? st77xx::cmd::DISPON : st77xx::cmd::DISPOFF);
}

void ST77XX::EnableSleep(bool bEnable) {
	Write_Command(bEnable ? st77xx::cmd::SLPIN : st77xx::cmd::SLPOUT);
}

//TODO This should be a PWM pin
void ST77XX::SetBackLight(uint32_t nValue) {
	FUNC_PREFIX(gpio_write(SPI_LCD_BL_GPIO, nValue == 0 ? LOW : HIGH));
}

void ST77XX::SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	const auto x_start = x0 + m_nShiftX;
	const auto x_end = x1 + m_nShiftX;
	const auto y_start = y0 + m_nShiftY;
	const auto y_end = y1 + m_nShiftY;

	Write_Command(st77xx::cmd::CASET);
	{
		uint8_t data[] = { static_cast<uint8_t>(x_start >> 8), static_cast<uint8_t>(x_start), static_cast<uint8_t>(x_end >> 8), static_cast<uint8_t>(x_end) };
		WriteData(data, sizeof(data));
	}

	Write_Command(st77xx::cmd::RASET);
	{
		uint8_t data[] = { static_cast<uint8_t>(y_start >> 8), static_cast<uint8_t>(y_start), static_cast<uint8_t>(y_end >> 8), static_cast<uint8_t>(y_end) };
		WriteData(data, sizeof(data));
	}

	Write_Command(st77xx::cmd::RAMWR);
}
