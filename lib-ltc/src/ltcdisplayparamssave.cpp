/**
 * @file ltcdisplayparamssave.cpp
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
#include <ltcdisplayrgb.h>
#include "ws28xx.h"
#include "ws28xxconst.h"
#include "rgbmapping.h"

#include "propertiesbuilder.h"

#include "debug.h"

void LtcDisplayParams::Builder(const struct TLtcDisplayParams *ptLtcDisplayParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (ptLtcDisplayParams != nullptr) {
		memcpy(&m_tLtcDisplayParams, ptLtcDisplayParams, sizeof(struct TLtcDisplayParams));
	} else {
		m_pLtcDisplayParamsStore->Copy(&m_tLtcDisplayParams);
	}

	PropertiesBuilder builder(LtcDisplayParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddComment("MAX7219");
	builder.Add(LtcDisplayParamsConst::MAX7219_TYPE, m_tLtcDisplayParams.nMax7219Type == LTCDISPLAYMAX7219_TYPE_7SEGMENT ? "7segment" : "matrix" , isMaskSet(LtcDisplayParamsMask::MAX7219_TYPE));
	builder.Add(LtcDisplayParamsConst::MAX7219_INTENSITY, m_tLtcDisplayParams.nMax7219Intensity, isMaskSet(LtcDisplayParamsMask::MAX7219_INTENSITY));

	builder.AddComment("RGB Display (generic)");
	builder.Add(LtcDisplayParamsConst::INTENSITY, m_tLtcDisplayParams.nDisplayRgbIntensity, isMaskSet(LtcDisplayParamsMask::DISPLAYRGB_INTENSITY));
	builder.Add(LtcDisplayParamsConst::COLON_BLINK_MODE, m_tLtcDisplayParams.nDisplayRgbColonBlinkMode == static_cast<uint8_t>(ltcdisplayrgb::ColonBlinkMode::OFF) ? "off" : (m_tLtcDisplayParams.nDisplayRgbColonBlinkMode == static_cast<uint8_t>(ltcdisplayrgb::ColonBlinkMode::DOWN) ? "down" : "up") , isMaskSet(LtcDisplayParamsMask::DISPLAYRGB_COLON_BLINK_MODE));

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(ltcdisplayrgb::ColourIndex::LAST); nIndex++) {
		builder.AddHex24(LtcDisplayParamsConst::COLOUR[nIndex], m_tLtcDisplayParams.aDisplayRgbColour[nIndex],isMaskSet(LtcDisplayParamsMask::DISLAYRGB_COLOUR_INDEX << nIndex));
	}

	builder.AddComment("WS28xx (specific)");
	builder.Add(LtcDisplayParamsConst::WS28XX_TYPE, m_tLtcDisplayParams.nWS28xxDisplayType == static_cast<uint8_t>(ltcdisplayrgb::WS28xxType::SEGMENT) ? "7segment" : "matrix" , isMaskSet(LtcDisplayParamsMask::WS28XX_DISPLAY_TYPE));
	builder.Add(DevicesParamsConst::LED_TYPE, WS28xx::GetLedTypeString(static_cast<TWS28XXType>(m_tLtcDisplayParams.nWS28xxLedType)), isMaskSet(LtcDisplayParamsMask::WS28XX_LED_TYPE));

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(LtcDisplayParamsMask::WS28XX_RGB_MAPPING)) {
		m_tLtcDisplayParams.nWS28xxRgbMapping = WS28xx::GetRgbMapping(static_cast<TWS28XXType>(m_tLtcDisplayParams.nWS28xxLedType));
	}
	builder.Add(DevicesParamsConst::LED_RGB_MAPPING, RGBMapping::ToString(static_cast<TRGBMapping>(m_tLtcDisplayParams.nWS28xxRgbMapping)), isMaskSet(LtcDisplayParamsMask::WS28XX_RGB_MAPPING));

	builder.AddComment("RGB panel (specific)");
	char aTemp[sizeof(m_tLtcDisplayParams.aInfoMessage) + 1];
	memcpy(aTemp, m_tLtcDisplayParams.aInfoMessage, sizeof(m_tLtcDisplayParams.aInfoMessage));
	aTemp[sizeof(m_tLtcDisplayParams.aInfoMessage)] = '\0';
	builder.Add(LtcDisplayParamsConst::INFO_MSG, aTemp, isMaskSet(LtcDisplayParamsMask::INFO_MSG));

	nSize = builder.GetSize();
}

void LtcDisplayParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pLtcDisplayParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
