/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "display.h"
#include "hardware.h"

#include "hal_spi.h"
#include "hal_gpio.h"

#include "spi/lcd_font.h"

#include "debug.h"

#if defined (SPI_LCD_240X320)
static constexpr sFONT *s_pFONT = &Font16x24;
#else
static constexpr sFONT *s_pFONT = &Font12x12;
#endif

static constexpr auto COLOR_BACKGROUND = 0x001F;
static constexpr auto COLOR_FOREGROUND = 0xFFE0;

Display *Display::s_pThis;

Display::Display() : m_nMillis(Hardware::Get()->Millis()) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(20000000));
	FUNC_PREFIX(spi_setDataMode(SPI_MODE0));

#if defined (SPI_LCD_RST_GPIO)
	FUNC_PREFIX(gpio_fsel(SPI_LCD_RST_GPIO, GPIO_FSEL_OUTPUT));
#endif
	FUNC_PREFIX(gpio_fsel(SPI_LCD_DC_GPIO, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(SPI_LCD_BL_GPIO, GPIO_FSEL_OUTPUT));
#if defined(SPI_LCD_HAVE_CS_GPIO)
	FUNC_PREFIX(gpio_fsel(SPI_LCD_CS_GPIO, GPIO_FSEL_OUTPUT));
#endif

	SpiLcd.SetBackLight(1);
	SpiLcd.Init();
	SetFlipVertically(false);
	SpiLcd.FillColour(COLOR_BACKGROUND);

	m_nCols = static_cast<uint8_t>(SpiLcd.GetWidth() / s_pFONT->Width);
	m_nRows = static_cast<uint8_t>(SpiLcd.GetHeight() / s_pFONT->Height);

#if defined (DISPLAYTIMEOUT_GPIO)
	FUNC_PREFIX(gpio_fsel(DISPLAYTIMEOUT_GPIO, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_set_pud(DISPLAYTIMEOUT_GPIO, GPIO_PULL_UP));
#endif

	PrintInfo();
	DEBUG_EXIT
}

void Display::Cls() {
	SpiLcd.FillColour(COLOR_BACKGROUND);
}

void Display::SetCursorPos(uint32_t nCol, uint32_t nRow) {
	m_nCursorX = static_cast<uint16_t>(nCol * s_pFONT->Width);
	m_nCursorY = static_cast<uint16_t>(nRow * s_pFONT->Height);
}

void Display::PutChar(int c) {
	SpiLcd.DrawChar(m_nCursorX, m_nCursorY, static_cast<char>(c), s_pFONT, COLOR_BACKGROUND, COLOR_FOREGROUND);

	m_nCursorX += s_pFONT->Width;

	if (m_nCursorX >= SpiLcd.GetWidth()) {
		m_nCursorX = 0;

		m_nCursorY += s_pFONT->Height;

		if (m_nCursorY >= SpiLcd.GetHeight()) {
			m_nCursorY = 0;
		}
	}
}
