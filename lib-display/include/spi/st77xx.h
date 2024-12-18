/**
 * @file st77xx.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_ST77XX_H_
#define SPI_ST77XX_H_

#include <cstdint>

#include "spi/paint.h"
#include "spi/spilcd.h"

#include "hal_gpio.h"

#include "debug.h"

namespace st77xx {
namespace cmd {
static constexpr uint8_t NOP 	 = 0x00;
static constexpr uint8_t SWRESET = 0x01;	///< Software Reset
static constexpr uint8_t RDDID	 = 0x04;	///< Read Display ID
static constexpr uint8_t RDDST	 = 0x09;	///< Read Display Status
static constexpr uint8_t SLPIN	 = 0x10;	///< Sleep In
static constexpr uint8_t SLPOUT	 = 0x11;	///< Sleep Out
static constexpr uint8_t PTLON	 = 0x12;	///< Partial Display Mode On
static constexpr uint8_t NORON	 = 0x13;	///< Normal Display Mode On
static constexpr uint8_t INVOFF  = 0x20;	///< Display Inversion Off
static constexpr uint8_t INVON	 = 0x21;	///< Display Inversion On
static constexpr uint8_t GAMSET  = 0x26;	///< Gamma Set
static constexpr uint8_t DISPOFF = 0x28;	///< Display Offs
static constexpr uint8_t DISPON	 = 0x29;	///< Display On
static constexpr uint8_t CASET	 = 0x2A;	///< Column Address Set
static constexpr uint8_t RASET	 = 0x2B;	///< Row Address Set
static constexpr uint8_t RAMWR	 = 0x2C;	///< Memory Write
static constexpr uint8_t RAMRD	 = 0x2E;	///< Memory Read
static constexpr uint8_t PTLAR	 = 0x30;	///< Partial Area
static constexpr uint8_t TEOFF	 = 0x34;	///< Tearing Effect Line OFF
static constexpr uint8_t TEON	 = 0x35;	///< Tearing Effect Line ON .
static constexpr uint8_t MADCTL	 = 0x36;	///< Memory Data Access Control.
static constexpr uint8_t IDMOFF  = 0x38;	///< Idle Mode Off .
static constexpr uint8_t COLMOD	 = 0x3A;	///< Interface Pixel Format
}  // namespace cmd
namespace data {
/**
 * Memory Data Access Control Register (0x36H)
 * MAP:     D7  D6  D5  D4  D3  D2  D1  D0
 * param:   MY  MX  MV  ML  RGB MH  -   -
 *
 */
/* Page Address Order ('0': Top to Bottom, '1': the opposite) */
static constexpr uint8_t MADCTL_MY	= 0x80;
/* Column Address Order ('0': Left to Right, '1': the opposite) */
static constexpr uint8_t MADCTL_MX	= 0x40;
/* Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode) */
static constexpr uint8_t MADCTL_MV	= 0x20;
/* Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite) */
static constexpr uint8_t MADCTL_ML	= 0x10;
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
static constexpr uint8_t MADCTL_RGB	= 0x00;
static constexpr uint8_t MADCTL_BGR = 0x08;
}  // namespace data

namespace colour {
static constexpr uint16_t BLACK		= 0x0000;
static constexpr uint16_t BLUE		= 0x001F;
static constexpr uint16_t CYAN		= 0x07FF;
static constexpr uint16_t DARKBLUE	= 0X01CF;
static constexpr uint16_t GRAY		= 0X8430;
static constexpr uint16_t GREEN		= 0x07E0;
static constexpr uint16_t MAGENTA	= 0xF81F;
static constexpr uint16_t ORANGE	= 0xFC00;
static constexpr uint16_t RED		= 0xF800;
static constexpr uint16_t WHITE		= 0xFFFF;
static constexpr uint16_t YELLOW	= 0xFFE0;
}  // namespace colour

}  // namespace st77xx

class ST77XX : public Paint {
public:
	ST77XX(uint32_t nCS) : Paint(nCS) {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	~ST77XX() override {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void EnableDisplay(const bool bEnable) {
		WriteCommand(bEnable ? st77xx::cmd::DISPON : st77xx::cmd::DISPOFF);
	}

	void EnableSleep(const bool bEnable) {
		WriteCommand(bEnable ? st77xx::cmd::SLPIN : st77xx::cmd::SLPOUT);
	}

	void SetBackLight(const uint32_t nValue) {
		FUNC_PREFIX(gpio_write(SPI_LCD_BL_GPIO, nValue == 0 ? 0 : 1));
	}

	void SetAddressWindow(const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1) override {
		const auto nStartX = x0 + m_nShiftX;
		const auto nEndX = x1 + m_nShiftX;
		const auto nStartY = y0 + m_nShiftY;
		const auto nEndY = y1 + m_nShiftY;

		WriteCommand(st77xx::cmd::CASET);
		{
			uint8_t data[] = { static_cast<uint8_t>(nStartX >> 8), static_cast<uint8_t>(nStartX), static_cast<uint8_t>(nEndX >> 8), static_cast<uint8_t>(nEndX) };
			WriteData(data, sizeof(data));
		}

		WriteCommand(st77xx::cmd::RASET);
		{
			uint8_t data[] = { static_cast<uint8_t>(nStartY >> 8), static_cast<uint8_t>(nStartY), static_cast<uint8_t>(nEndY >> 8), static_cast<uint8_t>(nEndY) };
			WriteData(data, sizeof(data));
		}

		WriteCommand(st77xx::cmd::RAMWR);
	}

protected:
	uint32_t m_nShiftX { 0 };
	uint32_t m_nShiftY { 0 };
};

#endif /* SPI_ST77XX_H_ */
