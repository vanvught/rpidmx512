/**
 * @file ltcdisplayparams.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "ltcdisplayparams.h"
#include "ltcdisplayparamsconst.h"
#include "devicesparamsconst.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayrgb.h"
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
# include "pixeltype.h"
#endif

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

static constexpr char aColonBlinkMode[3][5] = { "off", "down", "up" };

using namespace ltcdisplayrgb;

namespace defaults {
static constexpr auto OLED_INTENSITY = 0x7F;
static constexpr auto MAX7219_INTENSITY = 0x04;
static constexpr auto ROTARY_FULLSTEP = 0x00;
}  // namespace defaults

LtcDisplayParams::LtcDisplayParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	m_Params.nWS28xxType = static_cast<uint8_t>(Defaults::LED_TYPE);
#endif
	m_Params.nGlobalBrightness = Defaults::GLOBAL_BRIGHTNESS;	// Not used
	m_Params.nMax7219Type = static_cast<uint8_t>(ltc::display::max7219::Types::MATRIX);
	m_Params.nMax7219Intensity = defaults::MAX7219_INTENSITY;
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	m_Params.nWS28xxRgbMapping = static_cast<uint8_t>(pixel::Map::RGB);
#endif
	m_Params.nDisplayRgbIntensity = Defaults::MASTER;
	m_Params.nDisplayRgbColonBlinkMode = static_cast<uint8_t>(Defaults::COLON_BLINK_MODE);
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::TIME)] = Defaults::COLOUR_TIME;
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::COLON)] = Defaults::COLOUR_COLON;
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::MESSAGE)] = Defaults::COLOUR_MESSAGE;
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::FPS)] = Defaults::COLOUR_FPS;
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::INFO)] = Defaults::COLOUR_INFO;
	m_Params.aDisplayRgbColour[static_cast<uint32_t>(ColourIndex::SOURCE)] = Defaults::COLOUR_SOURCE;
	m_Params.nWS28xxDisplayType = static_cast<uint8_t>(WS28xxType::SEGMENT);
	memset(m_Params.aInfoMessage, ' ', sizeof(m_Params.aInfoMessage));
	m_Params.nOledIntensity = defaults::OLED_INTENSITY;
	m_Params.nRotaryFullStep = defaults::ROTARY_FULLSTEP;

	DEBUG_EXIT
}

void LtcDisplayParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(LtcDisplayParams::StaticCallbackFunction, this);

	if (configfile.Read(LtcDisplayParamsConst::FILE_NAME)) {
		LtcDisplayParamsStore::Update(&m_Params);
	} else
#endif
		LtcDisplayParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcDisplayParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(LtcDisplayParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	LtcDisplayParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcDisplayParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aBuffer[16];
	uint8_t nValue8;
	uint32_t nValue32;
	uint32_t nLength = sizeof(aBuffer) - 1;

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::OLED_INTENSITY, nValue8) == Sscan::OK) {
		m_Params.nOledIntensity = nValue8;

		if (nValue8 != defaults::OLED_INTENSITY) {
			m_Params.nSetList |= ltcdisplayparams::Mask::OLED_INTENSITY;
		} else {
			m_Params.nSetList &= ~ltcdisplayparams::Mask::OLED_INTENSITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::ROTARY_FULLSTEP, nValue8) == Sscan::OK) {
		m_Params.nRotaryFullStep = (nValue8 != 0);

		if (nValue8 != defaults::ROTARY_FULLSTEP) {
			m_Params.nSetList |= ltcdisplayparams::Mask::ROTARY_FULLSTEP;
		} else {
			m_Params.nSetList &= ~ltcdisplayparams::Mask::ROTARY_FULLSTEP;
		}
		return;
	}

	if (Sscan::Char(pLine, LtcDisplayParamsConst::MAX7219_TYPE, aBuffer, nLength) == Sscan::OK) {
		if (strncasecmp(aBuffer, "7segment", nLength) == 0) {
			m_Params.nMax7219Type = static_cast<uint8_t>(ltc::display::max7219::Types::SEGMENT);
			m_Params.nSetList |= ltcdisplayparams::Mask::MAX7219_TYPE;
		} else {
			m_Params.nMax7219Type = static_cast<uint8_t>(ltc::display::max7219::Types::MATRIX);
			m_Params.nSetList &= ~ltcdisplayparams::Mask::MAX7219_TYPE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::MAX7219_INTENSITY, nValue8) == Sscan::OK) {
		if (nValue8 <= 0x0F) {
			m_Params.nMax7219Intensity = nValue8;

			if (nValue8 != defaults::MAX7219_INTENSITY) {
				m_Params.nSetList |= ltcdisplayparams::Mask::MAX7219_INTENSITY;
			} else {
				m_Params.nSetList &= ~ltcdisplayparams::Mask::MAX7219_INTENSITY;
			}
		}
		return;
	}

	nLength = 8;

	if (Sscan::Char(pLine, LtcDisplayParamsConst::WS28XX_TYPE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		if (strncasecmp(aBuffer, "7segment", nLength) == 0) {
			m_Params.nWS28xxDisplayType = static_cast<uint8_t>(WS28xxType::SEGMENT);
			m_Params.nSetList |= ltcdisplayparams::Mask::WS28XX_DISPLAY_TYPE;
		} else if (strncasecmp(aBuffer, "matrix", nLength) == 0) {
			m_Params.nWS28xxDisplayType = static_cast<uint8_t>(WS28xxType::MATRIX);
			m_Params.nSetList |= ltcdisplayparams::Mask::WS28XX_DISPLAY_TYPE;
		}
		return;
	}

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	nLength = pixel::TYPES_MAX_NAME_LENGTH;

	if (Sscan::Char(pLine, DevicesParamsConst::TYPE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		const auto type = pixel::pixel_get_type(aBuffer);

		if (type != pixel::Type::UNDEFINED) {
			m_Params.nWS28xxType = static_cast<uint8_t>(type);
			m_Params.nSetList |= ltcdisplayparams::Mask::WS28XX_TYPE;
		} else {
			m_Params.nWS28xxType = static_cast<uint8_t>(pixel::defaults::TYPE);
			m_Params.nSetList &= ~ltcdisplayparams::Mask::WS28XX_TYPE;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::MAP, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		pixel::Map tMapping;
		if ((tMapping = pixel::pixel_get_map(aBuffer)) != pixel::Map::UNDEFINED) {
			m_Params.nWS28xxRgbMapping = static_cast<uint8_t>(tMapping);
			m_Params.nSetList |= ltcdisplayparams::Mask::WS28XX_RGB_MAPPING;
		}
		return;
	}
#endif

	if (Sscan::Uint8(pLine, LtcDisplayParamsConst::INTENSITY, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nDisplayRgbIntensity = nValue8;
			m_Params.nSetList |= ltcdisplayparams::Mask::DISPLAYRGB_INTENSITY;
		}
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, LtcDisplayParamsConst::COLON_BLINK_MODE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		for (uint32_t i = 0; i < (sizeof(aColonBlinkMode) / sizeof(aColonBlinkMode[0])); i++) {
			if (strcasecmp(aBuffer, aColonBlinkMode[i]) == 0) {
				m_Params.nDisplayRgbColonBlinkMode = static_cast<uint8_t>(i);
				m_Params.nSetList |= ltcdisplayparams::Mask::DISPLAYRGB_COLON_BLINK_MODE;
				return;
			}
		}
		return;
	}

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ColourIndex::LAST); nIndex++) {
		if(Sscan::Hex24Uint32(pLine, LtcDisplayParamsConst::COLOUR[nIndex], nValue32) == Sscan::OK) {
			m_Params.aDisplayRgbColour[nIndex] = nValue32;
			m_Params.nSetList |= (ltcdisplayparams::Mask::DISLAYRGB_COLOUR_INDEX << nIndex);
			return;
		}
	}

	nLength = sizeof(m_Params.aInfoMessage);
	if (Sscan::Char(pLine, LtcDisplayParamsConst::INFO_MSG, m_Params.aInfoMessage, nLength) == Sscan::OK) {
		for (; nLength < sizeof(m_Params.aInfoMessage); nLength++) {
			m_Params.aInfoMessage[nLength] = ' ';
		}
		m_Params.nSetList |= ltcdisplayparams::Mask::INFO_MSG;
		return;
	}
}

void LtcDisplayParams::Builder(const struct ltcdisplayparams::Params *ptLtcDisplayParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (ptLtcDisplayParams != nullptr) {
		memcpy(&m_Params, ptLtcDisplayParams, sizeof(struct ltcdisplayparams::Params));
	} else {
		LtcDisplayParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(LtcDisplayParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddComment("OLED SSD1306/11");
	builder.Add(LtcDisplayParamsConst::OLED_INTENSITY, m_Params.nOledIntensity, isMaskSet(ltcdisplayparams::Mask::OLED_INTENSITY));

	builder.AddComment("Rotary control");
	builder.Add(LtcDisplayParamsConst::ROTARY_FULLSTEP, m_Params.nRotaryFullStep, isMaskSet(ltcdisplayparams::Mask::ROTARY_FULLSTEP));

	builder.AddComment("MAX7219");
	builder.Add(LtcDisplayParamsConst::MAX7219_TYPE, m_Params.nMax7219Type == static_cast<uint8_t>(ltc::display::max7219::Types::SEGMENT) ? "7segment" : "matrix" , isMaskSet(ltcdisplayparams::Mask::MAX7219_TYPE));
	builder.Add(LtcDisplayParamsConst::MAX7219_INTENSITY, m_Params.nMax7219Intensity, isMaskSet(ltcdisplayparams::Mask::MAX7219_INTENSITY));

	builder.AddComment("RGB Display (generic)");
	builder.Add(LtcDisplayParamsConst::INTENSITY, m_Params.nDisplayRgbIntensity, isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_INTENSITY));
	builder.Add(LtcDisplayParamsConst::COLON_BLINK_MODE, m_Params.nDisplayRgbColonBlinkMode == static_cast<uint8_t>(ltcdisplayrgb::ColonBlinkMode::OFF) ? "off" : (m_Params.nDisplayRgbColonBlinkMode == static_cast<uint8_t>(ltcdisplayrgb::ColonBlinkMode::DOWN) ? "down" : "up") , isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_COLON_BLINK_MODE));

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ltcdisplayrgb::ColourIndex::LAST); nIndex++) {
		builder.AddHex24(LtcDisplayParamsConst::COLOUR[nIndex], m_Params.aDisplayRgbColour[nIndex],isMaskSet(ltcdisplayparams::Mask::DISLAYRGB_COLOUR_INDEX << nIndex));
	}

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	builder.AddComment("WS28xx (specific)");
	builder.Add(LtcDisplayParamsConst::WS28XX_TYPE, m_Params.nWS28xxDisplayType == static_cast<uint8_t>(ltcdisplayrgb::WS28xxType::SEGMENT) ? "7segment" : "matrix" , isMaskSet(ltcdisplayparams::Mask::WS28XX_DISPLAY_TYPE));
	builder.Add(DevicesParamsConst::TYPE, pixel::pixel_get_type(static_cast<pixel::Type>(m_Params.nWS28xxType)), isMaskSet(ltcdisplayparams::Mask::WS28XX_TYPE));

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(ltcdisplayparams::Mask::WS28XX_RGB_MAPPING)) {
		m_Params.nWS28xxRgbMapping = static_cast<uint8_t>(pixel::pixel_get_map(static_cast<pixel::Type>(m_Params.nWS28xxType)));
	}
	builder.Add(DevicesParamsConst::MAP, pixel::pixel_get_map(static_cast<pixel::Map>(m_Params.nWS28xxRgbMapping)), isMaskSet(ltcdisplayparams::Mask::WS28XX_RGB_MAPPING));
#endif

#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
	builder.AddComment("RGB panel (specific)");
	char aTemp[sizeof(m_Params.aInfoMessage) + 1];
	memcpy(aTemp, m_Params.aInfoMessage, sizeof(m_Params.aInfoMessage));
	aTemp[sizeof(m_Params.aInfoMessage)] = '\0';
	builder.Add(LtcDisplayParamsConst::INFO_MSG, aTemp, isMaskSet(ltcdisplayparams::Mask::INFO_MSG));
#endif

	nSize = builder.GetSize();
}

void LtcDisplayParams::Set(LtcDisplayRgb *pLtcDisplayRgb) {
	assert(pLtcDisplayRgb != nullptr);

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	if (isMaskSet(ltcdisplayparams::Mask::WS28XX_RGB_MAPPING)) {
		pLtcDisplayRgb->SetMapping(static_cast<pixel::Map>(m_Params.nWS28xxRgbMapping));
	}
#endif

	if (isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_INTENSITY)) {
		pLtcDisplayRgb->SetMaster(m_Params.nDisplayRgbIntensity);
	}

	if (isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_COLON_BLINK_MODE)) {
		pLtcDisplayRgb->SetColonBlinkMode(static_cast<ColonBlinkMode>(m_Params.nDisplayRgbColonBlinkMode));
	}

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ColourIndex::LAST); nIndex++) {
		if (isMaskSet((ltcdisplayparams::Mask::DISLAYRGB_COLOUR_INDEX << nIndex))) {
			pLtcDisplayRgb->SetColour(m_Params.aDisplayRgbColour[nIndex], static_cast<ltcdisplayrgb::ColourIndex>(nIndex));
		}
	}
}

void LtcDisplayParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcDisplayParams*>(p))->callbackFunction(s);
}

void LtcDisplayParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcDisplayParamsConst::FILE_NAME);

	if (isMaskSet(ltcdisplayparams::Mask::WS28XX_DISPLAY_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::WS28XX_TYPE, m_Params.nWS28xxDisplayType, m_Params.nWS28xxDisplayType == static_cast<uint8_t>(ltcdisplayrgb::WS28xxType::SEGMENT) ? "7segment" : "matrix");
	}

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	if (isMaskSet(ltcdisplayparams::Mask::WS28XX_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, pixel::pixel_get_type(static_cast<pixel::Type>(m_Params.nWS28xxType)), static_cast<int>(m_Params.nWS28xxType));
	}

	if (isMaskSet(ltcdisplayparams::Mask::WS28XX_RGB_MAPPING)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::MAP, pixel::pixel_get_map(static_cast<pixel::Map>(m_Params.nWS28xxRgbMapping)), static_cast<int>(m_Params.nWS28xxRgbMapping));
	}
#endif

	if (isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::INTENSITY, m_Params.nDisplayRgbIntensity);
	}

	if (isMaskSet(ltcdisplayparams::Mask::DISPLAYRGB_COLON_BLINK_MODE)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::COLON_BLINK_MODE, m_Params.nDisplayRgbColonBlinkMode);
	}

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ltcdisplayrgb::ColourIndex::LAST); nIndex++) {
		if (isMaskSet((ltcdisplayparams::Mask::DISLAYRGB_COLOUR_INDEX << nIndex))) {
			printf(" %s=%.6x\n", LtcDisplayParamsConst::COLOUR[nIndex], m_Params.aDisplayRgbColour[nIndex]);
		}
	}

	if (isMaskSet(ltcdisplayparams::Mask::GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, static_cast<int>(m_Params.nGlobalBrightness));
	}

	if (isMaskSet(ltcdisplayparams::Mask::MAX7219_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::MAX7219_TYPE, m_Params.nMax7219Type, m_Params.nMax7219Type == static_cast<uint8_t>(ltc::display::max7219::Types::SEGMENT) ? "7segment" : "matrix");
	}

	if (isMaskSet(ltcdisplayparams::Mask::MAX7219_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::MAX7219_INTENSITY, m_Params.nMax7219Intensity);
	}

	if (isMaskSet(ltcdisplayparams::Mask::OLED_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::OLED_INTENSITY, m_Params.nOledIntensity);
	}

	if (isMaskSet(ltcdisplayparams::Mask::ROTARY_FULLSTEP)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::ROTARY_FULLSTEP, m_Params.nRotaryFullStep);
	}
}
