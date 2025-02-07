/**
 * @file st7735s.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * ST7735S
 * 132RGB x 162dot 262K Color with Frame Memory
 * Single-Chip TFT Controller/Driver
 * Datasheet
 * Version 1.1
 * 2011/11
 */

#ifndef SPI_ST7735S_H_
#define SPI_ST7735S_H_

#include <cstdint>
#include <cassert>

#include "spi/config.h"
#include "spi/st77xx.h"

#include "debug.h"

namespace st7735s {
namespace cmd {
static constexpr uint8_t INVCTR  = 0xB4;	///< Display Inversion Control
static constexpr uint8_t PWCTR1  = 0xC0;	///< Power Control 1
static constexpr uint8_t PWCTR2  = 0xC1;	///< Power Control 2
static constexpr uint8_t PWCTR3  = 0xC2;	///< Power Control 3 in normal mode, full colors
static constexpr uint8_t PWCTR4  = 0xC3;	///< Power Control 4 in idle mode 8colors
static constexpr uint8_t PWCTR5  = 0xC4;	///< Power Control 5 in partial mode, full colors
static constexpr uint8_t GMCTRP1 = 0xE0;	///< Gamma (‘+’polarity) Correction Characteristics Setting
static constexpr uint8_t GMCTRN1 = 0xE1;	///< Gamma ‘-’polarity Correction Characteristics Setting
}  // namespace cmd
#if defined (SPI_LCD_128X128)
 static constexpr uint32_t ROTATION_0_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_0_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_1_SHIFT_X =  3;
 static constexpr uint32_t ROTATION_1_SHIFT_Y =  2;
 static constexpr uint32_t ROTATION_2_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_X =  1;
 static constexpr uint32_t ROTATION_3_SHIFT_Y =  1;
#elif defined (SPI_LCD_160X80)
 static constexpr uint32_t ROTATION_0_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_0_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_1_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_1_SHIFT_Y =  24;
 static constexpr uint32_t ROTATION_2_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_Y =  24;
#endif
}  // namespace st7735s

class ST7735S : public ST77XX {
public:
	ST7735S(uint32_t nCS) : ST77XX(nCS) {
		DEBUG_ENTRY

		if (m_nInstance == 0) {
		HardwareReset();
		}

		m_nInstance++;

		WriteCommand(st77xx::cmd::SWRESET);
		udelay(1000 * 150);

		static constexpr uint8_t config[] = {
			    1, st77xx::cmd::COLMOD, 0x05,	///< Page 150, 5 -> 16-bit/pixel
				1, st77xx::cmd::GAMSET, 0x08,	///< Page 125, 8 -> Gamma Curve 4
				1, st7735s::cmd::INVCTR, 0x01,	///< Page 162, Display Inversion mode control -> 3-bit,  0=Dot, 1=Column
			   16, st7735s::cmd::GMCTRP1,0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2c,
			                0x29, 0x25, 0x2b, 0x39, 0x00, 0x01, 0x03, 0x10,
			   16, st7735s::cmd::GMCTRN1,0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2c,
			                0x2e, 0x2e, 0x37, 0x3f, 0x00, 0x00, 0x02, 0x10,
				0, st77xx::cmd::INVOFF,			///< Page 124, Enter into display inversion mode
			    0, st77xx::cmd::IDMOFF,			///< Page 147, Recover from Idle mode on
				0, st77xx::cmd::NORON			///< Page 122, Normal Display mode on
		};

		uint32_t nArgLength = 0;

		for (uint32_t i = 0; i < sizeof(config); i+=(nArgLength + 2)) {
			nArgLength = config[i];
			DEBUG_PRINTF("i=%u, nArgLength=%u", i, nArgLength);
			WriteCommand(&config[i + 1], nArgLength);
		}

		SetRotation(0);

		WriteCommand(st77xx::cmd::SLPOUT);	///< Sleep Out
		WriteCommand(st77xx::cmd::DISPON);	///< Display On

		DEBUG_EXIT
	}

	~ST7735S() override {};

	void SetRotation(const uint32_t nRotation) {
		DEBUG_ENTRY
		DEBUG_PRINTF("nRotation=%u", nRotation);

		WriteCommand(st77xx::cmd::MADCTL);

		switch (nRotation) {
		case 0:
			WriteDataByte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MY | st77xx::data::MADCTL_BGR);
			m_nShiftX = st7735s::ROTATION_0_SHIFT_X;
			m_nShiftY = st7735s::ROTATION_0_SHIFT_Y;
			m_nWidth = config::WIDTH;
			m_nHeight = config::HEIGHT;
			break;
		case 1:
			WriteDataByte(st77xx::data::MADCTL_MY | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_BGR);
			m_nShiftX = st7735s::ROTATION_1_SHIFT_X;
			m_nShiftY = st7735s::ROTATION_1_SHIFT_Y;
			m_nWidth = config::HEIGHT;
			m_nHeight = config::WIDTH;
			break;
		case 2:
			WriteDataByte(st77xx::data::MADCTL_BGR);
			m_nShiftX = st7735s::ROTATION_2_SHIFT_X;
			m_nShiftY = st7735s::ROTATION_2_SHIFT_Y;
			m_nWidth = config::WIDTH;
			m_nHeight = config::HEIGHT;
			break;
		case 3:
			WriteDataByte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_BGR);
			m_nShiftX = st7735s::ROTATION_3_SHIFT_X;
			m_nShiftY = st7735s::ROTATION_3_SHIFT_Y;
			m_nWidth = config::HEIGHT;
			m_nHeight = config::WIDTH;
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}

		m_nRotate = nRotation;

		DEBUG_EXIT
	}

private:
	static inline uint32_t m_nInstance;
};

#endif /* SPI_ST7735S_H_ */
