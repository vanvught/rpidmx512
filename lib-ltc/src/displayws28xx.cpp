/**
 * @file displayws28xx.cpp
 */
/* 
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Based on: displaymax7219.cpp
 * Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "hardware.h"

#include "displayws28xx.h"
#include "displayws28xx_font.h"

DisplayWS28xx *DisplayWS28xx::s_pThis = 0;

DisplayWS28xx::DisplayWS28xx(TWS28XXType tLedType, bool bShowSysTime) {
    m_bShowSysTime = bShowSysTime;
	m_tLedType = tLedType;
	s_pThis = this;	
	s_wsticker = 0;
}

DisplayWS28xx::~DisplayWS28xx(void) {
	delete m_WS28xx;
	m_WS28xx = 0;
}

void DisplayWS28xx::Init(uint8_t nIntensity, tWS28xxMapping lMapping) {		
	m_WS28xx = new WS28xx(m_tLedType,WS28XX_LED_COUNT);
	m_WS28xx->Initialize();
	m_WS28xx->SetGlobalBrightness(nIntensity);
	l_mapping = lMapping;
}

// set the current RGB values, remapping them to different LED strip mappings
void DisplayWS28xx::SetRGB(uint8_t red, uint8_t green, uint8_t blue) {
 switch (l_mapping)
 {
 case RGB:
	 curR = red;
	 curG = green;
	 curB = blue;
	 break;
 
  case RBG:
	 curR = red;
	 curG = blue;
	 curB = green;
	 break;

 case BGR:
	 curR = blue;
	 curG = green;
	 curB = red;
	 break;
  
 default:
	curR = red;
	curG = green;
	curB = blue;
	break;
 }
}

void DisplayWS28xx::Blackout() {
	m_WS28xx->Blackout();
}

bool DisplayWS28xx::Run(){		
	m_nMillis = Hardware::Get()->Millis();
	
	if (m_nMillis >= s_wsticker) {
		s_wsticker = m_nMillis + TEST_INTERVAL_MS;

		level++;
		if (level > 90)
		{
			level = 48;
			
			if (onoff > 3) {
				onoff = 0;
				//printf("\n");
			}

			if (onoff == 0)
			{
				printf("Red ");
				SetRGB(255, 0, 0);
			}
			else if (onoff == 1)
			{
				printf("Green ");
				SetRGB(0, 255, 0);
			}
			else if (onoff == 2)
			{
				printf("Blue ");
				SetRGB(0, 0, 255);
			}			
			else if (onoff == 3)
			{
				printf("White ");
				SetRGB(255, 255, 255);
			}
			onoff++;

		}
       
		WriteChar(level,0);
		m_WS28xx->Update();
		return 1;	
	}
	return 0;
}

void DisplayWS28xx::Show(const char* pTimecode) {
	for (uint8_t cnt = 0; cnt < NUM_OF_DIGITS; cnt++)
	WriteChar(pTimecode[cnt],cnt);
}

void DisplayWS28xx::ShowSysTime(void) {  // TODO: Adapt
	

	/* time_t ltime;
	struct tm *local_time;

	ltime = time(0);
	local_time = localtime(&ltime);

	if (__builtin_expect((m_nSecondsPrevious == (uint32_t) local_time->tm_sec), 1)) {
		return;
	}

	m_nSecondsPrevious = local_time->tm_sec;

	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT7,  MAX7219_CHAR_BLANK);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT6, (local_time->tm_hour / 10));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT5, (local_time->tm_hour % 10) | 0x80);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT4, (local_time->tm_min  / 10));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT3, (local_time->tm_min  % 10) | 0x80);
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT2, (local_time->tm_sec  / 10));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT1, (local_time->tm_sec  % 10));
	max7219_spi_write_reg(&m_DeviceInfo, MAX7219_REG_DIGIT0,  MAX7219_CHAR_BLANK); */
}

void DisplayWS28xx::RenderSegment(uint8_t OnOff, uint16_t cur_digit_base, uint8_t cur_segment) {	
	uint16_t cur_seg_base = cur_digit_base + (cur_segment * LEDS_PER_SEGMENT);
	for (uint16_t cnt = cur_seg_base; cnt < (cur_seg_base + LEDS_PER_SEGMENT); cnt++) {
	  if (OnOff) { 
		  m_WS28xx->SetLED(cnt,curR,curG,curB); // on		  
	  }
	  else {
		  m_WS28xx->SetLED(cnt,0,0,0); // off		 
	  }	  
	}
}

void DisplayWS28xx::WriteChar(uint8_t nChar, uint8_t nPos) {
	if (nChar > sizeof(Seg7Array) || nChar < 0) 
		return; 

	uint16_t cur_digit_base = nPos * SEGMENTS_PER_DIGIT;
	const uint8_t chr = Seg7Array[nChar];
	RenderSegment(chr & (1<<6), cur_digit_base, 0);
	RenderSegment(chr & (1<<5), cur_digit_base, 1);
	RenderSegment(chr & (1<<4), cur_digit_base, 2);
	RenderSegment(chr & (1<<3), cur_digit_base, 3);
	RenderSegment(chr & (1<<2), cur_digit_base, 4);
	RenderSegment(chr & (1<<1), cur_digit_base, 5);
	RenderSegment(chr & (1<<0), cur_digit_base, 6);


}
