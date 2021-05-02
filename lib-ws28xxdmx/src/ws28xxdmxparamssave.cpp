/**
 * @file ws28xxparamssave.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "ws28xxdmxparams.h"
#include "pixeltype.h"
#include "pixelconfiguration.h"

#include "propertiesbuilder.h"

#include "devicesparamsconst.h"
#include "lightsetconst.h"

#include "debug.h"

using namespace ws28xxdmxparams;
using namespace pixel;

void WS28xxDmxParams::Builder(const struct TWS28xxDmxParams *ptWS28xxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptWS28xxParams != nullptr) {
		memcpy(&m_tWS28xxParams, ptWS28xxParams, sizeof(struct TWS28xxDmxParams));
	} else {
		m_pWS28xxParamsStore->Copy(&m_tWS28xxParams);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_tWS28xxParams.nType)), isMaskSet(WS28xxDmxParamsMask::TYPE));
	builder.Add(DevicesParamsConst::COUNT, m_tWS28xxParams.nCount, isMaskSet(WS28xxDmxParamsMask::COUNT));

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(WS28xxDmxParamsMask::MAP)) {
		m_tWS28xxParams.nMap = static_cast<uint8_t>(PixelConfiguration::GetRgbMapping(static_cast<pixel::Type>(m_tWS28xxParams.nType)));
	}
	builder.Add(DevicesParamsConst::MAP, PixelType::GetMap(static_cast<Map>(m_tWS28xxParams.nMap)), isMaskSet(WS28xxDmxParamsMask::MAP));

	if (!isMaskSet(WS28xxDmxParamsMask::LOW_CODE) || !isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		uint8_t nLowCode;
		uint8_t nHighCode;

		PixelConfiguration::GetTxH(static_cast<pixel::Type>(m_tWS28xxParams.nType), nLowCode, nHighCode);

		if (!isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
			m_tWS28xxParams.nLowCode = nLowCode;
		}


		if (!isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
			m_tWS28xxParams.nHighCode = nHighCode;
		}
	}

	builder.AddComment("Overwrite timing (us)");
	builder.Add(DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_tWS28xxParams.nLowCode), isMaskSet(WS28xxDmxParamsMask::LOW_CODE), 2);
	builder.Add(DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_tWS28xxParams.nHighCode), isMaskSet(WS28xxDmxParamsMask::HIGH_CODE), 2);

	builder.AddComment("Grouping");
	builder.Add(DevicesParamsConst::GROUPING_ENABLED, isMaskSet(WS28xxDmxParamsMask::GROUPING_ENABLED));
	builder.Add(DevicesParamsConst::GROUPING_COUNT, m_tWS28xxParams.nGroupingCount, isMaskSet(WS28xxDmxParamsMask::GROUPING_COUNT));

#if defined (PARAMS_INLCUDE_ALL) || !defined(OUTPUT_PIXEL_MULTI)
	builder.AddComment("DMX");
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tWS28xxParams.nDmxStartAddress, isMaskSet(WS28xxDmxParamsMask::DMX_START_ADDRESS));

	builder.AddComment("Clock based chips");
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_tWS28xxParams.nSpiSpeedHz, isMaskSet(WS28xxDmxParamsMask::SPI_SPEED));

	builder.AddComment("APA102");
	builder.Add(DevicesParamsConst::GLOBAL_BRIGHTNESS, m_tWS28xxParams.nGlobalBrightness, isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS));
#endif

#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_PIXEL_MULTI)
	for (uint32_t i = 0; i < std::min(static_cast<size_t>(MAX_OUTPUTS), sizeof(LightSetConst::PARAMS_START_UNI_PORT) / sizeof(LightSetConst::PARAMS_START_UNI_PORT[0])); i++) {
		builder.Add(LightSetConst::PARAMS_START_UNI_PORT[i],m_tWS28xxParams.nStartUniverse[i], isMaskSet(WS28xxDmxParamsMask::START_UNI_PORT_1 << i));
	}
	builder.Add(DevicesParamsConst::ACTIVE_OUT, m_tWS28xxParams.nActiveOutputs, isMaskSet(WS28xxDmxParamsMask::ACTIVE_OUT));
#endif

	builder.AddComment("Test pattern");
	builder.Add(LightSetConst::PARAMS_TEST_PATTERN, m_tWS28xxParams.nTestPattern, isMaskSet(WS28xxDmxParamsMask::TEST_PATTERN));

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
