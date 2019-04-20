/**
 * @file tlc59711dmxparams.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "devicesparamsconst.h"

#define TLC59711_TYPES_COUNT 				2
#define TLC59711_TYPES_MAX_NAME_LENGTH 		10
static const char sLedTypes[TLC59711_TYPES_COUNT][TLC59711_TYPES_MAX_NAME_LENGTH] ALIGNED = { "TLC59711\0", "TLC59711W" };

TLC59711DmxParams::TLC59711DmxParams(TLC59711DmxParamsStore *pTLC59711ParamsStore): m_pLC59711ParamsStore(pTLC59711ParamsStore) {
	m_tTLC59711Params.nSetList = 0;
	m_tTLC59711Params.LedType = TTLC59711_TYPE_RGB;
	m_tTLC59711Params.nLedCount = 4;
	m_tTLC59711Params.nDmxStartAddress = 1;
	m_tTLC59711Params.nSpiSpeedHz = 0;
}

TLC59711DmxParams::~TLC59711DmxParams(void) {
	m_tTLC59711Params.nSetList = 0;
}

bool TLC59711DmxParams::Load(void) {
	m_tTLC59711Params.nSetList = 0;

	ReadConfigFile configfile(TLC59711DmxParams::staticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pLC59711ParamsStore != 0) {
			m_pLC59711ParamsStore->Update(&m_tTLC59711Params);
		}
	} else if (m_pLC59711ParamsStore != 0) {
		m_pLC59711ParamsStore->Copy(&m_tTLC59711Params);
	} else {
		return false;
	}

	return true;
}

void TLC59711DmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pLC59711ParamsStore != 0);

	if (m_pLC59711ParamsStore == 0) {
		return;
	}

	m_tTLC59711Params.nSetList = 0;

	ReadConfigFile config(TLC59711DmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLC59711ParamsStore->Update(&m_tTLC59711Params);
}

void TLC59711DmxParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;
	uint32_t value32;
	uint8_t len;
	char buffer[12];

	len = 9;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_TYPE, buffer, &len) == SSCAN_OK) {
		buffer[len] = '\0';
		if (strcasecmp(buffer, sLedTypes[TTLC59711_TYPE_RGB]) == 0) {
			m_tTLC59711Params.LedType = TTLC59711_TYPE_RGB;
			m_tTLC59711Params.nSetList |= TLC59711DMX_PARAMS_MASK_LED_TYPE;
		} else if (strcasecmp(buffer, sLedTypes[TTLC59711_TYPE_RGBW]) == 0) {
			m_tTLC59711Params.LedType = TTLC59711_TYPE_RGBW;
			m_tTLC59711Params.nSetList |= TLC59711DMX_PARAMS_MASK_LED_TYPE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::LED_COUNT, &value8) == SSCAN_OK) {
		if ((value8 != 0) && (value8 <= 170)) {
			m_tTLC59711Params.nLedCount = value8;
			m_tTLC59711Params.nSetList |= TLC59711DMX_PARAMS_MASK_LED_COUNT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= 512)) {
			m_tTLC59711Params.nDmxStartAddress = value16;
			m_tTLC59711Params.nSetList |= TLC59711DMX_PARAMS_MASK_START_ADDRESS;
		}
		return;
	}

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, &value32) == SSCAN_OK) {
		m_tTLC59711Params.nSpiSpeedHz = value32;
		m_tTLC59711Params.nSetList |= TLC59711DMX_PARAMS_MASK_SPI_SPEED;
	}
}

void TLC59711DmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_tTLC59711Params.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DevicesParamsConst::FILE_NAME);

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_LED_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE, sLedTypes[m_tTLC59711Params.LedType], (int) m_tTLC59711Params.LedType);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_LED_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_COUNT, m_tTLC59711Params.nLedCount);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_START_ADDRESS)) {
		printf(" %s=%d\n", DevicesParamsConst::DMX_START_ADDRESS, m_tTLC59711Params.nDmxStartAddress);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_SPI_SPEED)) {
		printf(" %s=%d Hz\n", DevicesParamsConst::SPI_SPEED_HZ, m_tTLC59711Params.nSpiSpeedHz);
	}
#endif
}

const char* TLC59711DmxParams::GetLedTypeString(TTLC59711Type tTTLC59711Type) {
	assert (tTTLC59711Type < TLC59711_TYPES_COUNT);

	return sLedTypes[tTTLC59711Type];
}

bool TLC59711DmxParams::IsSetLedType(void) const {
	return isMaskSet(TLC59711DMX_PARAMS_MASK_LED_TYPE);
}

bool TLC59711DmxParams::IsSetLedCount(void) const {
	return isMaskSet(TLC59711DMX_PARAMS_MASK_LED_COUNT);
}

void TLC59711DmxParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((TLC59711DmxParams *) p)->callbackFunction(s);
}

bool TLC59711DmxParams::isMaskSet(uint32_t nMask) const {
	return (m_tTLC59711Params.nSetList & nMask) == nMask;
}
