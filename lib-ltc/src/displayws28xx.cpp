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
#include <stdint.h>
#include "hardware.h"

#include "displayws28xx.h"

DisplayWS28xx *DisplayWS28xx::s_pThis = 0;

DisplayWS28xx::DisplayWS28xx(TWS28XXType tLedType, bool bShowSysTime) {
    m_bShowSysTime = bShowSysTime;
	s_pThis = this;	
	s_wsticker = 0;
	curR = 255; curG = 0; curB = 0;
}

DisplayWS28xx::~DisplayWS28xx(void) {
	delete m_WS28xx;
	m_WS28xx = 0;
}

void DisplayWS28xx::Init(uint8_t nIntensity) {
	m_WS28xx = new WS28xx(WS2812,WS28XX_LED_COUNT);
	m_WS28xx->SetGlobalBrightness(nIntensity);
	m_WS28xx->Initialize();
}

void DisplayWS28xx::Blackout() {
	m_WS28xx->Blackout();
}


uint32_t DisplayWS28xx::Run(){		
	
	m_nMillis = Hardware::Get()->Millis();

	if (m_nMillis >= s_wsticker) {
		s_wsticker = m_nMillis + TEST_INTERVAL_MS;
		level++;
		if (level > 15)
		 level = 0;		
		DisplayWS28xx::WriteChar(level,0);

		m_WS28xx->Update();
		return 1;
	}
	return 0;
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

// return true if changed
void DisplayWS28xx::RenderSegment(uint8_t OnOff, uint32_t cur_digit_base, uint8_t cur_segment) {	
	uint32_t cur_seg_base = cur_digit_base + (cur_segment * LEDS_PER_SEGMENT);
	for (uint32_t cnt = cur_seg_base; cnt < (cur_seg_base + LEDS_PER_SEGMENT); cnt++) {
	  if (OnOff) { 
		  m_WS28xx->SetLED(cnt,curR,curG,curB); // on
	  }
	  else m_WS28xx->SetLED(cnt,0,0,0); // off
	}
}

void DisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos) {
	if (nChar > sizeof(Seg7Array) || nChar < 0) 
		return; 

	uint32_t cur_digit_base = nPos * SEGMENTS_PER_DIGIT;
	
	const uint8_t chr = Seg7Array[nChar];
	
	RenderSegment(chr & (1<<6), cur_digit_base, 0);
	RenderSegment(chr & (1<<5), cur_digit_base, 1);
	RenderSegment(chr & (1<<4), cur_digit_base, 2);
	RenderSegment(chr & (1<<3), cur_digit_base, 3);
	RenderSegment(chr & (1<<2), cur_digit_base, 4);
	RenderSegment(chr & (1<<1), cur_digit_base, 5);
	RenderSegment(chr & (1<<0), cur_digit_base, 6);


}
