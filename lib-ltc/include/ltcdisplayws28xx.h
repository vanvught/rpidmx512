/**
 * @file ltcdisplayws28xx.h
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LTCDISPLAYWS28XX_H_
#define LTCDISPLAYWS28XX_H_

#include <stdint.h>

#include "ltcdisplayws28xxset.h"

#include "ws28xx.h"
#include "rgbmapping.h"

enum TLtcDisplayWS28xxTypes {
	LTCDISPLAYWS28XX_TYPE_7SEGMENT,
	LTCDISPLAYWS28XX_TYPE_MATRIX
};

enum TLtcDisplayWS28xxColonBlinkMode {
	LTCDISPLAYWS28XX_COLON_BLINK_MODE_OFF,
	LTCDISPLAYWS28XX_COLON_BLINK_MODE_DOWN,
	LTCDISPLAYWS28XX_COLON_BLINK_MODE_UP
};

enum TLtcDisplayWS28xxColourIndex {
	LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT,
	LTCDISPLAYWS28XX_COLOUR_INDEX_COLON,
	LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE,
	LTCDISPLAYWS28XX_COLOUR_INDEX_LAST
};

enum TLtcDisplayWS28xxDefaults {
	LTCDISPLAYWS28XX_DEFAULT_LED_TYPE = WS2812B,
	LTCDISPLAYWS28XX_DEFAULT_COLOUR_DIGIT = (uint32_t) 0x00FF0000,
	LTCDISPLAYWS28XX_DEFAULT_COLOUR_COLON = (uint32_t) 0x00FFFC00,
	LTCDISPLAYWS28XX_DEFAULT_COLOUR_MESSAGE = (uint32_t) 0x00FFFFFF,
	LTCDISPLAYWS28XX_DEFAULT_COLON_BLINK_MODE = LTCDISPLAYWS28XX_COLON_BLINK_MODE_UP,
	LTCDISPLAYWS28XX_DEFAULT_MASTER = 0xFF,
	LTCDISPLAYWS28XX_DEFAULT_GLOBAL_BRIGHTNESS = 0xFF,
};

class LtcDisplayWS28xx {
public:
	LtcDisplayWS28xx(TLtcDisplayWS28xxTypes tType = LTCDISPLAYWS28XX_TYPE_7SEGMENT);
	~LtcDisplayWS28xx(void);

	void SetMapping(TRGBMapping tMapping) {
		m_tMapping = tMapping;
	}

	void SetMaster(uint8_t nValue) {
		m_nMaster = nValue;
	}

	void SetColonBlinkMode(TLtcDisplayWS28xxColonBlinkMode tColonBlinkMode) {
		m_tColonBlinkMode = tColonBlinkMode;
	}

	void SetColour(uint32_t nRGB, TLtcDisplayWS28xxColourIndex tIndex) {
		if (tIndex >= LTCDISPLAYWS28XX_COLOUR_INDEX_LAST) {
			return;
		}
		m_aColour[(uint32_t) tIndex] = nRGB;
	}

	void Init(TWS28XXType tLedType, uint8_t nIntensity = 0xFF);
	void Print(void);

	void Run(void);

	void Show(const char *pTimecode);
	void ShowSysTime(const char *pSystemTime);

	void WriteChar(uint8_t nChar, uint8_t nPos = 0);

	static LtcDisplayWS28xx *Get(void) {
		return s_pThis;
	}

	void SetMessage(const char *pMessage, uint32_t nSize);
	void SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, TLtcDisplayWS28xxColourIndex tIndex);

private:
	void SetRGB(uint32_t nRGB, TLtcDisplayWS28xxColourIndex tIndex);
	void SetRGB(const char *pHexString);
	uint32_t hexadecimalToDecimal(const char *pHexValue, uint32_t nLength = 6);
	void ShowMessage(void);

private:
	TLtcDisplayWS28xxTypes m_tDisplayWS28xxTypes;
	uint8_t m_nIntensity;
	int32_t m_nHandle;
	uint8_t m_Buffer[64];
  	TRGBMapping m_tMapping;
  	TWS28XXType m_tLedType;
	uint32_t m_aColour[LTCDISPLAYWS28XX_COLOUR_INDEX_LAST];
	uint32_t m_nMaster;
	bool m_bShowMsg;
	char m_aMessage[LTCDISPLAY_MAX_MESSAGE_SIZE];
	uint32_t m_nMsgTimer;
	uint32_t m_nColonBlinkMillis;
	uint32_t m_nSecondsPrevious;
	enum TLtcDisplayWS28xxColonBlinkMode m_tColonBlinkMode;

	LtcDisplayWS28xxSet *m_pLtcDisplayWS28xxSet;
	struct TLtcDisplayRgbColours m_tColours;
	struct TLtcDisplayRgbColours m_tColoursMessage;
	struct TLtcDisplayRgbColours m_tColoursColons;

	static LtcDisplayWS28xx *s_pThis;
};

#endif /* LTCDISPLAYWS28XX_H_ */
