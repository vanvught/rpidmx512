/**
 * @file st7789.h
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

#ifndef SPI_ST7789_H
#define SPI_ST7789_H

#include <cstdint>
#include <cassert>

#include "spi/config.h"
#include "spi/st77xx.h"

#include "debug.h"

namespace st7789 {
namespace cmd {
static constexpr uint8_t GCTRL    = 0xB7;	///< Gate Control
static constexpr uint8_t VCOMS    = 0xBB;	///< VCOM Setting
static constexpr uint8_t LCMCTRL  = 0xC0;	///< LCM Control
static constexpr uint8_t VDVVRHEN = 0xC2;	///< VDV and VRH Command Enable
static constexpr uint8_t VRHS     = 0xC3;	///< VRH Set
static constexpr uint8_t VDVS     = 0xC4;	///< VDV Set
static constexpr uint8_t FRCTRL2  = 0xC6;	///< Frame Rate Control in Normal Mode
static constexpr uint8_t PWCTRL1  = 0xD0;	///< Power Control 1
}  // namespace cmd
#if defined (SPI_LCD_240X240)
 static constexpr uint32_t ROTATION_0_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_0_SHIFT_Y = 80;
 static constexpr uint32_t ROTATION_1_SHIFT_X = 80;
 static constexpr uint32_t ROTATION_1_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_Y =  0;
#elif defined (SPI_LCD_240X320)
 static constexpr uint32_t ROTATION_0_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_0_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_1_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_1_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_2_SHIFT_Y =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_X =  0;
 static constexpr uint32_t ROTATION_3_SHIFT_Y =  0;
#endif
}  // namespace st7789

class ST7789 : public ST77XX {
public:
	ST7789(uint32_t nCS) : ST77XX(nCS) {
		DEBUG_ENTRY

#if defined(SPI_LCD_RST_GPIO)
		if (m_nInstance == 0) {
			HardwareReset();
		}
		m_nInstance++;
#endif

		WriteCommand(st77xx::cmd::SWRESET);
		udelay(1000 * 150);

		static constexpr uint8_t config[] = {
				1, st77xx::cmd::COLMOD, 0x55,
				1, st7789::cmd::GCTRL, 0x35,
				1, st7789::cmd::VCOMS, 0x19,
				1, st7789::cmd::LCMCTRL, 0x2C,
				1, st7789::cmd::VDVVRHEN, 0x01,
				1, st7789::cmd::VRHS, 0x12,
				1, st7789::cmd::VDVS, 0x20,
				1, st7789::cmd::FRCTRL2, 0x0F,
				2, st7789::cmd::PWCTRL1, 0xA4, 0xA1,
				0, st77xx::cmd::NORON,
				0, st77xx::cmd::INVON
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

	~ST7789() override {
		DEBUG_ENTRY
		DEBUG_EXIT
	};

	void SetRotation(const uint32_t nRotation) {
		WriteCommand(st77xx::cmd::MADCTL);

		switch (nRotation) {
		case 0:
			WriteDataByte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MY | st77xx::data::MADCTL_RGB);
			m_nShiftX = st7789::ROTATION_0_SHIFT_X;
			m_nShiftY = st7789::ROTATION_0_SHIFT_Y;
			m_nWidth = config::WIDTH;
			m_nHeight = config::HEIGHT;
			break;
		case 1:
			WriteDataByte(st77xx::data::MADCTL_MY | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_RGB);
			m_nShiftX = st7789::ROTATION_1_SHIFT_X;
			m_nShiftY = st7789::ROTATION_1_SHIFT_Y;
			m_nWidth = config::HEIGHT;
			m_nHeight = config::WIDTH;
			break;
		case 2:
			WriteDataByte(st77xx::data::MADCTL_RGB);
			m_nShiftX = st7789::ROTATION_2_SHIFT_X;
			m_nShiftY = st7789::ROTATION_2_SHIFT_Y;
			m_nWidth = config::WIDTH;
			m_nHeight = config::HEIGHT;
			break;
		case 3:
			WriteDataByte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_RGB);
			m_nShiftX = st7789::ROTATION_3_SHIFT_X;
			m_nShiftY = st7789::ROTATION_3_SHIFT_Y;
			m_nWidth = config::HEIGHT;
			m_nHeight = config::WIDTH;
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}

		m_nRotate = nRotation;
	}

private:
#if defined(SPI_LCD_RST_GPIO)
	static inline uint32_t m_nInstance;
#endif
};

#endif /* SPI_ST7789_H */
