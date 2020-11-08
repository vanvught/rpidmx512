/**
 * @file ws28xxparamssave.cpp
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

#include "ws28xxdmxparams.h"
#include "ws28xx.h"

#include "propertiesbuilder.h"

#include "devicesparamsconst.h"
#include "lightsetconst.h"

#include "debug.h"

void WS28xxDmxParams::Builder(const struct TWS28xxDmxParams *ptWS28xxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptWS28xxParams != nullptr) {
		memcpy(&m_tWS28xxParams, ptWS28xxParams, sizeof(struct TWS28xxDmxParams));
	} else {
		m_pWS28xxParamsStore->Copy(&m_tWS28xxParams);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::LED_TYPE, WS28xx::GetLedTypeString(m_tWS28xxParams.tLedType), isMaskSet(WS28xxDmxParamsMask::LED_TYPE));
	builder.Add(DevicesParamsConst::LED_COUNT, m_tWS28xxParams.nLedCount, isMaskSet(WS28xxDmxParamsMask::LED_COUNT));

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(WS28xxDmxParamsMask::RGB_MAPPING)) {
		m_tWS28xxParams.nRgbMapping = WS28xx::GetRgbMapping(m_tWS28xxParams.tLedType);
	}
	builder.Add(DevicesParamsConst::LED_RGB_MAPPING, RGBMapping::ToString(static_cast<TRGBMapping>(m_tWS28xxParams.nRgbMapping)), isMaskSet(WS28xxDmxParamsMask::RGB_MAPPING));

	if (!isMaskSet(WS28xxDmxParamsMask::LOW_CODE) || !isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		uint8_t nLowCode;
		uint8_t nHighCode;

		WS28xx::GetTxH(m_tWS28xxParams.tLedType, nLowCode, nHighCode);

		if (!isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
			m_tWS28xxParams.nLowCode = nLowCode;
		}


		if (!isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
			m_tWS28xxParams.nHighCode = nHighCode;
		}
	}

	builder.AddComment("Overwrite timing (us)");
	builder.Add(DevicesParamsConst::LED_T0H, WS28xx::ConvertTxH(m_tWS28xxParams.nLowCode), isMaskSet(WS28xxDmxParamsMask::LOW_CODE), 2);
	builder.Add(DevicesParamsConst::LED_T1H, WS28xx::ConvertTxH(m_tWS28xxParams.nHighCode), isMaskSet(WS28xxDmxParamsMask::HIGH_CODE), 2);

	builder.AddComment("Grouping");
	builder.Add(DevicesParamsConst::LED_GROUPING, m_tWS28xxParams.bLedGrouping, isMaskSet(WS28xxDmxParamsMask::LED_GROUPING));
	builder.Add(DevicesParamsConst::LED_GROUP_COUNT, m_tWS28xxParams.nLedGroupCount, isMaskSet(WS28xxDmxParamsMask::LED_GROUP_COUNT));

	builder.AddComment("DMX");
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tWS28xxParams.nDmxStartAddress, isMaskSet(WS28xxDmxParamsMask::DMX_START_ADDRESS));

	builder.AddComment("Clock based chips");
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_tWS28xxParams.nSpiSpeedHz, isMaskSet(WS28xxDmxParamsMask::SPI_SPEED));

	builder.AddComment("APA102");
	builder.Add(DevicesParamsConst::GLOBAL_BRIGHTNESS, m_tWS28xxParams.nGlobalBrightness, isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS));

	builder.AddComment("Multi port");
	builder.Add(DevicesParamsConst::ACTIVE_OUT, m_tWS28xxParams.nActiveOutputs, isMaskSet(WS28xxDmxParamsMask::ACTIVE_OUT));
	builder.Add(DevicesParamsConst::USE_SI5351A, m_tWS28xxParams.bUseSI5351A, isMaskSet(WS28xxDmxParamsMask::USE_SI5351A));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void WS28xxDmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pWS28xxParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
