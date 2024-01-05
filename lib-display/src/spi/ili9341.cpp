/**
 * @file ili9341.cpp
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

#include "spi/ili9341.h"
#include "spi/spi_lcd.h"

#include "hal_gpio.h"

#include "debug.h"

using namespace spi::lcd;

ILI9341::ILI9341() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

ILI9341::~ILI9341() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ILI9341::Init() {
	DEBUG_ENTRY

#if defined(SPI_LCD_RST_GPIO)
	HW_Reset();
#endif

	Write_Command(0xC0);    //Power control
	WriteData_Byte(0x23);   //VRH[5:0]

	Write_Command(0xC1);    //Power control
	WriteData_Byte(0x10);   //SAP[2:0];BT[3:0]

	Write_Command(0xC5);    //VCM control
	WriteData_Byte(0x3e); //
	WriteData_Byte(0x28);

	Write_Command(0xC7);    //VCM control2
	WriteData_Byte(0x86);  //--

	Write_Command(0x3A);
	WriteData_Byte(0x55);

	Write_Command(0xB1);
	WriteData_Byte(0x00);
	WriteData_Byte(0x18);

	Write_Command(0xB6);    // Display Function Control
	WriteData_Byte(0x08);
	WriteData_Byte(0xA2);
	WriteData_Byte(0x27);

	Write_Command(0xF2);    // 3Gamma Function Disable
	WriteData_Byte(0x00);

	Write_Command(0x26);    //Gamma curve selected
	WriteData_Byte(0x01);

	Write_Command(0xE0);    //Set Gamma
	WriteData_Byte(0x0F);
	WriteData_Byte(0x31);
	WriteData_Byte(0x2B);
	WriteData_Byte(0x0C);
	WriteData_Byte(0x0E);
	WriteData_Byte(0x08);
	WriteData_Byte(0x4E);
	WriteData_Byte(0xF1);
	WriteData_Byte(0x37);
	WriteData_Byte(0x07);
	WriteData_Byte(0x10);
	WriteData_Byte(0x03);
	WriteData_Byte(0x0E);
	WriteData_Byte(0x09);
	WriteData_Byte(0x00);

	Write_Command(0XE1);    //Set Gamma
	WriteData_Byte(0x00);
	WriteData_Byte(0x0E);
	WriteData_Byte(0x14);
	WriteData_Byte(0x03);
	WriteData_Byte(0x11);
	WriteData_Byte(0x07);
	WriteData_Byte(0x31);
	WriteData_Byte(0xC1);
	WriteData_Byte(0x48);
	WriteData_Byte(0x08);
	WriteData_Byte(0x0F);
	WriteData_Byte(0x0C);
	WriteData_Byte(0x31);
	WriteData_Byte(0x36);
	WriteData_Byte(0x0F);

	SetRotation(0);

	Write_Command(0x11);    //Exit Sleep
	Write_Command(0x29);    //Display on

	DEBUG_EXIT
}

void ILI9341::SetRotation(uint32_t nRotation) {
	Write_Command(ili9341::cmd::MADCTL);

	switch (nRotation) {
	case 0:
		WriteData_Byte(ili9341::data::MADCTL_BGR);
		m_nWidth = config::WIDTH;
		m_nHeight = config::HEIGHT;
		break;
	case 1:
		WriteData_Byte(ili9341::data::MADCTL_MX | ili9341::data::MADCTL_MV | ili9341::data::MADCTL_BGR);
		m_nWidth = config::HEIGHT;
		m_nHeight = config::WIDTH;
		break;
	case 2:
		WriteData_Byte(ili9341::data::MADCTL_MX | ili9341::data::MADCTL_MY | ili9341::data::MADCTL_BGR);
		m_nWidth = config::WIDTH;
		m_nHeight = config::HEIGHT;
		break;
	case 3:
		WriteData_Byte(ili9341::data::MADCTL_MY | ili9341::data::MADCTL_MV | ili9341::data::MADCTL_BGR);
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

void ILI9341::SetBackLight(uint32_t nValue) {
	FUNC_PREFIX(gpio_write(SPI_LCD_BL_GPIO, nValue == 0 ? LOW : HIGH));
}

void ILI9341::EnableDisplay(bool bEnable) {
	Write_Command(bEnable ? ili9341::cmd::DISPON : ili9341::cmd::DISPOFF);
}

void ILI9341::EnableSleep(bool bEnable) {
	Write_Command(bEnable ? ili9341::cmd::SLPIN : ili9341::cmd::SLPOUT);
}

void ILI9341::EnableColourInversion(bool bEnable) {
	Write_Command(bEnable ? ili9341::cmd::INVON : ili9341::cmd::INVOFF);
}

void ILI9341::SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	Write_Command(ili9341::cmd::CASET);
	{
		uint8_t data[] = { static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0), static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1) };
		WriteData(data, sizeof(data));
	}

	Write_Command(ili9341::cmd::RASET);
	{
		uint8_t data[] = { static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0), static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1) };
		WriteData(data, sizeof(data));
	}

	Write_Command(ili9341::cmd::RAMWR);
}
