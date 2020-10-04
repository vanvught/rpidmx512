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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "ltcdisplayparams.h"

#include "ltcdisplayparamsconst.h"
#include "devicesparamsconst.h"
// Displays
#include "ltcdisplaymax7219.h"
#include "ltcdisplayrgb.h"
#include "ws28xx.h"
#include "ws28xxconst.h"
#include "rgbmapping.h"

#include "readconfigfile.h"
#include "sscan.h"

static constexpr char aColonBlinkMode[3][5] = { "off", "down", "up" };

LtcDisplayParams::LtcDisplayParams(LtcDisplayParamsStore *pLtcDisplayParamsStore): m_pLtcDisplayParamsStore(pLtcDisplayParamsStore) {
	m_tLtcDisplayParams.nWS28xxLedType = LTCDISPLAYWS28XX_DEFAULT_LED_TYPE;
	m_tLtcDisplayParams.nGlobalBrightness = LTCDISPLAYWS28XX_DEFAULT_GLOBAL_BRIGHTNESS;
	m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_MATRIX;
	m_tLtcDisplayParams.nMax7219Intensity = 4;
	m_tLtcDisplayParams.nWS28xxRgbMapping = RGB_MAPPING_RGB;
	m_tLtcDisplayParams.nDisplayRgbIntensity = LTCDISPLAYWS28XX_DEFAULT_MASTER;
	m_tLtcDisplayParams.nDisplayRgbColonBlinkMode = LTCDISPLAYWS28XX_DEFAULT_COLON_BLINK_MODE;
	m_tLtcDisplayParams.aDisplayRgbColour[LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_DIGIT;
	m_tLtcDisplayParams.aDisplayRgbColour[LTCDISPLAYWS28XX_COLOUR_INDEX_COLON] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_COLON;
	m_tLtcDisplayParams.aDisplayRgbColour[LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE] = LTCDISPLAYWS28XX_DEFAULT_COLOUR_MESSAGE;
	m_tLtcDisplayParams.nWS28xxDisplayType = static_cast<uint8_t>(LtcDisplayRgbWS28xxType::SEGMENT);
}

bool LtcDisplayParams::Load() {
	m_tLtcDisplayParams.nSetList = 0;

	ReadConfigFile configfile(LtcDisplayParams::staticCallbackFunction, this);

	if (configfile.Read(LtcDisplayParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pLtcDisplayParamsStore != nullptr) {
			m_pLtcDisplayParamsStore->Update(&m_tLtcDisplayParams);
		}
	} else if (m_pLtcDisplayParamsStore != nullptr) {
		m_pLtcDisplayParamsStore->Copy(&m_tLtcDisplayParams);
	} else {
		return false;
	}

	return true;
}

void LtcDisplayParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pLtcDisplayParamsStore != nullptr);

	if (m_pLtcDisplayParamsStore == nullptr) {
		return;
	}

	m_tLtcDisplayParams.nSetList = 0;

	ReadConfigFile config(LtcDisplayParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLtcDisplayParamsStore->Update(&m_tLtcDisplayParams);
}

void LtcDisplayParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aBuffer[16];
	uint8_t nValue8;
	uint32_t nValue32;
	uint32_t nLength = sizeof(aBuffer) - 1;

	if (Sscan::Char(pLine, LtcDisplayParamsConst::MAX7219_TYPE, aBuffer, nLength) == Sscan::OK) {
		if (strncasecmp(aBuffer, "7segment", nLength) == 0) {
			m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_7SEGMENT;
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::MAX7219_TYPE;
		} else if (strncasecmp(aBuffer, "matrix", nLength) == 0) {
			m_tLtcDisplayParams.nMax7219Type = LTCDISPLAYMAX7219_TYPE_MATRIX;
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::MAX7219_TYPE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::MAX7219_INTENSITY, nValue8) == Sscan::OK) {
		if (nValue8 <= 0x0F) {
			m_tLtcDisplayParams.nMax7219Intensity = nValue8;
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::MAX7219_INTENSITY;
		}
		return;
	}

	nLength = 8;
	if (Sscan::Char(pLine, LtcDisplayParamsConst::WS28XX_TYPE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		if (strncasecmp(aBuffer, "7segment", nLength) == 0) {
			m_tLtcDisplayParams.nWS28xxDisplayType = static_cast<uint8_t>(LtcDisplayRgbWS28xxType::SEGMENT);
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::WS28XX_DISPLAY_TYPE;
		} else if (strncasecmp(aBuffer, "matrix", nLength) == 0) {
			m_tLtcDisplayParams.nWS28xxDisplayType = static_cast<uint8_t>(LtcDisplayRgbWS28xxType::MATRIX);
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::WS28XX_DISPLAY_TYPE;
		}
		return;
	}

	nLength = 7;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_TYPE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		for (uint32_t i = 0; i < WS28XX_UNDEFINED; i++) {
			if (strcasecmp(aBuffer, WS28xxConst::TYPES[i]) == 0) {
				m_tLtcDisplayParams.nWS28xxLedType = i;
				m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::WS28XX_LED_TYPE;
				return;
			}
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_RGB_MAPPING, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		enum TRGBMapping tMapping;
		if ((tMapping = RGBMapping::FromString(aBuffer)) != RGB_MAPPING_UNDEFINED) {
			m_tLtcDisplayParams.nWS28xxRgbMapping = tMapping;
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::WS28XX_RGB_MAPPING;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::WS28XX_INTENSITY, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcDisplayParams.nDisplayRgbIntensity = nValue8;
			m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::DISPLAYRGB_INTENSITY;
		}
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, LtcDisplayParamsConst::WS28XX_COLON_BLINK_MODE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		for (uint32_t i = 0; i < (sizeof(aColonBlinkMode) / sizeof(aColonBlinkMode[0])); i++) {
			if (strcasecmp(aBuffer, aColonBlinkMode[i]) == 0) {
				m_tLtcDisplayParams.nDisplayRgbColonBlinkMode = i;
				m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::DISPLAYRGB_COLON_BLINK_MODE;
				return;
			}
		}
		return;
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if(Sscan::Hex24Uint32(pLine, LtcDisplayParamsConst::WS28XX_COLOUR[nIndex], nValue32) == Sscan::OK) {
			m_tLtcDisplayParams.aDisplayRgbColour[nIndex] = nValue32;
			m_tLtcDisplayParams.nSetList |= (LtcDisplayParamsMask::DISLAYRGB_COLOUR_INDEX << nIndex);
			return;
		}
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, nValue8) == Sscan::OK) {
		m_tLtcDisplayParams.nGlobalBrightness = nValue8;
		m_tLtcDisplayParams.nSetList |= LtcDisplayParamsMask::GLOBAL_BRIGHTNESS;
		return;
	}
}

void LtcDisplayParams::Set(LtcDisplayRgb *pLtcDisplayWS28xx) {
	assert(pLtcDisplayWS28xx != nullptr);

	if (isMaskSet(LtcDisplayParamsMask::WS28XX_RGB_MAPPING)) {
		pLtcDisplayWS28xx->SetMapping(static_cast<TRGBMapping>(m_tLtcDisplayParams.nWS28xxRgbMapping));
	}

	if (isMaskSet(LtcDisplayParamsMask::DISPLAYRGB_INTENSITY)) {
		pLtcDisplayWS28xx->SetMaster(m_tLtcDisplayParams.nDisplayRgbIntensity);
	}

	if (isMaskSet(LtcDisplayParamsMask::DISPLAYRGB_COLON_BLINK_MODE)) {
		pLtcDisplayWS28xx->SetColonBlinkMode(static_cast<TLtcDisplayWS28xxColonBlinkMode>(m_tLtcDisplayParams.nDisplayRgbColonBlinkMode));
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if (isMaskSet((LtcDisplayParamsMask::DISLAYRGB_COLOUR_INDEX << nIndex))) {
			pLtcDisplayWS28xx->SetColour(m_tLtcDisplayParams.aDisplayRgbColour[nIndex], static_cast<TLtcDisplayWS28xxColourIndex>(nIndex));
		}
	}
}

void LtcDisplayParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcDisplayParams*>(p))->callbackFunction(s);
}
