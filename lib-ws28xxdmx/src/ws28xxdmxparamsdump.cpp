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

#include <stdint.h>
#include <algorithm>
#ifndef NDEBUG
# include <stdio.h>
#endif

#include "ws28xxdmxparams.h"
#include "devicesparamsconst.h"
#include "lightsetconst.h"

#include "ws28xx.h"
#include "ws28xxconst.h"
#include "ws28xxdmx.h"

#include "rgbmapping.h"

using namespace ws28xxdmxparams;

void WS28xxDmxParams::Dump() {
#ifndef NDEBUG
	if (m_tWS28xxParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DevicesParamsConst::FILE_NAME);

	if (isMaskSet(WS28xxDmxParamsMask::LED_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE, WS28xx::GetLedTypeString(static_cast<ws28xx::Type>(m_tWS28xxParams.tLedType)), static_cast<int>(m_tWS28xxParams.tLedType));
	}

	if (isMaskSet(WS28xxDmxParamsMask::RGB_MAPPING)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::LED_RGB_MAPPING, static_cast<int>(m_tWS28xxParams.nRgbMapping), RGBMapping::ToString(static_cast<rgbmapping::Map>(m_tWS28xxParams.nRgbMapping)));
	}

	if (isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T0H, WS28xx::ConvertTxH(m_tWS28xxParams.nLowCode), m_tWS28xxParams.nLowCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T1H, WS28xx::ConvertTxH(m_tWS28xxParams.nHighCode), m_tWS28xxParams.nHighCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::LED_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_COUNT, m_tWS28xxParams.nLedCount);
	}

	for (uint32_t i = 0; i < std::min(static_cast<size_t>(MAX_OUTPUTS), sizeof(LightSetConst::PARAMS_START_UNI_PORT) / sizeof(LightSetConst::PARAMS_START_UNI_PORT[0])); i++) {
		if (isMaskSet(WS28xxDmxParamsMask::START_UNI_PORT_1 << i)) {
				printf(" %s=%d\n", LightSetConst::PARAMS_START_UNI_PORT[i], m_tWS28xxParams.nStartUniverse[i]);
		}
	}

	if (isMaskSet(WS28xxDmxParamsMask::ACTIVE_OUT)) {
		printf(" %s=%d\n", DevicesParamsConst::ACTIVE_OUT, m_tWS28xxParams.nActiveOutputs);
	}

	if(isMaskSet(WS28xxDmxParamsMask::LED_GROUPING)) {
		printf(" %s=1 [Yes]\n", DevicesParamsConst::LED_GROUPING);
	}

	if (isMaskSet(WS28xxDmxParamsMask::LED_GROUP_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_GROUP_COUNT, m_tWS28xxParams.nLedGroupCount);
	}

	if (isMaskSet(WS28xxDmxParamsMask::SPI_SPEED)) {
		printf(" %s=%d\n", DevicesParamsConst::SPI_SPEED_HZ, m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, m_tWS28xxParams.nGlobalBrightness);
	}

	if (isMaskSet(WS28xxDmxParamsMask::DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_DMX_START_ADDRESS, m_tWS28xxParams.nDmxStartAddress);
	}

	if (isMaskSet(WS28xxDmxParamsMask::TEST_PATTERN)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_TEST_PATTERN, m_tWS28xxParams.nTestPattern);
	}
#endif
}

