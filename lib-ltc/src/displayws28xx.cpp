/**
 * @file displayws28xx.cpp
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <assert.h>

//#include "max72197segment.h"
//#include "max7219matrix.h"

#include "displayws28xx.h"


DisplayWS28xx *DisplayWS28xx::s_pThis = 0;

DisplayWS28xx::DisplayWS28xx(TWS28XXType tLedType, bool bShowSysTime) {
    m_bShowSysTime = bShowSysTime;
	s_pThis = this;	
}

DisplayWS28xx::~DisplayWS28xx(void) {
	delete m_WS28xx;
	m_WS28xx = 0;
}

void DisplayWS28xx::Init(uint8_t nIntensity) {
	m_WS28xx = new WS28xx(WS2812,16);
	m_WS28xx->Initialize();
	m_WS28xx->SetGlobalBrightness(nIntensity);
	m_WS28xx->SetLED(0,0,255,0);
	m_WS28xx->SetLED(1,0,255,0);
	m_WS28xx->Update();
}

void DisplayWS28xx::Blackout() {
	m_WS28xx->Blackout();
}


void DisplayWS28xx::Show(const char* pTimecode) {
	
	//ws28xx_setDigit(int Digit, pTimecode[0] - '0');

	/* max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT7, (pTimecode[0] - '0'));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT6, (pTimecode[1] - '0') | 0x80);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT5, (pTimecode[3] - '0'));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT4, (pTimecode[4] - '0') | 0x80);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT3, (pTimecode[6] - '0'));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT2, (pTimecode[7] - '0') | 0x80);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT1, (pTimecode[9] - '0'));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT0, (pTimecode[10] - '0'));
    */
	
}

void DisplayWS28xx::ShowSysTime(void) {
	if (m_bShowSysTime) {
		//m_pMax7219Set->ShowSysTime();
	}
}



void DisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos) {
	//m_WS28xx->WriteChar(nChar, nPos);
}
