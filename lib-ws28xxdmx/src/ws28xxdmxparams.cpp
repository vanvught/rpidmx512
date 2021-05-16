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
#include <cstring>
#include <algorithm>
#include <cassert>

#include "ws28xxdmxparams.h"

#include "pixeltype.h"
#include "ws28xxdmx.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "devicesparamsconst.h"

#include "debug.h"

using namespace ws28xxdmxparams;
using namespace pixel;
using namespace lightset;

WS28xxDmxParams::WS28xxDmxParams(WS28xxDmxParamsStore *pWS28XXStripeParamsStore): m_pWS28xxParamsStore(pWS28XXStripeParamsStore) {
	m_tWS28xxParams.nSetList = 0;
	m_tWS28xxParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
	m_tWS28xxParams.nCount = defaults::COUNT;
	m_tWS28xxParams.nDmxStartAddress = Dmx::START_ADDRESS_DEFAULT;
	m_tWS28xxParams.nSpiSpeedHz = spi::speed::ws2801::default_hz;
	m_tWS28xxParams.nGlobalBrightness = 0xFF;
	m_tWS28xxParams.nActiveOutputs = defaults::OUTPUT_PORTS;
	m_tWS28xxParams.nGroupingCount = 1;
	m_tWS28xxParams.nMap = static_cast<uint8_t>(Map::UNDEFINED);
	m_tWS28xxParams.nLowCode = 0;
	m_tWS28xxParams.nHighCode = 0;
	uint16_t nStartUniverse = 1;
	for (uint32_t i = 0; i < MAX_OUTPUTS; i++) {
#if defined (NODE_ARTNET)
		const auto nDiff = ((nStartUniverse + 4) & 0xF) - (nStartUniverse & 0xF);

		if (nDiff != 4) {
			nStartUniverse = static_cast<uint16_t>(nStartUniverse + nDiff + 15);
		}
#endif
		m_tWS28xxParams.nStartUniverse[i] = nStartUniverse;
		nStartUniverse += 4;
	}
	m_tWS28xxParams.nTestPattern = 0;
}

bool WS28xxDmxParams::Load() {
	m_tWS28xxParams.nSetList = 0;

	ReadConfigFile configfile(WS28xxDmxParams::staticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pWS28xxParamsStore != nullptr) {
			m_pWS28xxParamsStore->Update(&m_tWS28xxParams);
		}
	} else if (m_pWS28xxParamsStore != nullptr) {
		m_pWS28xxParamsStore->Copy(&m_tWS28xxParams);
	} else {
		return false;
	}

	return true;
}

void WS28xxDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pWS28xxParamsStore != nullptr);

	if (m_pWS28xxParamsStore == nullptr) {
		return;
	}

	m_tWS28xxParams.nSetList = 0;

	ReadConfigFile config(WS28xxDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pWS28xxParamsStore->Update(&m_tWS28xxParams);
}

void WS28xxDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint16_t nValue16;
	float fValue;
	char cBuffer[16];

	uint32_t nLength = TYPES_MAX_NAME_LENGTH;

	if (Sscan::Char(pLine, DevicesParamsConst::TYPE, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		const auto type = PixelType::GetType(cBuffer);

		if (type != pixel::Type::UNDEFINED) {
			m_tWS28xxParams.nType = static_cast<uint8_t>(type);
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::TYPE;
		} else {
			m_tWS28xxParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::TYPE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::COUNT, nValue16) == Sscan::OK) {
		if (nValue16 != 0 && nValue16 <= std::max(max::ledcount::RGB, max::ledcount::RGBW)) {
			m_tWS28xxParams.nCount = nValue16;
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::COUNT;
		} else {
			m_tWS28xxParams.nCount = defaults::COUNT;
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::COUNT;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::MAP, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';

		const auto map = PixelType::GetMap(cBuffer);

		if (map != Map::UNDEFINED) {
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::MAP;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::MAP;
		}

		m_tWS28xxParams.nMap = static_cast<uint8_t>(map);
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T0H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::LOW_CODE;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::LOW_CODE;
		}

		m_tWS28xxParams.nLowCode = nValue8;
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T1H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::HIGH_CODE;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::HIGH_CODE;
		}

		m_tWS28xxParams.nHighCode = nValue8;
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::GROUPING_COUNT, nValue16) == Sscan::OK) {
		if (nValue16 > 1 && nValue16 <= std::max(max::ledcount::RGB, max::ledcount::RGBW)) {
			m_tWS28xxParams.nGroupingCount = nValue16;
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::GROUPING_COUNT;
		} else {
			m_tWS28xxParams.nGroupingCount = 1;
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::GROUPING_COUNT;
		}
		return;
	}

	uint32_t nValue32;

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, nValue32) == Sscan::OK) {
		if (nValue32 != pixel::spi::speed::ws2801::default_hz) {
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::SPI_SPEED;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::SPI_SPEED;
		}
		m_tWS28xxParams.nSpiSpeedHz = nValue32;
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, nValue8) == Sscan::OK) {
		if ((nValue8 != 0) && (nValue8 != 0xFF)) {
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS;
			m_tWS28xxParams.nGlobalBrightness = nValue8;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS;
			m_tWS28xxParams.nGlobalBrightness = 0xFF;
		}
		return;
	}

