/**
 * @file pixeldmxparamsdump.cpp
 *
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NDEBUG
# include <cstdint>
# include <cstdio>
# include <algorithm>

# include "pixeltype.h"

# include "devicesparamsconst.h"
# include "lightsetparamsconst.h"
#endif

#include "pixeldmxparams.h"

using namespace pixeldmxparams;

void PixelDmxParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DevicesParamsConst::FILE_NAME);

	if (isMaskSet(Mask::TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_pixelDmxParams.nType)), static_cast<int>(m_pixelDmxParams.nType));
	}

	if (isMaskSet(Mask::MAP)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::MAP, static_cast<int>(m_pixelDmxParams.nMap), PixelType::GetMap(static_cast<pixel::Map>(m_pixelDmxParams.nMap)));
	}

	if (isMaskSet(Mask::LOW_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_pixelDmxParams.nLowCode), m_pixelDmxParams.nLowCode);
	}

	if (isMaskSet(Mask::HIGH_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_pixelDmxParams.nHighCode), m_pixelDmxParams.nHighCode);
	}

	if (isMaskSet(Mask::COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::COUNT, m_pixelDmxParams.nCount);
	}

	for (uint32_t i = 0; i < std::min(static_cast<size_t>(MAX_PORTS), sizeof(LightSetParamsConst::START_UNI_PORT) / sizeof(LightSetParamsConst::START_UNI_PORT[0])); i++) {
		if (isMaskSet(Mask::START_UNI_PORT_1 << i)) {
			printf(" %s=%d\n", LightSetParamsConst::START_UNI_PORT[i], m_pixelDmxParams.nStartUniverse[i]);
		}
	}

	if (isMaskSet(Mask::ACTIVE_OUT)) {
		printf(" %s=%d\n", DevicesParamsConst::ACTIVE_OUT, m_pixelDmxParams.nActiveOutputs);
	}

	if (isMaskSet(Mask::GROUPING_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::GROUPING_COUNT, m_pixelDmxParams.nGroupingCount);
	}

	if (isMaskSet(Mask::SPI_SPEED)) {
		printf(" %s=%d\n", DevicesParamsConst::SPI_SPEED_HZ, m_pixelDmxParams.nSpiSpeedHz);
	}

	if (isMaskSet(Mask::GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, m_pixelDmxParams.nGlobalBrightness);
	}

	if (isMaskSet(Mask::DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_pixelDmxParams.nDmxStartAddress);
	}

	if (isMaskSet(Mask::TEST_PATTERN)) {
		printf(" %s=%d\n", DevicesParamsConst::TEST_PATTERN, m_pixelDmxParams.nTestPattern);
	}

	if (isMaskSet(Mask::GAMMA_CORRECTION)) {
		printf(" %s=1 [Yes]\n", DevicesParamsConst::GAMMA_CORRECTION);
		printf(" %s=%1.1f [%u]\n", DevicesParamsConst::GAMMA_VALUE, static_cast<float>(m_pixelDmxParams.nGammaValue) / 10, m_pixelDmxParams.nGammaValue);
	}
#endif
}
