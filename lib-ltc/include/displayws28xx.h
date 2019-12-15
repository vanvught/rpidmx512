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

#if defined(USE_SPI_DMA)
 #include "h3/ws28xxdma.h"
#else
 #include "ws28xx.h"
#endif
#include "rgbmapping.h"

enum TColonBlinkMode {
	COLON_BLINK_MODE_OFF,
	COLON_BLINK_MODE_DOWN,
	COLON_BLINK_MODE_UP
};

enum TWS28xxColourIndex {
	WS28XX_COLOUR_INDEX_SEGMENT,
	WS28XX_COLOUR_INDEX_COLON,
	WS28XX_COLOUR_INDEX_MESSAGE,
	WS28XX_COLOUR_INDEX_LAST
};

enum TWS28xxDisplayDefaults {
	WS28XXDISPLAY_DEFAULT_LED_TYPE = WS2812B,
	WS28XXDISPLAY_DEFAULT_COLOUR_SEGMENT = (uint32_t) 0x00FF0000,
	WS28XXDISPLAY_DEFAULT_COLOUR_COLON = (uint32_t) 0x00FFFC00,
	WS28XXDISPLAY_DEFAULT_COLOUR_MESSAGE = (uint32_t) 0x00FFFFFF,
	WS28XXDISPLAY_DEFAULT_COLON_BLINK_MODE = COLON_BLINK_MODE_UP,
	WS28XXDISPLAY_DEFAULT_MASTER = 0xFF,
	WS28XXDISPLAY_DEFAULT_GLOBAL_BRIGHTNESS = 0xFF,
};

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

#define WS28XX_NUM_OF_DIGITS	8
#define WS28XX_NUM_OF_COLONS	3

#define SEGMENTS_PER_DIGIT 		7		///< number of LEDs that make up one digit
#define LEDS_PER_SEGMENT 		1		///< number of LEDs that make up one segment
#define LEDS_PER_COLON 			2		///< number of LEDs that make up one colon

#define WS28XX_LED_COUNT 		((LEDS_PER_SEGMENT * SEGMENTS_PER_DIGIT * WS28XX_NUM_OF_DIGITS) + (LEDS_PER_COLON * WS28XX_NUM_OF_COLONS))

#define WS28XX_UPDATE_MS		(15)	///< update fades every msec

#define WS28XX_MAX_MSG_SIZE		(WS28XX_NUM_OF_DIGITS + WS28XX_NUM_OF_COLONS + 1)

#define WS82XX_MSG_TIME_MS		(3000)

class DisplayWS28xx {
public:
	DisplayWS28xx(TWS28XXType tLedType);
	~DisplayWS28xx(void);

	void Init(uint8_t nIntensity);
	void Run();

	void Print(void);

	void Show(const char *pTimecode);
	void ShowSysTime(const char *pSystemTime);

	void SetMapping(TRGBMapping tMapping) {
		m_tMapping = tMapping;
	}

	void SetMaster(uint8_t nValue) {
		m_nMaster = nValue;
	}

	void SetColonBlinkMode(TColonBlinkMode tColonBlinkMode) {
		m_nColonBlinkMode = tColonBlinkMode;
	}

	void SetColour(uint32_t nRGB, TWS28xxColourIndex tIndex) {
		if (tIndex >= WS28XX_COLOUR_INDEX_LAST) {
			return;
		}
		m_aColour[(uint32_t) tIndex] = nRGB;
	}

	static DisplayWS28xx *Get(void) {
		return s_pThis;
	}

private:
  	// set RGB for next character(s)
	void SetRGB(uint32_t nRGB, TWS28xxColourIndex tIndex);
  	void SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, TWS28xxColourIndex tIndex);
	// set RGB from a hex string  
	void SetRGB(const char *pHexString);
	// write a character from to digits 0..7
	void WriteChar(uint8_t nChar, uint8_t nPos, uint8_t nRed = 255, uint8_t nGreen = 255, uint8_t nBlue = 255);
	// write a colon if ':' or '.' to a colon at nPos = 0,1,2
	void WriteColon(uint8_t nChar, uint8_t nPos, uint8_t nRed = 128, uint8_t nGreen = 128, uint8_t nBlue = 128);
	// set a message to appear temporarily 
	void SetMessage(const char *pMessage, uint32_t nSize);
	// draw one segment of a digit
	void RenderSegment(bool bOnOff, uint16_t cur_digit_base, uint8_t cur_segment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	uint32_t hexadecimalToDecimal(const char *pHexValue, uint32_t nLength = 6);
	void ShowMessage(void);

private:
#if defined(USE_SPI_DMA)
	WS28xxDMA *m_pWS28xx;
#else
	WS28xx *m_pWS28xx;
#endif
	TWS28XXType m_tLedType;
  	TRGBMapping m_tMapping;
	uint32_t m_aColour[WS28XX_COLOUR_INDEX_LAST];
	uint8_t m_Buffer[64];					// UDP buffer
	int32_t m_nHandle;						// UDP handle
	uint32_t m_nMaster;						// Master brightness
	bool m_bShowMsg;  						// if true, showing message
	char m_aMessage[WS28XX_MAX_MSG_SIZE]; 	// 11 + chr(0)
	uint32_t m_nSecondsPrevious;   			// has seconds changed
	TColonBlinkMode m_nColonBlinkMode; 		// 0 = no blink; 1 = blink down; 2 = blink up
	uint32_t m_nMillis; 					// current timer clock in MS
	uint32_t m_nWsTicker;
	uint32_t m_nMsgTimer;
	uint32_t m_ColonBlinkMillis;
	uint8_t nRedSegment, nGreenSegment, nBlueSegment;	// RGB for segments
	uint8_t nRedColon, nGreenColon, nBlueColon;			// RGB for colons  :
	uint8_t nRedMsg, nGreenMsg, nBlueMsg;				// RGB for messages

	static DisplayWS28xx *s_pThis;
};

#endif /* DISPLAYWS28XX_H_ */
