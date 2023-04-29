/**
 * @file pixeldmxparams.cpp
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

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "pixeldmxparams.h"
#include "pixeltype.h"
#include "pixelpatterns.h"
#include "pixelconfiguration.h"
#include "gamma/gamma_tables.h"

#include "lightset.h"
#include "lightsetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "devicesparamsconst.h"

#include "debug.h"

using namespace pixel;
using namespace lightset;

#if defined (NODE_ARTNET_MULTI)
# define NODE_ARTNET
#endif

#if defined (NODE_ARTNET)
static uint16_t getStartUniverse(uint16_t universe) {
	const auto net = universe & (0x7F << 8);
	const auto sub = universe & (0x0F << 4);
	const auto calculated = universe + 3;
	const auto net2 = static_cast<uint16_t>(calculated & (0x7F << 8));
	const auto sub2 = static_cast<uint16_t>(calculated & (0x0F << 4));

	if ((net == net2) && (sub == sub2)) {
		return universe;
	}

	return static_cast<uint16_t>(net2 + sub2);
}
#endif

PixelDmxParams::PixelDmxParams(PixelDmxParamsStore *pPixelDmxParamsStore): m_pPixelDmxParamsStore(pPixelDmxParamsStore) {
	m_pixelDmxParams.nSetList = 0;
	m_pixelDmxParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
	m_pixelDmxParams.nCount = defaults::COUNT;
	m_pixelDmxParams.nDmxStartAddress = dmx::START_ADDRESS_DEFAULT;
	m_pixelDmxParams.nSpiSpeedHz = spi::speed::ws2801::default_hz;
	m_pixelDmxParams.nGlobalBrightness = 0xFF;
	m_pixelDmxParams.nActiveOutputs = defaults::OUTPUT_PORTS;
	m_pixelDmxParams.nGroupingCount = 1;
	m_pixelDmxParams.nMap = static_cast<uint8_t>(Map::UNDEFINED);
	m_pixelDmxParams.nLowCode = 0;
	m_pixelDmxParams.nHighCode = 0;
	m_pixelDmxParams.nGammaValue = 0;

	uint16_t nStartUniverse = 1;

	for (uint32_t i = 0; i < pixeldmxparams::MAX_PORTS; i++) {
		m_pixelDmxParams.nStartUniverse[i] = nStartUniverse;
#if defined (NODE_ARTNET)
		if (i > 0) {
			m_pixelDmxParams.nStartUniverse[i] = getStartUniverse(static_cast<uint16_t>(4 + m_pixelDmxParams.nStartUniverse[i-1]));
		}
#else
		nStartUniverse = static_cast<uint16_t>(nStartUniverse + 4);
#endif
	}
	m_pixelDmxParams.nTestPattern = 0;
}

bool PixelDmxParams::Load() {
	m_pixelDmxParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(PixelDmxParams::staticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pPixelDmxParamsStore != nullptr) {
			m_pPixelDmxParamsStore->Update(&m_pixelDmxParams);
		}
	} else
#endif
	if (m_pPixelDmxParamsStore != nullptr) {
		m_pPixelDmxParamsStore->Copy(&m_pixelDmxParams);
	} else {
		return false;
	}

	return true;
}

void PixelDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pPixelDmxParamsStore != nullptr);

	if (m_pPixelDmxParamsStore == nullptr) {
		return;
	}

	m_pixelDmxParams.nSetList = 0;

	ReadConfigFile config(PixelDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pPixelDmxParamsStore->Update(&m_pixelDmxParams);
}

void PixelDmxParams::callbackFunction(const char *pLine) {
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
			m_pixelDmxParams.nType = static_cast<uint8_t>(type);
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::TYPE;
		} else {
			m_pixelDmxParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::TYPE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::COUNT, nValue16) == Sscan::OK) {
		if (nValue16 != 0 && nValue16 <= std::max(max::ledcount::RGB, max::ledcount::RGBW)) {
			m_pixelDmxParams.nCount = nValue16;
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::COUNT;
		} else {
			m_pixelDmxParams.nCount = defaults::COUNT;
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::COUNT;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::MAP, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';

		const auto map = PixelType::GetMap(cBuffer);

		if (map != Map::UNDEFINED) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::MAP;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::MAP;
		}

		m_pixelDmxParams.nMap = static_cast<uint8_t>(map);
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T0H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::LOW_CODE;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::LOW_CODE;
		}

		m_pixelDmxParams.nLowCode = nValue8;
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T1H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::HIGH_CODE;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::HIGH_CODE;
		}

		m_pixelDmxParams.nHighCode = nValue8;
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::GROUPING_COUNT, nValue16) == Sscan::OK) {
		if (nValue16 > 1 && nValue16 <= std::max(max::ledcount::RGB, max::ledcount::RGBW)) {
			m_pixelDmxParams.nGroupingCount = nValue16;
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::GROUPING_COUNT;
		} else {
			m_pixelDmxParams.nGroupingCount = 1;
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::GROUPING_COUNT;
		}
		return;
	}

	uint32_t nValue32;

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, nValue32) == Sscan::OK) {
		if (nValue32 != pixel::spi::speed::ws2801::default_hz) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::SPI_SPEED;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::SPI_SPEED;
		}
		m_pixelDmxParams.nSpiSpeedHz = nValue32;
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, nValue8) == Sscan::OK) {
		if ((nValue8 != 0) && (nValue8 != 0xFF)) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::GLOBAL_BRIGHTNESS;
			m_pixelDmxParams.nGlobalBrightness = nValue8;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::GLOBAL_BRIGHTNESS;
			m_pixelDmxParams.nGlobalBrightness = 0xFF;
		}
		return;
	}

#if defined (PARAMS_INLCUDE_ALL) || !defined(OUTPUT_DMX_PIXEL_MULTI)
	if (Sscan::Uint16(pLine, LightSetParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && nValue16 <= (dmx::UNIVERSE_SIZE) && (nValue16 != dmx::START_ADDRESS_DEFAULT)) {
			m_pixelDmxParams.nDmxStartAddress = nValue16;
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::DMX_START_ADDRESS;
		} else {
			m_pixelDmxParams.nDmxStartAddress = dmx::START_ADDRESS_DEFAULT;
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::DMX_START_ADDRESS;
		}
		return;
	}
#endif

	for (uint32_t i = 0; i < pixeldmxparams::MAX_PORTS; i++) {
		if (Sscan::Uint16(pLine, LightSetParamsConst::START_UNI_PORT[i], nValue16) == Sscan::OK) {
#if !defined (NODE_ARTNET)
			if (nValue16 > 0) {
#endif
				m_pixelDmxParams.nStartUniverse[i] = nValue16;
				m_pixelDmxParams.nSetList |= (pixeldmxparams::Mask::START_UNI_PORT_1 << i);
#if !defined (NODE_ARTNET)
			} else {
				m_pixelDmxParams.nStartUniverse[i] = static_cast<uint16_t>(1 + (i * 4));
				m_pixelDmxParams.nSetList &= ~(pixeldmxparams::Mask::START_UNI_PORT_1 << i);
			}
#endif
		}
	}

#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_DMX_PIXEL_MULTI)
	if (Sscan::Uint8(pLine, DevicesParamsConst::ACTIVE_OUT, nValue8) == Sscan::OK) {
		if ((nValue8 > 0) &&  (nValue8 <= pixeldmxparams::MAX_PORTS) &&  (nValue8 != pixel::defaults::OUTPUT_PORTS)) {
			m_pixelDmxParams.nActiveOutputs = nValue8;
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::ACTIVE_OUT;
		} else {
			m_pixelDmxParams.nActiveOutputs = pixel::defaults::OUTPUT_PORTS;
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::ACTIVE_OUT;
		}
		return;
	}
#endif

	if (Sscan::Uint8(pLine, DevicesParamsConst::TEST_PATTERN, nValue8) == Sscan::OK) {
		if ((nValue8 != static_cast<uint8_t>(pixelpatterns::Pattern::NONE)) && (nValue8 < static_cast<uint8_t>(pixelpatterns::Pattern::LAST))) {
			m_pixelDmxParams.nTestPattern = nValue8;
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::TEST_PATTERN;
		} else {
			m_pixelDmxParams.nTestPattern = static_cast<uint8_t>(pixelpatterns::Pattern::NONE);
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::TEST_PATTERN;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GAMMA_CORRECTION, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_pixelDmxParams.nSetList |= pixeldmxparams::Mask::GAMMA_CORRECTION;
		} else {
			m_pixelDmxParams.nSetList &= ~pixeldmxparams::Mask::GAMMA_CORRECTION;
		}
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::GAMMA_VALUE, fValue) == Sscan::OK) {
		const uint8_t nValue = static_cast<uint8_t>(fValue * 10);
		if ((nValue < gamma::MIN) || (nValue > gamma::MAX)) {
			m_pixelDmxParams.nGammaValue = 0;
		} else {
			m_pixelDmxParams.nGammaValue = nValue;
		}
	}
}

void PixelDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<PixelDmxParams*>(p))->callbackFunction(s);
}

void PixelDmxParams::Builder(const struct pixeldmxparams::Params *ptWS28xxParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (ptWS28xxParams != nullptr) {
		memcpy(&m_pixelDmxParams, ptWS28xxParams, sizeof(struct pixeldmxparams::Params));
	} else {
		m_pPixelDmxParamsStore->Copy(&m_pixelDmxParams);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_pixelDmxParams.nType)), isMaskSet(pixeldmxparams::Mask::TYPE));
	builder.Add(DevicesParamsConst::COUNT, m_pixelDmxParams.nCount, isMaskSet(pixeldmxparams::Mask::COUNT));
	builder.Add(DevicesParamsConst::GAMMA_CORRECTION, isMaskSet(pixeldmxparams::Mask::GAMMA_CORRECTION));

	if (m_pixelDmxParams.nGammaValue == 0) {
		builder.Add(DevicesParamsConst::GAMMA_VALUE, "<default>", false);
	} else {
		builder.Add(DevicesParamsConst::GAMMA_VALUE, static_cast<float>(m_pixelDmxParams.nGammaValue) / 10, true);
	}

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(pixeldmxparams::Mask::MAP)) {
		m_pixelDmxParams.nMap = static_cast<uint8_t>(PixelType::GetMap(static_cast<pixel::Type>(m_pixelDmxParams.nType)));
	}
	builder.Add(DevicesParamsConst::MAP, PixelType::GetMap(static_cast<Map>(m_pixelDmxParams.nMap)), isMaskSet(pixeldmxparams::Mask::MAP));

	if (!isMaskSet(pixeldmxparams::Mask::LOW_CODE) || !isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
		uint8_t nLowCode;
		uint8_t nHighCode;

		PixelConfiguration::GetTxH(static_cast<pixel::Type>(m_pixelDmxParams.nType), nLowCode, nHighCode);

		if (!isMaskSet(pixeldmxparams::Mask::LOW_CODE)) {
			m_pixelDmxParams.nLowCode = nLowCode;
		}

		if (!isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
			m_pixelDmxParams.nHighCode = nHighCode;
		}
	}

	builder.AddComment("Overwrite timing (us)");
	builder.Add(DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_pixelDmxParams.nLowCode), isMaskSet(pixeldmxparams::Mask::LOW_CODE), 2);
	builder.Add(DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_pixelDmxParams.nHighCode), isMaskSet(pixeldmxparams::Mask::HIGH_CODE), 2);

	builder.AddComment("Grouping");
	builder.Add(DevicesParamsConst::GROUPING_COUNT, m_pixelDmxParams.nGroupingCount, isMaskSet(pixeldmxparams::Mask::GROUPING_COUNT));

	builder.AddComment("Clock based chips");
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_pixelDmxParams.nSpiSpeedHz, isMaskSet(pixeldmxparams::Mask::SPI_SPEED));

	builder.AddComment("APA102/SK9822");
	builder.Add(DevicesParamsConst::GLOBAL_BRIGHTNESS, m_pixelDmxParams.nGlobalBrightness, isMaskSet(pixeldmxparams::Mask::GLOBAL_BRIGHTNESS));

#if defined (PARAMS_INLCUDE_ALL) || !defined(OUTPUT_DMX_PIXEL_MULTI)
	builder.AddComment("DMX");
	builder.Add(LightSetParamsConst::DMX_START_ADDRESS, m_pixelDmxParams.nDmxStartAddress, isMaskSet(pixeldmxparams::Mask::DMX_START_ADDRESS));
#endif

	for (uint32_t i = 0; i < pixeldmxparams::MAX_PORTS; i++) {
		builder.Add(LightSetParamsConst::START_UNI_PORT[i],m_pixelDmxParams.nStartUniverse[i], isMaskSet(pixeldmxparams::Mask::START_UNI_PORT_1 << i));
	}
#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_DMX_PIXEL_MULTI)
	builder.Add(DevicesParamsConst::ACTIVE_OUT, m_pixelDmxParams.nActiveOutputs, isMaskSet(pixeldmxparams::Mask::ACTIVE_OUT));
#endif

	builder.AddComment("Test pattern");
	builder.Add(DevicesParamsConst::TEST_PATTERN, m_pixelDmxParams.nTestPattern, isMaskSet(pixeldmxparams::Mask::TEST_PATTERN));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
}

void PixelDmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pPixelDmxParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void PixelDmxParams::Set(PixelDmxConfiguration *pPixelDmxConfiguration) {
	assert(pPixelDmxConfiguration != nullptr);

	// Pixel

	if (isMaskSet(pixeldmxparams::Mask::TYPE)) {
		pPixelDmxConfiguration->SetType(static_cast<Type>(m_pixelDmxParams.nType));
	}

	if (isMaskSet(pixeldmxparams::Mask::COUNT)) {
		pPixelDmxConfiguration->SetCount(m_pixelDmxParams.nCount);
	}

	if (isMaskSet(pixeldmxparams::Mask::MAP)) {
		pPixelDmxConfiguration->SetMap(static_cast<Map>(m_pixelDmxParams.nMap));
	}

	if (isMaskSet(pixeldmxparams::Mask::LOW_CODE)) {
		pPixelDmxConfiguration->SetLowCode(m_pixelDmxParams.nLowCode);
	}

	if (isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
		pPixelDmxConfiguration->SetHighCode(m_pixelDmxParams.nHighCode);
	}

	if (isMaskSet(pixeldmxparams::Mask::SPI_SPEED)) {
		pPixelDmxConfiguration->SetClockSpeedHz(m_pixelDmxParams.nSpiSpeedHz);
	}

	if (isMaskSet(pixeldmxparams::Mask::GLOBAL_BRIGHTNESS)) {
		pPixelDmxConfiguration->SetGlobalBrightness(m_pixelDmxParams.nGlobalBrightness);
	}

	if (isMaskSet(pixeldmxparams::Mask::GAMMA_CORRECTION)) {
		pPixelDmxConfiguration->SetEnableGammaCorrection(true);
		if (m_pixelDmxParams.nGammaValue != 0) {
			pPixelDmxConfiguration->SetGammaTable(m_pixelDmxParams.nGammaValue);
		}
	}

	// Dmx

	if (isMaskSet(pixeldmxparams::Mask::DMX_START_ADDRESS)) {
		pPixelDmxConfiguration->SetDmxStartAddress(m_pixelDmxParams.nDmxStartAddress);
	}

	if (isMaskSet(pixeldmxparams::Mask::GROUPING_COUNT)) {
		pPixelDmxConfiguration->SetGroupingCount(m_pixelDmxParams.nGroupingCount);
	}

#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_DMX_PIXEL_MULTI)
	if (isMaskSet(pixeldmxparams::Mask::ACTIVE_OUT)) {
		pPixelDmxConfiguration->SetOutputPorts(m_pixelDmxParams.nActiveOutputs);
	}
#endif
}
