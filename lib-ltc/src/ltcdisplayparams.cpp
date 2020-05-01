/**
 * @file ltcdisplayparams.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "ltcdisplayparams.h"

#include "ltcdisplayparamsconst.h"
#include "devicesparamsconst.h"
// Displays
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"
#include "ws28xx.h"
#include "ws28xxconst.h"
#include "rgbmapping.h"

#include "readconfigfile.h"
#include "sscan.h"

static const char aColonBlinkMode[3][5] = { "off", "down", "up" };

LtcDisplayParams::LtcDisplayParams(LtcDisplayParamsStore *pLtcDisplayParamsStore): m_pLtcDisplayParamsStore(pLtcDisplayParamsStore) {
	m_tLtcDisplayParams.nLedType = LTCDISPLAYWS28XX_DEFAULT_LED_TYPE;
	m_tLtcDisplayParams.nGlobalBrightness = LTCDISPLAYWS28XX_DEFAULT_GLOBAL_BRIGHTNESS;
	m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_MATRIX;
	m_tLtcDisplayParams.nMax7219Intensity = 4;
	m_tLtcDisplayParams.nRgbMapping = RGB_MAPPING_RGB;
	m_tLtcDisplayParams.nWS28xxIntensity = LTCDISPLAYWS28XX_DEFAULT_MASTER;
	m_tLtcDisplayParams.nWS28xxColonBlinkMode = LTCDISPLAYWS28XX_DEFAULT_COLON_BLINK_MODE;
	m_tLtcDisplayParams.aWS28xxColour[LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_DIGIT;
	m_tLtcDisplayParams.aWS28xxColour[LTCDISPLAYWS28XX_COLOUR_INDEX_COLON] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_COLON;
	m_tLtcDisplayParams.aWS28xxColour[LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_MESSAGE;
	m_tLtcDisplayParams.nWS28xxType = LTCDISPLAYWS28XX_TYPE_7SEGMENT;
}

LtcDisplayParams::~LtcDisplayParams(void) {
}

bool LtcDisplayParams::Load(void) {
	m_tLtcDisplayParams.nSetList = 0;

	ReadConfigFile configfile(LtcDisplayParams::staticCallbackFunction, this);

	if (configfile.Read(LtcDisplayParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pLtcDisplayParamsStore != 0) {
			m_pLtcDisplayParamsStore->Update(&m_tLtcDisplayParams);
		}
	} else if (m_pLtcDisplayParamsStore != 0) {
		m_pLtcDisplayParamsStore->Copy(&m_tLtcDisplayParams);
	} else {
		return false;
	}

	return true;
}

void LtcDisplayParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pLtcDisplayParamsStore != 0);

	if (m_pLtcDisplayParamsStore == 0) {
		return;
	}

	m_tLtcDisplayParams.nSetList = 0;

	ReadConfigFile config(LtcDisplayParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLtcDisplayParamsStore->Update(&m_tLtcDisplayParams);
}

void LtcDisplayParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char aBuffer[16];
	uint8_t nValue8;
	uint32_t nValue32;
	uint8_t nLength = sizeof(aBuffer);

	if (Sscan::Char(pLine, LtcDisplayParamsConst::MAX7219_TYPE, aBuffer, &nLength) == SSCAN_OK) {
		if (strncasecmp(aBuffer, "7segment", nLength) == 0) {
			m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_7SEGMENT;
			m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_MAX7219_TYPE;
		} else if (strncasecmp(aBuffer, "matrix", nLength) == 0) {
			m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_MATRIX;
			m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_MAX7219_TYPE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::MAX7219_INTENSITY, &nValue8) == SSCAN_OK) {
		if (nValue8 <= 0x0F) {
			m_tLtcDisplayParams.nMax7219Intensity = nValue8;
			m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_MAX7219_INTENSITY;
		}
		return;
	}

	nLength = 7;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_TYPE, aBuffer, &nLength) == SSCAN_OK) {
		aBuffer[nLength] = '\0';
		for (uint32_t i = 0; i < WS28XX_UNDEFINED; i++) {
			if (strcasecmp(aBuffer, WS28xxConst::TYPES[i]) == 0) {
				m_tLtcDisplayParams.nLedType = i;
				m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_LED_TYPE;
				return;
			}
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_RGB_MAPPING, aBuffer, &nLength) == SSCAN_OK) {
		aBuffer[nLength] = '\0';
		enum TRGBMapping tMapping;
		if ((tMapping = RGBMapping::FromString(aBuffer)) != RGB_MAPPING_UNDEFINED) {
			m_tLtcDisplayParams.nRgbMapping = tMapping;
			m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_RGB_MAPPING;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::WS28XX_INTENSITY, &nValue8) == SSCAN_OK) {
		if (nValue8 != 0) {
			m_tLtcDisplayParams.nWS28xxIntensity = nValue8;
			m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_WS28XX_INTENSITY;
		}
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, LtcDisplayParamsConst::WS28XX_COLON_BLINK_MODE, aBuffer, &nLength) == SSCAN_OK) {
		aBuffer[nLength] = '\0';
		for (uint32_t i = 0; i < (sizeof(aColonBlinkMode) / sizeof(aColonBlinkMode[0])); i++) {
			if (strcasecmp(aBuffer, aColonBlinkMode[i]) == 0) {
				m_tLtcDisplayParams.nWS28xxColonBlinkMode = i;
				m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_WS28XX_COLON_BLINK_MODE;
				return;
			}
		}
		return;
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if(Sscan::Hex24Uint32(pLine, LtcDisplayParamsConst::WS28XX_COLOUR[nIndex], &nValue32) == SSCAN_OK) {
			m_tLtcDisplayParams.aWS28xxColour[nIndex] = nValue32;
			m_tLtcDisplayParams.nSetList |= (LTCDISPLAY_PARAMS_MASK_WS28XX_COLOUR_INDEX << nIndex);
			return;
		}
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, &nValue8) == SSCAN_OK) {
		m_tLtcDisplayParams.nGlobalBrightness = nValue8;
		m_tLtcDisplayParams.nSetList |= LTCDISPLAY_PARAMS_MASK_GLOBAL_BRIGHTNESS;
		return;
	}
}

void LtcDisplayParams::Set(LtcDisplayWS28xx *pLtcDisplayWS28xx) {
	assert(pLtcDisplayWS28xx != 0);

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_RGB_MAPPING)) {
		pLtcDisplayWS28xx->SetMapping(static_cast<TRGBMapping>(m_tLtcDisplayParams.nRgbMapping));
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_WS28XX_INTENSITY)) {
		pLtcDisplayWS28xx->SetMaster(m_tLtcDisplayParams.nWS28xxIntensity);
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_WS28XX_COLON_BLINK_MODE)) {
		pLtcDisplayWS28xx->SetColonBlinkMode(static_cast<TLtcDisplayWS28xxColonBlinkMode>(m_tLtcDisplayParams.nWS28xxColonBlinkMode));
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if (isMaskSet((LTCDISPLAY_PARAMS_MASK_WS28XX_COLOUR_INDEX << nIndex))) {
			pLtcDisplayWS28xx->SetColour(m_tLtcDisplayParams.aWS28xxColour[nIndex], static_cast<TLtcDisplayWS28xxColourIndex>(nIndex));
		}
	}
}

void LtcDisplayParams::Dump(void) {
#ifndef NDEBUG
	if (m_tLtcDisplayParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcDisplayParamsConst::FILE_NAME);

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_WS28XX_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::WS28XX_TYPE, m_tLtcDisplayParams.nWS28xxType, m_tLtcDisplayParams.nWS28xxType == LTCDISPLAYWS28XX_TYPE_7SEGMENT ? "7segment" : "matrix");
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_LED_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE,
				WS28xx::GetLedTypeString(
						static_cast<TWS28XXType>(m_tLtcDisplayParams.nLedType)),
				static_cast<int>(m_tLtcDisplayParams.nLedType));
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_RGB_MAPPING)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_RGB_MAPPING,
				RGBMapping::ToString(
						static_cast<TRGBMapping>(m_tLtcDisplayParams.nRgbMapping)),
				static_cast<int>(m_tLtcDisplayParams.nRgbMapping));
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_WS28XX_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::WS28XX_INTENSITY, m_tLtcDisplayParams.nWS28xxIntensity);
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_WS28XX_COLON_BLINK_MODE)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::WS28XX_COLON_BLINK_MODE, m_tLtcDisplayParams.nWS28xxColonBlinkMode);
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if (isMaskSet((LTCDISPLAY_PARAMS_MASK_WS28XX_COLOUR_INDEX << nIndex))) {
			printf(" %s=%.6x\n", LtcDisplayParamsConst::WS28XX_COLOUR[nIndex], m_tLtcDisplayParams.aWS28xxColour[nIndex]);
		}
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS,
				static_cast<int>(m_tLtcDisplayParams.nGlobalBrightness));
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_MAX7219_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::MAX7219_TYPE, m_tLtcDisplayParams.nMax7219Type, m_tLtcDisplayParams.nMax7219Type == LTCDISPLAYMAX7219_TYPE_7SEGMENT ? "7segment" : "matrix");
	}

	if (isMaskSet(LTCDISPLAY_PARAMS_MASK_MAX7219_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::MAX7219_INTENSITY, m_tLtcDisplayParams.nMax7219Intensity);
	}
#endif
}

void LtcDisplayParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<LtcDisplayParams*>(p))->callbackFunction(s);
}
