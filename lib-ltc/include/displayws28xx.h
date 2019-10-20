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

#define TEST_INTERVAL_MS (250)	// msec

// LED order
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

	void Show(const char *pTimecode);
	void ShowSysTime(void);

  // set RGB for next character(s)
  void SetRGB(uint8_t red, uint8_t green, uint8_t blue);

  // write a character
	void WriteChar(uint8_t nChar, uint8_t nPos = 0);
	void WriteColon(uint8_t nChar, uint8_t nPos);
	
	static DisplayWS28xx* Get(void) {
		return s_pThis;
	}

private:
	void RenderSegment(uint8_t OnOff, uint16_t cur_digit_base, uint8_t cur_segment);

	WS28xx *m_WS28xx;
  TWS28XXType m_tLedType = WS2812;
  tWS28xxMapping l_mapping = RGB;

	bool m_bShowSysTime;
	uint8_t curR, curG, curB;
  
  uint8_t level = 0;
  uint8_t onoff = 0;

	// timer
	uint32_t s_wsticker;
	uint32_t m_nMillis;
	
	static DisplayWS28xx *s_pThis;
};

#endif /* DISPLAYWS28XX_H_ */
