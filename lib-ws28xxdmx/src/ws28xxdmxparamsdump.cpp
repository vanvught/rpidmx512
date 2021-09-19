/**
 * @file ws28xxparams.cpp
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#ifndef NDEBUG
# include <cstdio>
#endif

#include "ws28xxdmxparams.h"
#include "devicesparamsconst.h"
#include "lightsetparamsconst.h"

#include "pixeltype.h"
#include "ws28xxdmx.h"

using namespace ws28xxdmxparams;

void WS28xxDmxParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DevicesParamsConst::FILE_NAME);

	if (isMaskSet(WS28xxDmxParamsMask::TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_tWS28xxParams.nType)), static_cast<int>(m_tWS28xxParams.nType));
	}

	if (isMaskSet(WS28xxDmxParamsMask::MAP)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::MAP, static_cast<int>(m_tWS28xxParams.nMap), PixelType::GetMap(static_cast<pixel::Map>(m_tWS28xxParams.nMap)));
	}

	if (isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_tWS28xxParams.nLowCode), m_tWS28xxParams.nLowCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_tWS28xxParams.nHighCode), m_tWS28xxParams.nHighCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::COUNT, m_tWS28xxParams.nCount);
	}

	for (uint32_t i = 0; i < std::min(static_cast<size_t>(MAX_OUTPUTS), sizeof(LightSetParamsConst::START_UNI_PORT) / sizeof(LightSetParamsConst::START_UNI_PORT[0])); i++) {
		if (isMaskSet(WS28xxDmxParamsMask::START_UNI_PORT_1 << i)) {
			printf(" %s=%d\n", LightSetParamsConst::START_UNI_PORT[i], m_tWS28xxParams.nStartUniverse[i]);
		}
	}

	if (isMaskSet(WS28xxDmxParamsMask::ACTIVE_OUT)) {
		printf(" %s=%d\n", DevicesParamsConst::ACTIVE_OUT, m_tWS28xxParams.nActiveOutputs);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GROUPING_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::GROUPING_COUNT, m_tWS28xxParams.nGroupingCount);
	}

	if (isMaskSet(WS28xxDmxParamsMask::SPI_SPEED)) {
		printf(" %s=%d\n", DevicesParamsConst::SPI_SPEED_HZ, m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, m_tWS28xxParams.nGlobalBrightness);
	}

	if (isMaskSet(WS28xxDmxParamsMask::DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_tWS28xxParams.nDmxStartAddress);
	}

	if (isMaskSet(WS28xxDmxParamsMask::TEST_PATTERN)) {
		printf(" %s=%d\n", LightSetParamsConst::TEST_PATTERN, m_tWS28xxParams.nTestPattern);
	}
#endif
}