#if defined (PARAMS_INLCUDE_ALL) || !defined(OUTPUT_PIXEL_MULTI)
	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && nValue16 <= (Dmx::UNIVERSE_SIZE) && (nValue16 != Dmx::START_ADDRESS_DEFAULT)) {
			m_tWS28xxParams.nDmxStartAddress = nValue16;
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::DMX_START_ADDRESS;
		} else {
			m_tWS28xxParams.nDmxStartAddress = Dmx::START_ADDRESS_DEFAULT;
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::DMX_START_ADDRESS;
		}
		return;
	}
#endif

#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_PIXEL_MULTI)
	for (uint32_t i = 0; i < std::min(static_cast<size_t>(MAX_OUTPUTS), sizeof(LightSetConst::PARAMS_START_UNI_PORT) / sizeof(LightSetConst::PARAMS_START_UNI_PORT[0])); i++) {
		if (Sscan::Uint16(pLine, LightSetConst::PARAMS_START_UNI_PORT[i], nValue16) == Sscan::OK) {
#if !defined (NODE_ARTNET)
			if (nValue16 > 0) {
#endif
				m_tWS28xxParams.nStartUniverse[i] = nValue16;
				m_tWS28xxParams.nSetList |= (WS28xxDmxParamsMask::START_UNI_PORT_1 << i);
#if !defined (NODE_ARTNET)
			} else {
				m_tWS28xxParams.nStartUniverse[i] = static_cast<uint16_t>(1 + (i * 4));
				m_tWS28xxParams.nSetList &= ~(WS28xxDmxParamsMask::START_UNI_PORT_1 << i);
			}
#endif
		}
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::ACTIVE_OUT, nValue8) == Sscan::OK) {
		if ((nValue8 > 0) &&  (nValue8 <= 8) &&  (nValue8 != pixel::defaults::OUTPUT_PORTS)) {
			m_tWS28xxParams.nActiveOutputs = nValue8;
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::ACTIVE_OUT;
		} else {
			m_tWS28xxParams.nActiveOutputs = pixel::defaults::OUTPUT_PORTS;
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::ACTIVE_OUT;
		}
		return;
	}
#endif

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_TEST_PATTERN, nValue8) == Sscan::OK) {
		if ((nValue8 != static_cast<uint8_t>(pixelpatterns::Pattern::NONE)) && (nValue8 < static_cast<uint8_t>(pixelpatterns::Pattern::LAST))) {
			m_tWS28xxParams.nTestPattern = nValue8;
			m_tWS28xxParams.nSetList |= WS28xxDmxParamsMask::TEST_PATTERN;
		} else {
			m_tWS28xxParams.nTestPattern = static_cast<uint8_t>(pixelpatterns::Pattern::NONE);
			m_tWS28xxParams.nSetList &= ~WS28xxDmxParamsMask::TEST_PATTERN;
		}
		return;
	}
}

void WS28xxDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<WS28xxDmxParams*>(p))->callbackFunction(s);
}
