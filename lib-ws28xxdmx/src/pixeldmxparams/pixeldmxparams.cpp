/**
 * @file pixeldmxparams.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_PIXELDMX)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <algorithm>
#include <cassert>

#include "pixeldmxparams.h"
#include "pixeldmxparamsconst.h"
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixelpatterns.h"
#include "pixelconfiguration.h"

#include "gamma/gamma_tables.h"

#include "dmxnode.h"
#include "dmxnodeparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "devicesparamsconst.h"

#include "debug.h"

namespace pixeldmxparams::store {
	static void Update(const struct pixeldmxparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, pParams, sizeof(struct pixeldmxparams::Params));
	}

	static void Copy(struct pixeldmxparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::WS28XXDMX, pParams, sizeof(struct pixeldmxparams::Params));
	}
}  // namespace pixeldmxparams::store

PixelDmxParams::PixelDmxParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void PixelDmxParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(PixelDmxParams::StaticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		pixeldmxparams::store::Update(&m_Params);
	} else
#endif
		pixeldmxparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void PixelDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(m_Params));

	ReadConfigFile config(PixelDmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	pixeldmxparams::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void PixelDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char cBuffer[16];

	uint32_t nLength = pixel::TYPES_MAX_NAME_LENGTH;

	if (Sscan::Char(pLine, DevicesParamsConst::TYPE, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		const auto type = pixel::pixel_get_type(cBuffer);

		if (type != pixel::Type::UNDEFINED) {
			m_Params.nType = static_cast<uint8_t>(type);
			m_Params.nSetList |= pixeldmxparams::Mask::TYPE;
		}

		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, DevicesParamsConst::COUNT, nValue16) == Sscan::OK) {
		if (nValue16 != 0 && nValue16 <= std::max(pixel::max::ledcount::RGB, pixel::max::ledcount::RGBW)) {
			m_Params.nCount = nValue16;
			m_Params.nSetList |= pixeldmxparams::Mask::COUNT;
		}

		return;
	}

	nLength = 3;

	if (Sscan::Char(pLine, DevicesParamsConst::MAP, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';

		const auto map = pixel::pixel_get_map(cBuffer);

		if (map != pixel::Map::UNDEFINED) {
			m_Params.nMap = static_cast<uint8_t>(map);
			m_Params.nSetList |= pixeldmxparams::Mask::MAP;
		}

		return;
	}

	float fValue;
	uint8_t nValue8;

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T0H, fValue) == Sscan::OK) {
		if ((nValue8 = pixel::pixel_convert_TxH(fValue)) != 0) {
			m_Params.nLowCode = nValue8;
			m_Params.nSetList |= pixeldmxparams::Mask::LOW_CODE;
		}

		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T1H, fValue) == Sscan::OK) {
		if ((nValue8 = pixel::pixel_convert_TxH(fValue)) != 0) {
			m_Params.nHighCode = nValue8;
			m_Params.nSetList |= pixeldmxparams::Mask::HIGH_CODE;
		}

		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::GROUPING_COUNT, nValue16) == Sscan::OK) {
		if (nValue16 > 1 && nValue16 <= std::max(pixel::max::ledcount::RGB, pixel::max::ledcount::RGBW)) {
			m_Params.nGroupingCount = nValue16;
			m_Params.nSetList |= pixeldmxparams::Mask::GROUPING_COUNT;
		}

		return;
	}

	uint32_t nValue32;

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, nValue32) == Sscan::OK) {
		if (nValue32 != pixel::spi::speed::ws2801::default_hz) {
			m_Params.nSpiSpeedHz = nValue32;
			m_Params.nSetList |= pixeldmxparams::Mask::SPI_SPEED;
		}

		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, nValue8) == Sscan::OK) {
		if ((nValue8 != 0) && (nValue8 != 0xFF)) {
			m_Params.nSetList |= pixeldmxparams::Mask::GLOBAL_BRIGHTNESS;
			m_Params.nGlobalBrightness = nValue8;
		}

		return;
	}

#if !defined(OUTPUT_DMX_PIXEL_MULTI)
	if (Sscan::Uint16(pLine, DmxNodeParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 <= dmxnode::UNIVERSE_SIZE)) {
			m_Params.nDmxStartAddress = nValue16;
			m_Params.nSetList |= pixeldmxparams::Mask::DMX_START_ADDRESS;
		}

		return;
	}
#endif

	for (uint32_t i = 0; i < pixeldmxparams::MAX_PORTS; i++) {
		if (Sscan::Uint16(pLine, PixelDmxParamsConst::START_UNI_PORT[i], nValue16) == Sscan::OK) {
			if (nValue16 > 0) {
				m_Params.nStartUniverse[i] = nValue16;
				m_Params.nSetList |= (pixeldmxparams::Mask::START_UNI_PORT_1 << i);
			} else {
				m_Params.nStartUniverse[i] = static_cast<uint16_t>(1 + (i * 4));
				m_Params.nSetList &= ~(pixeldmxparams::Mask::START_UNI_PORT_1 << i);
			}
		}
	}

#if defined(OUTPUT_DMX_PIXEL_MULTI)
	if (Sscan::Uint8(pLine, DevicesParamsConst::ACTIVE_OUT, nValue8) == Sscan::OK) {
		if ((nValue8 > 0) &&  (nValue8 <= pixeldmxparams::MAX_PORTS) &&  (nValue8 != pixel::defaults::OUTPUT_PORTS)) {
			m_Params.nActiveOutputs = nValue8;
			m_Params.nSetList |= pixeldmxparams::Mask::ACTIVE_OUT;
		}

		return;
	}
#endif

	if (Sscan::Uint8(pLine, DevicesParamsConst::TEST_PATTERN, nValue8) == Sscan::OK) {
		if (nValue8 < static_cast<uint8_t>(pixelpatterns::Pattern::LAST)) {
			m_Params.nTestPattern = nValue8;
			m_Params.nSetList |= pixeldmxparams::Mask::TEST_PATTERN;
		}

		return;
	}

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	if (Sscan::Uint8(pLine, DevicesParamsConst::GAMMA_CORRECTION, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= pixeldmxparams::Mask::GAMMA_CORRECTION;
		} else {
			m_Params.nSetList &= ~pixeldmxparams::Mask::GAMMA_CORRECTION;
		}
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::GAMMA_VALUE, fValue) == Sscan::OK) {
		const auto nValue = static_cast<uint8_t>(fValue * 10);
		if ((nValue < gamma::MIN) || (nValue > gamma::MAX)) {
			m_Params.nGammaValue = 0;
		} else {
			m_Params.nGammaValue = nValue;
		}
		return;
	}
#endif
}

void PixelDmxParams::Builder(const struct pixeldmxparams::Params *ptWS28xxParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	auto& pixelDmxConfiguration = PixelDmxConfiguration::Get();

	if (ptWS28xxParams != nullptr) {
		memcpy(&m_Params, ptWS28xxParams, sizeof(struct pixeldmxparams::Params));
	} else {
		pixeldmxparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(pixeldmxparams::Mask::TYPE)) {
		m_Params.nType = static_cast<uint8_t>(pixelDmxConfiguration.GetType());
	}

	builder.Add(DevicesParamsConst::TYPE, pixel::pixel_get_type(static_cast<pixel::Type>(m_Params.nType)));

	if (m_Params.nCount == 0) {
		m_Params.nCount = pixelDmxConfiguration.GetCount();
	}

	builder.Add(DevicesParamsConst::COUNT, m_Params.nCount);

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	builder.Add(DevicesParamsConst::GAMMA_CORRECTION, isMaskSet(pixeldmxparams::Mask::GAMMA_CORRECTION));

	if (m_Params.nGammaValue == 0) {
		builder.Add(DevicesParamsConst::GAMMA_VALUE, "<default>", false);
	} else {
		builder.Add(DevicesParamsConst::GAMMA_VALUE, static_cast<float>(m_Params.nGammaValue) / 10, true);
	}
#endif

	if (!isMaskSet(pixeldmxparams::Mask::MAP)) {
		m_Params.nMap = static_cast<uint8_t>(pixelDmxConfiguration.GetMap());
	}

	builder.AddComment("Overwrite datasheet");
	builder.Add(DevicesParamsConst::MAP, pixel::pixel_get_map(static_cast<pixel::Map>(m_Params.nMap)));

	if (!isMaskSet(pixeldmxparams::Mask::LOW_CODE) || !isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
		uint8_t nLowCode;
		uint8_t nHighCode;

		PixelConfiguration::Get().GetTxH(static_cast<pixel::Type>(m_Params.nType), nLowCode, nHighCode);

		if (!isMaskSet(pixeldmxparams::Mask::LOW_CODE)) {
			m_Params.nLowCode = nLowCode;
		}

		if (!isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
			m_Params.nHighCode = nHighCode;
		}
	}

	builder.AddComment("Overwrite timing (us)");
	builder.Add(DevicesParamsConst::LED_T0H, pixel::pixel_convert_TxH(m_Params.nLowCode), true, 2);
	builder.Add(DevicesParamsConst::LED_T1H, pixel::pixel_convert_TxH(m_Params.nHighCode), true, 2);

	if (m_Params.nGroupingCount == 0) {
		m_Params.nGroupingCount = pixelDmxConfiguration.GetGroupingCount();
	}

	builder.AddComment("Grouping");
	builder.Add(DevicesParamsConst::GROUPING_COUNT, m_Params.nGroupingCount);

	if (m_Params.nSpiSpeedHz == 0) {
		m_Params.nSpiSpeedHz = pixelDmxConfiguration.GetClockSpeedHz();
	}

	builder.AddComment("Clock based chips");
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_Params.nSpiSpeedHz);

	if (m_Params.nGlobalBrightness == 0) {
		m_Params.nGlobalBrightness = pixelDmxConfiguration.GetGlobalBrightness();
	}

	builder.AddComment("APA102/SK9822");
	builder.Add(DevicesParamsConst::GLOBAL_BRIGHTNESS, m_Params.nGlobalBrightness);

#if !defined(OUTPUT_DMX_PIXEL_MULTI)
	if (m_Params.nDmxStartAddress == 0) {
		m_Params.nDmxStartAddress = pixelDmxConfiguration.GetDmxStartAddress();
	}

	builder.AddComment("DMX");
	builder.Add(DmxNodeParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);
#endif

	for (uint32_t i = 0; i < pixeldmxparams::MAX_PORTS; i++) {
		builder.Add(PixelDmxParamsConst::START_UNI_PORT[i],m_Params.nStartUniverse[i], isMaskSet(pixeldmxparams::Mask::START_UNI_PORT_1 << i));
	}

#if defined(OUTPUT_DMX_PIXEL_MULTI)
	if (m_Params.nActiveOutputs == 0) {
		m_Params.nActiveOutputs = pixelDmxConfiguration.GetOutputPorts();
	}

	builder.Add(DevicesParamsConst::ACTIVE_OUT, m_Params.nActiveOutputs);
#endif

	builder.AddComment("Test pattern");
	builder.Add(DevicesParamsConst::TEST_PATTERN, m_Params.nTestPattern);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
}

void PixelDmxParams::Set() {
	// Pixel
	auto& pixelConfiguration = PixelConfiguration::Get();

	if (isMaskSet(pixeldmxparams::Mask::TYPE)) {
		pixelConfiguration.SetType(static_cast<pixel::Type>(m_Params.nType));
	}

	if (isMaskSet(pixeldmxparams::Mask::COUNT)) {
		pixelConfiguration.SetCount(m_Params.nCount);
	}

	if (isMaskSet(pixeldmxparams::Mask::MAP)) {
		pixelConfiguration.SetMap(static_cast<pixel::Map>(m_Params.nMap));
	}

	if (isMaskSet(pixeldmxparams::Mask::LOW_CODE)) {
		pixelConfiguration.SetLowCode(m_Params.nLowCode);
	}

	if (isMaskSet(pixeldmxparams::Mask::HIGH_CODE)) {
		pixelConfiguration.SetHighCode(m_Params.nHighCode);
	}

	if (isMaskSet(pixeldmxparams::Mask::SPI_SPEED)) {
		pixelConfiguration.SetClockSpeedHz(m_Params.nSpiSpeedHz);
	}

	if (isMaskSet(pixeldmxparams::Mask::GLOBAL_BRIGHTNESS)) {
		pixelConfiguration.SetGlobalBrightness(m_Params.nGlobalBrightness);
	}

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	if (isMaskSet(pixeldmxparams::Mask::GAMMA_CORRECTION)) {
		pixelConfiguration.SetEnableGammaCorrection(true);
		if (m_Params.nGammaValue != 0) {
			pixelConfiguration.SetGammaTable(m_Params.nGammaValue);
		}
	}
#endif

	// Dmx
	auto& pixelDmxConfiguration = PixelDmxConfiguration::Get();

#if !defined(OUTPUT_DMX_PIXEL_MULTI)
	if (isMaskSet(pixeldmxparams::Mask::DMX_START_ADDRESS)) {
		pixelDmxConfiguration.SetDmxStartAddress(m_Params.nDmxStartAddress);
	}
#endif

	if (isMaskSet(pixeldmxparams::Mask::GROUPING_COUNT)) {
		pixelDmxConfiguration.SetGroupingCount(m_Params.nGroupingCount);
	}

#if defined(OUTPUT_DMX_PIXEL_MULTI)
	if (isMaskSet(pixeldmxparams::Mask::ACTIVE_OUT)) {
		pixelDmxConfiguration.SetOutputPorts(m_Params.nActiveOutputs);
	}
#endif
}

void PixelDmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<PixelDmxParams *>(p))->callbackFunction(s);
}

void PixelDmxParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DevicesParamsConst::FILE_NAME);
	printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, pixel::pixel_get_type(static_cast<pixel::Type>(m_Params.nType)), static_cast<int>(m_Params.nType));
	printf(" %s=%d [%s]\n", DevicesParamsConst::MAP, static_cast<int>(m_Params.nMap), pixel::pixel_get_map(static_cast<pixel::Map>(m_Params.nMap)));
	printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T0H, pixel::pixel_convert_TxH(m_Params.nLowCode), m_Params.nLowCode);
	printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T1H, pixel::pixel_convert_TxH(m_Params.nHighCode), m_Params.nHighCode);
	printf(" %s=%d\n", DevicesParamsConst::COUNT, m_Params.nCount);
	printf(" %s=%d\n", DevicesParamsConst::GROUPING_COUNT, m_Params.nGroupingCount);

	for (uint32_t i = 0; i < std::min(static_cast<size_t>(pixelpatterns::MAX_PORTS), sizeof(PixelDmxParamsConst::START_UNI_PORT) / sizeof(PixelDmxParamsConst::START_UNI_PORT[0])); i++) {
		printf(" %s=%d\n", PixelDmxParamsConst::START_UNI_PORT[i], m_Params.nStartUniverse[i]);
	}

	printf(" %s=%d\n", DevicesParamsConst::ACTIVE_OUT, m_Params.nActiveOutputs);
	printf(" %s=%u\n", DevicesParamsConst::SPI_SPEED_HZ, static_cast<unsigned int>(m_Params.nSpiSpeedHz));
	printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, m_Params.nGlobalBrightness);
#if !defined(OUTPUT_DMX_PIXEL_MULTI)
	printf(" %s=%d\n", DmxNodeParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);
#endif
	printf(" %s=%d\n", DevicesParamsConst::TEST_PATTERN, m_Params.nTestPattern);
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	printf(" %s=%d\n", DevicesParamsConst::GAMMA_CORRECTION, isMaskSet(pixeldmxparams::Mask::GAMMA_CORRECTION));
	printf(" %s=%1.1f [%u]\n", DevicesParamsConst::GAMMA_VALUE, static_cast<float>(m_Params.nGammaValue) / 10, m_Params.nGammaValue);
#endif
}
