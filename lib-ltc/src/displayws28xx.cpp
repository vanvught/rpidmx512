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
#include <stdlib.h>
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
	master = 255; 
	curR = 255;  // default to full red
	curG = 0;
	curB = 0;
	mColonBlinkMode = 1;
}

void DisplayWS28xx::SetMaster(uint8_t value){
	master = value;
}

// set the current RGB values, remapping them to different LED strip mappings
void DisplayWS28xx::SetRGB(uint8_t red, uint8_t green, uint8_t blue)
{
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

// set the current RGB values from a hex string, eg FFCC00
void DisplayWS28xx::SetRGB(const char *hexstr)
{
	uint32_t rgb = hexadecimalToDecimal(hexstr);
	uint8_t r=(uint8_t)(rgb>>16);
	uint8_t g=(uint8_t)(rgb>>8);
	uint8_t b=(uint8_t)rgb & 0xff;
	SetRGB(r,g,b);
}


void DisplayWS28xx::Blackout() {
	m_WS28xx->Blackout();
}



bool DisplayWS28xx::Run(){		
	m_nMillis = Hardware::Get()->Millis(); // millis now
	
	if (m_nMillis >= s_wsticker) {
		s_wsticker = m_nMillis + WS28XX_UPDATE_MS;

		// We will be running temporary messages and fades here.

		return 1;	
	}
	return 0;
}



// Convert hexadecimal string to decimal value
int DisplayWS28xx::hexadecimalToDecimal(const char *hexVal, int len) 
{    
    int base = 1;       
    int dec_val = 0; 
    for (int i=len-1; i>=0; i--) 
    {    
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
            base = base * 16; 
        }   
        else if (hexVal[i]>='A' && hexVal[i]<='F') 
        { 
            dec_val += (hexVal[i] - 55)*base; 
            base = base*16; 
        } 
    }       
    return dec_val; 
} 


// return absolute number of given integer n
inline int abs(int n) 
{
  return n * ( (n<0) * (-1) + (n>0));
}


void DisplayWS28xx::Show(const char *pTimecode)
{
	WriteChar(pTimecode[0], 0);
	WriteChar(pTimecode[1], 1); // hh

	WriteChar(pTimecode[3], 2);
	WriteChar(pTimecode[4], 3); // mm

	WriteChar(pTimecode[6], 4);
	WriteChar(pTimecode[7], 5); // sc

	WriteChar(pTimecode[9], 6);
	WriteChar(pTimecode[10], 7); // fr

	// option - blink colon 
	if (mColonBlinkMode > 0)
	{
		uint8_t mColonBlinkOffset = 0;
		switch (mColonBlinkMode)
		{
		case 1:
			mColonBlinkOffset = 0;
			break;
		case 2:
			mColonBlinkOffset = 255;
			break;
		default:
			mColonBlinkOffset = 0;
			break;
		}

		if (old_SC != pTimecode[7]) // seconds have changed
		{
			ms_colon_blink = m_nMillis + 1000;
			old_SC = pTimecode[7];
			colR = 0;
			colG = 0;
			colB = 0;
		}
		else if (m_nMillis < ms_colon_blink)
		{
			colR = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - curR));
			colG = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - curG));
			colB = (((float)(ms_colon_blink - m_nMillis) / 1000) * abs(mColonBlinkOffset - curB));
		}
	}
	else
	{
		colR = curR; // straight thru
		colG = curG;
		colB = curB;
	}

	WriteColon(pTimecode[2], 0); // 1st :
    WriteColon(pTimecode[5], 1); // 2nd :	
    WriteColon(pTimecode[8], 2); // 3rd :
	
	m_WS28xx->Update();	
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
	uint8_t red = curR;
	uint8_t green = curG;
	uint8_t blue = curB;
	if (master != 0 || master != 255) {	
		red = (master * curR) / 255;
		green = (master * curG) / 255;
		blue = (master * curB) / 255;		
	} 

	uint16_t cur_seg_base = cur_digit_base + (cur_segment * LEDS_PER_SEGMENT);
	for (uint16_t cnt = cur_seg_base; cnt < (cur_seg_base + LEDS_PER_SEGMENT); cnt++) {
	  if (OnOff) { 
		  m_WS28xx->SetLED(cnt,red,green,blue); // on		  
	  }
	  else {
		  m_WS28xx->SetLED(cnt,0,0,0); // off		 
	  }	  
	}
}

void DisplayWS28xx::WriteChar(const uint8_t nChar, uint8_t nPos) {
	if (nChar > sizeof(Seg7Array) || nChar < 0) 
		return; 
	uint16_t cur_digit_base = nPos * SEGMENTS_PER_DIGIT;

	uint8_t chr;	
	if (nChar & (1<<7)) // use custom bitmap
	  chr = nChar;
	else chr = Seg7Array[nChar]; // use displayws28xx_font

	RenderSegment(chr & (1<<6), cur_digit_base, 0);
	RenderSegment(chr & (1<<5), cur_digit_base, 1);
	RenderSegment(chr & (1<<4), cur_digit_base, 2);
	RenderSegment(chr & (1<<3), cur_digit_base, 3);
	RenderSegment(chr & (1<<2), cur_digit_base, 4);
	RenderSegment(chr & (1<<1), cur_digit_base, 5);
	RenderSegment(chr & (1<<0), cur_digit_base, 6);
}


void DisplayWS28xx::WriteColon(uint8_t nChar, uint8_t nPos) {
	uint8_t red = colR;
	uint8_t green = colG;
	uint8_t blue = colB;
	if (master != 0 || master != 255) {	
		red = (master * colR) / 255;
		green = (master * colG) / 255;
		blue = (master * colB) / 255;		
	} 

	if (nChar > sizeof(Seg7Array) || nChar < 0) 
		return; 

	uint16_t cur_digit_base = (NUM_OF_DIGITS * SEGMENTS_PER_DIGIT) + (nPos * LEDS_PER_COLON);
	
	bool OnOff = (nChar == ':' || nChar == '.' || nChar == ';') ? 1 : 0;
	for (uint16_t cnt = cur_digit_base; cnt < (cur_digit_base + LEDS_PER_COLON); cnt++) {
	if (OnOff) { 
		  m_WS28xx->SetLED(cnt,red,green,blue); // on		  
	  }
	  else {
		  m_WS28xx->SetLED(cnt,0,0,0); // off		 
	  }	
	}  
}
