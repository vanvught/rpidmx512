/**
 * @file st7789.cpp
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

#include "spi/st7789.h"
#include "spi/st77xx.h"
#include "spi/spi_lcd.h"

#include "hal_gpio.h"

#include "debug.h"

using namespace spi::lcd;

ST7789::ST7789() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

ST7789::~ST7789() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ST7789::Init(void) {
	DEBUG_ENTRY

#if defined(SPI_LCD_RST_PIN)
	HW_Reset();
#endif

	Write_Command(st77xx::cmd::SWRESET);
	ms_delay(150);

	Write_Command(st77xx::cmd::COLMOD);
	WriteData_Byte(0x55);

	Write_Command(0xB7);
	WriteData_Byte(0x35);

	Write_Command(0xBB);
	WriteData_Byte(0x19);

	Write_Command(0xC0);
	WriteData_Byte(0x2C);

	Write_Command(0xC2);
	WriteData_Byte(0x01);

	Write_Command(0xC3);
	WriteData_Byte(0x12);

	Write_Command(0xC4);
	WriteData_Byte(0x20);

	Write_Command(0xC6);
	WriteData_Byte(0x0F);

	Write_Command(0xD0);
	WriteData_Byte(0xA4);
	WriteData_Byte(0xA1);

	SetRotation(0);

	Write_Command(st77xx::cmd::INVON);
	Write_Command(st77xx::cmd::SLPOUT);
	Write_Command(st77xx::cmd::DISPON);

	DEBUG_EXIT
}

void ST7789::SetRotation(uint32_t nRotation) {
	Write_Command(st77xx::cmd::MADCTL);

	switch (nRotation) {
	case 0:
		WriteData_Byte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MY | st77xx::data::MADCTL_RGB);
		m_nShiftX = st7789::ROTATION_0_SHIFT_X;
		m_nShiftY = st7789::ROTATION_0_SHIFT_Y;
		m_nWidth = config::WIDTH;
		m_nHeight = config::HEIGHT;
		break;
	case 1:
		WriteData_Byte(st77xx::data::MADCTL_MY | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_RGB);
		m_nShiftX = st7789::ROTATION_1_SHIFT_X;
		m_nShiftY = st7789::ROTATION_1_SHIFT_Y;
		m_nWidth = config::HEIGHT;
		m_nHeight = config::WIDTH;
		break;
	case 2:
		WriteData_Byte(st77xx::data::MADCTL_RGB);
		m_nShiftX = st7789::ROTATION_2_SHIFT_X;
		m_nShiftY = st7789::ROTATION_2_SHIFT_Y;
		m_nWidth = config::WIDTH;
		m_nHeight = config::HEIGHT;
		break;
	case 3:
		WriteData_Byte(st77xx::data::MADCTL_MX | st77xx::data::MADCTL_MV | st77xx::data::MADCTL_RGB);
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

	m_nRotate = static_cast<uint16_t>(nRotation);
}
