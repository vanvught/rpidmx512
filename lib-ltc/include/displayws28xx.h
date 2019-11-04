/**
 * @file displayws28xx.h
 */
/* 
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Based on: displaymax7219.h
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

#ifndef DISPLAYWS28XX_H_
#define DISPLAYWS28XX_H_

#include <stdint.h>
#include <stdbool.h>

#include "ws28xx.h"

/*
 *  A 8 x 7 Segment (with 3 colons) TC Display constructed of WS82xx LEDS, 
 *  
 *   AAA
 *	F	  B
 *	F	  B    
 *	F	  B
 *	 GGG     x 8 
 * 	E	  C
 * 	E	  C	
 * 	E	  C
 *   DDD	
 * 
 *  Then the colons x 3 at the end.
 * 
*/

#define NUM_OF_DIGITS 8
#define NUM_OF_COLONS 3

#define SEGMENTS_PER_DIGIT 7 // number of LEDs that make up one digit
#define LEDS_PER_SEGMENT 1   // number of LEDs that make up one segment
#define LEDS_PER_COLON 2     // number of LEDs that make up one colon

#define WS28XX_LED_COUNT ((LEDS_PER_SEGMENT * SEGMENTS_PER_DIGIT * NUM_OF_DIGITS) + (LEDS_PER_COLON * NUM_OF_COLONS))

#define WS28XX_UPDATE_MS (15)	// update fades every msec


// WS28xx LED order
enum tWS28xxMapping {
	RGB = 0,  // normal
 	RBG,
  	BGR, 
};


class DisplayWS28xx {
public:
	DisplayWS28xx(TWS28XXType tLedType, bool bShowSysTime = false);
	~DisplayWS28xx(void);

	void Init(uint8_t nIntensity, tWS28xxMapping lMapping);
	void Blackout();

    // testing
	bool Run();

	// display current TC
	void Show(const char *pTimecode);
	// display system clock
	void ShowSysTime(void);

  	// set RGB for next character(s)
  	void SetRGB(uint8_t red, uint8_t green, uint8_t blue);
	// set RGB from a hex string  
	void SetRGB(const char *hexstr);

	// set the master brightness
	void SetMaster(uint8_t value);
	
	// write a character from to digits 0..7
	void WriteChar(const uint8_t nChar, uint8_t nPos = 0);
	// write a colon if ':' or '.' to a colon at nPos = 0,1,2
	void WriteColon(uint8_t nChar, uint8_t nPos);

	// return a pointer to this instance
	static DisplayWS28xx* Get(void) {
		return s_pThis;
	}

private:
	// draw one segment of a digit
	void RenderSegment(uint8_t OnOff, uint16_t cur_digit_base, uint8_t cur_segment);
	int hexadecimalToDecimal(const char *hexVal, int len = 6);

	WS28xx *m_WS28xx;
	TWS28XXType m_tLedType = WS2812;
  	tWS28xxMapping l_mapping = RGB;

	bool m_bShowSysTime;

	uint8_t curR = 0, curG = 0, curB = 0; // RGB set for character next rendered
	uint8_t colR = 0, colG = 0, colB = 0; // RGB for colons  : 

	uint32_t ms_colon_blink = 0; 
	uint8_t mColonBlinkMode = 1; // 0 = no blink; 1 = blink down; 2 = blink up
	
	uint8_t old_SC = 1;   // has seconds changed

	uint8_t level = 0;

	uint8_t master = 255;
	uint8_t onoff = 0;

	// timer
	uint32_t m_nMillis; // current timer clock in MS

	uint32_t s_wsticker;
		
	uint32_t m_LastSecondsChangedMS; // last timer time when the timecode seconds changed

	static DisplayWS28xx *s_pThis;
};

#endif /* DISPLAYWS28XX_H_ */
