/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "displayset.h"

#if defined(ENABLE_TC1602)
# include "tc1602.h"
#endif
#include "ssd1306.h"
#if defined(ENABLE_SSD1311)
# include "ssd1311.h"
#endif

#include "display7segment.h"

#include "hal_i2c.h"

using namespace display;

Display *Display::s_pThis = nullptr;

Display::Display() :
	m_nMillis(Hardware::Get()->Millis()),
	m_I2C(display::segment7::I2C_ADDRESS)
{
	assert(s_pThis == nullptr);
	s_pThis = this;

#if defined(ENABLE_SSD1311)
	Detect(display::Type::SSD1311);
#endif

	if (m_LcdDisplay == nullptr) {
		Detect(display::Type::SSD1306);
	}

	Detect7Segment();

	if (m_LcdDisplay != nullptr) {
		timeout::gpio_init();
	}

	PrintInfo();
}

Display::Display(uint8_t nCols, uint8_t nRows) :
	m_nMillis(Hardware::Get()->Millis()),
	m_I2C(display::segment7::I2C_ADDRESS)
{
	assert(s_pThis == nullptr);
	s_pThis = this;

	Detect(nCols, nRows);

	Detect7Segment();

	if (m_LcdDisplay != nullptr) {
		timeout::gpio_init();
	}

	PrintInfo();
}

Display::Display(display::Type tDisplayType):
	m_tType(tDisplayType),
	m_nMillis(Hardware::Get()->Millis()),
	m_I2C(display::segment7::I2C_ADDRESS)
{
	assert(s_pThis == nullptr);
	s_pThis = this;

	Detect(tDisplayType);

	Detect7Segment();

	if (m_LcdDisplay != nullptr) {
		timeout::gpio_init();
	}

	PrintInfo();
}

void Display::Detect(display::Type tDisplayType) {
	switch (tDisplayType) {
#if defined(ENABLE_TC1602)
		case display::Type::PCF8574T_1602:
			m_LcdDisplay = new Tc1602(16, 2);
			assert(m_LcdDisplay != nullptr);
			break;
		case display::Type::PCF8574T_2004:
			m_LcdDisplay = new Tc1602(20, 4);
			assert(m_LcdDisplay != nullptr);
			break;
#endif
#if defined(ENABLE_SSD1311)
		case display::Type::SSD1311:
			m_LcdDisplay = new Ssd1311;
			assert(m_LcdDisplay != nullptr);
			break;
#endif
		case display::Type::SSD1306:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			assert(m_LcdDisplay != nullptr);
			break;
		case display::Type::UNKNOWN:
			m_tType = display::Type::UNKNOWN;
			/* no break */
		default:
			break;
	}

	if (m_LcdDisplay != nullptr) {
		if (!m_LcdDisplay->Start()) {
			delete m_LcdDisplay;
			m_LcdDisplay = nullptr;
			m_tType = display::Type::UNKNOWN;
		} else {
			m_LcdDisplay->Cls();
		}
	}

	if (m_LcdDisplay == nullptr){
		m_nSleepTimeout = 0;
	}
}

void Display::Detect(__attribute__((unused)) uint8_t nCols, uint8_t nRows) {
	if (HAL_I2C::IsConnected(OLED_I2C_SLAVE_ADDRESS_DEFAULT)) {
		if (nRows <= 4) {
#if defined(ENABLE_SSD1311)
			m_LcdDisplay = new Ssd1311;
			assert(m_LcdDisplay != nullptr);

			if (m_LcdDisplay->Start()) {
				m_tType = display::Type::SSD1311;
				Printf(1, "SSD1311");
			} else
#endif
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_4ROWS);
			assert(m_LcdDisplay != nullptr);
		} else {
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			assert(m_LcdDisplay != nullptr);
		}

		if (m_LcdDisplay->Start()) {
			m_tType = display::Type::SSD1306;
			Printf(1, "SSD1306");
		}
	}
#if defined(ENABLE_TC1602)
	else if (HAL_I2C::IsConnected(TC1602_I2C_DEFAULT_SLAVE_ADDRESS)) {
		m_LcdDisplay = new Tc1602(nCols, nRows);
		assert(m_LcdDisplay != nullptr);

		if (m_LcdDisplay->Start()) {
			m_tType = display::Type::PCF8574T_1602;
			Printf(1, "TC1602_PCF8574T");
		}
	}
#endif

	if (m_LcdDisplay == nullptr) {
		m_nSleepTimeout = 0;
	}
}

