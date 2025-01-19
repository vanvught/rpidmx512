/**
 * @file tlc59711dmxparams.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightset.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "devicesparamsconst.h"
#include "lightsetparamsconst.h"

#include "debug.h"

static constexpr auto TLC59711_TYPES_MAX_NAME_LENGTH = 10U;
static constexpr char sLedTypes[static_cast<uint32_t>(tlc59711::Type::UNDEFINED)][TLC59711_TYPES_MAX_NAME_LENGTH] = { "TLC59711\0", "TLC59711W" };

TLC59711DmxParams::TLC59711DmxParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;
	m_Params.nType = static_cast<uint8_t>(tlc59711::Type::RGB);
	m_Params.nCount = 4;
	m_Params.nDmxStartAddress = lightset::dmx::START_ADDRESS_DEFAULT;
	m_Params.nSpiSpeedHz = 0;

	DEBUG_EXIT
}

void TLC59711DmxParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(TLC59711DmxParams::StaticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		TLC59711DmxParamsStore::Update(&m_Params);
	} else
#endif
		TLC59711DmxParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void TLC59711DmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(TLC59711DmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	TLC59711DmxParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void TLC59711DmxParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	char buffer[12];

	uint32_t nLength = 9;

	if (Sscan::Char(pLine, DevicesParamsConst::TYPE, buffer, nLength) == Sscan::OK) {
		buffer[nLength] = '\0';
		if (strcasecmp(buffer, sLedTypes[static_cast<uint8_t>(tlc59711::Type::RGB)]) == 0) {
			m_Params.nType = static_cast<uint8_t>(tlc59711::Type::RGB);
			m_Params.nSetList |= tlc59711dmxparams::Mask::TYPE;
		} else if (strcasecmp(buffer, sLedTypes[static_cast<uint8_t>(tlc59711::Type::RGBW)]) == 0) {
			m_Params.nType = static_cast<uint8_t>(tlc59711::Type::RGBW);
			m_Params.nSetList |= tlc59711dmxparams::Mask::TYPE;
		} else {
			m_Params.nSetList &= ~tlc59711dmxparams::Mask::TYPE;
		}
		return;
	}

	uint8_t value8;

	if (Sscan::Uint8(pLine, DevicesParamsConst::COUNT, value8) == Sscan::OK) {
		if ((value8 != 0) && (value8 <= 170)) {
			m_Params.nCount = value8;
			m_Params.nSetList |= tlc59711dmxparams::Mask::COUNT;
		} else {
			m_Params.nCount = 4;
			m_Params.nSetList &= ~tlc59711dmxparams::Mask::COUNT;
		}
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, LightSetParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && nValue16 <= (lightset::dmx::UNIVERSE_SIZE) && (nValue16 != lightset::dmx::START_ADDRESS_DEFAULT)) {
			m_Params.nDmxStartAddress = nValue16;
			m_Params.nSetList |= tlc59711dmxparams::Mask::DMX_START_ADDRESS;
		} else {
			m_Params.nDmxStartAddress = lightset::dmx::START_ADDRESS_DEFAULT;
			m_Params.nSetList &= ~tlc59711dmxparams::Mask::DMX_START_ADDRESS;
		}
		return;
	}

	uint32_t value32;

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, value32) == Sscan::OK) {
		m_Params.nSpiSpeedHz = value32;

		if (value32 != 0) {
			m_Params.nSetList |= tlc59711dmxparams::Mask::SPI_SPEED;
		} else {
			m_Params.nSetList &= ~tlc59711dmxparams::Mask::SPI_SPEED;
		}
		return;
	}
}

void TLC59711DmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<TLC59711DmxParams*>(p))->callbackFunction(s);
}

void TLC59711DmxParams::Builder(const struct tlc59711dmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct tlc59711dmxparams::Params));
	} else {
		TLC59711DmxParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::TYPE, GetType(static_cast<tlc59711::Type>(m_Params.nType)), isMaskSet(tlc59711dmxparams::Mask::TYPE));
	builder.Add(DevicesParamsConst::COUNT, m_Params.nCount, isMaskSet(tlc59711dmxparams::Mask::COUNT));
	builder.Add(LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress, isMaskSet(tlc59711dmxparams::Mask::DMX_START_ADDRESS));
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_Params.nSpiSpeedHz, isMaskSet(tlc59711dmxparams::Mask::SPI_SPEED));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void TLC59711DmxParams::Set(TLC59711Dmx *pTLC59711Dmx) {
	assert(pTLC59711Dmx != nullptr);

	if (isMaskSet(tlc59711dmxparams::Mask::TYPE)) {
		pTLC59711Dmx->SetType(static_cast<tlc59711::Type>(m_Params.nType));
	}

	if (isMaskSet(tlc59711dmxparams::Mask::COUNT)) {
		pTLC59711Dmx->SetCount(m_Params.nCount);
	}

	if (isMaskSet(tlc59711dmxparams::Mask::DMX_START_ADDRESS)) {
		pTLC59711Dmx->SetDmxStartAddress(m_Params.nDmxStartAddress);
	}

	if (isMaskSet(tlc59711dmxparams::Mask::SPI_SPEED)) {
		pTLC59711Dmx->SetSpiSpeedHz(m_Params.nSpiSpeedHz);
	}
}

void TLC59711Dmx::Print() {
	puts("PWM parameters");
	printf(" Type  : %s [%d]\n", TLC59711DmxParams::GetType(m_type), static_cast<uint32_t>(m_type)); //TODO Move TLC59711DmxParams to TLC59711
	printf(" Count : %d %s\n", m_nCount, m_type == tlc59711::Type::RGB ? "RGB" : "RGBW");
	printf(" Clock : %d Hz %s {Default: %d Hz, Maximum %d Hz}\n", m_nSpiSpeedHz, (m_nSpiSpeedHz == 0 ? "Default" : ""), TLC59711SpiSpeed::DEFAULT, TLC59711SpiSpeed::MAX);
	printf(" DMX   : StartAddress=%d, FootPrint=%d\n", m_nDmxStartAddress, m_nDmxFootprint);
}

void TLC59711DmxParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DevicesParamsConst::FILE_NAME);
	printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, sLedTypes[m_Params.nType], static_cast<int>(m_Params.nType));
	printf(" %s=%d\n", DevicesParamsConst::COUNT, m_Params.nCount);
	printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);
	printf(" %s=%d Hz\n", DevicesParamsConst::SPI_SPEED_HZ, m_Params.nSpiSpeedHz);
}

/*
 * Static
 */

const char *TLC59711DmxParams::GetType(tlc59711::Type type) {
	assert (type < tlc59711::Type::UNDEFINED);

	return sLedTypes[static_cast<uint32_t>(type)];
}


tlc59711::Type TLC59711DmxParams::GetType(const char *pValue) {
	assert(pValue != nullptr);

	if (strcasecmp(pValue, sLedTypes[static_cast<uint32_t>(tlc59711::Type::RGB)]) == 0) {
		return tlc59711::Type::RGB;
	} else if (strcasecmp(pValue, sLedTypes[static_cast<uint32_t>(tlc59711::Type::RGBW)]) == 0) {
		return tlc59711::Type::RGBW;
	}

	return tlc59711::Type::UNDEFINED;
}
