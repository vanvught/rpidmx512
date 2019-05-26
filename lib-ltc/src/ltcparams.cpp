/**
 * @file ltcparams.h
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "ltcparams.h"
#include "ltcreader.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#define SET_SOURCE				(1 << 0)
#define SET_MAX7219_TYPE		(1 << 1)
#define SET_MAX7219_INTENSITY	(1 << 2)

static const char PARAMS_FILE_NAME[] ALIGNED = "ltc.txt";
static const char PARAMS_SOURCE[] ALIGNED = "source";
static const char PARAMS_MAX7219_TYPE[] ALIGNED = "max7219_type";
static const char PARAMS_MAX7219_INTENSITY[] ALIGNED = "max7219_intensity";

LtcParams::LtcParams(LtcParamsStore* pLtcParamsStore): m_pLTcParamsStore(pLtcParamsStore) {
	uint8_t *p = (uint8_t *) &m_tLtcParams;

	for (uint32_t i = 0; i < sizeof(struct TLtcParams); i++) {
		*p++ = 0;
	}

	m_tLtcParams.tSource = (uint8_t) LTC_READER_SOURCE_LTC;
	m_tLtcParams.nMax7219Intensity = 4;
}

LtcParams::~LtcParams(void) {
	m_tLtcParams.nSetList = 0;
}

bool LtcParams::Load(void) {
	m_tLtcParams.nSetList = 0;

	ReadConfigFile configfile(LtcParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pLTcParamsStore != 0) {
			m_pLTcParamsStore->Update(&m_tLtcParams);
		}
	} else if (m_pLTcParamsStore != 0) {
		m_pLTcParamsStore->Copy(&m_tLtcParams);
	} else {
		return false;
	}

	return true;
}

void LtcParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pLTcParamsStore != 0);

	if (m_pLTcParamsStore == 0) {
		return;
	}

	m_tLtcParams.nSetList = 0;

	ReadConfigFile config(LtcParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLTcParamsStore->Update(&m_tLtcParams);
}

void LtcParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	char source[16];
	uint8_t len = sizeof(source);

	if (Sscan::Char(pLine, PARAMS_SOURCE, source, &len) == SSCAN_OK) {
		if (strncasecmp(source, "artnet", len) == 0) {
			m_tLtcParams.tSource = (uint8_t) LTC_READER_SOURCE_ARTNET;
			m_tLtcParams.nSetList |= SET_SOURCE;
		} else if (strncasecmp(source, "midi", len) == 0) {
			m_tLtcParams.tSource = (uint8_t) LTC_READER_SOURCE_MIDI;
			m_tLtcParams.nSetList |= SET_SOURCE;
		} else if (strncasecmp(source, "ltc", len) == 0) {
			m_tLtcParams.tSource = (uint8_t) LTC_READER_SOURCE_LTC;
			m_tLtcParams.nSetList |= SET_SOURCE;
		}
	}

	len = sizeof(source);

	if (Sscan::Char(pLine, PARAMS_MAX7219_TYPE, source, &len) == SSCAN_OK) {
		if (strncasecmp(source, "7segment", len) == 0) {
			m_tLtcParams.tMax7219Type = 1;
			m_tLtcParams.nSetList |= SET_MAX7219_TYPE;
		} else if (strncasecmp(source, "matrix", len) == 0) {
			m_tLtcParams.tMax7219Type = 0;
			m_tLtcParams.nSetList |= SET_MAX7219_TYPE;
		}
	}

	if (Sscan::Uint8(pLine, PARAMS_MAX7219_INTENSITY, &value8) == SSCAN_OK) {
		if (value8 <= 0x0F) {
			m_tLtcParams.nMax7219Intensity = value8;
			m_tLtcParams.nSetList |= SET_MAX7219_INTENSITY;
		}
		return;
	}
}

void LtcParams::Dump(void) {
#ifndef NDEBUG
	if (m_tLtcParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_SOURCE)) {
		printf(" %s=%d [%s]\n", PARAMS_SOURCE, m_tLtcParams.tSource, m_tLtcParams.tSource == (uint8_t) LTC_READER_SOURCE_ARTNET ? "artnet" : (m_tLtcParams.tSource == (uint8_t) LTC_READER_SOURCE_MIDI ? "midi" : "ltc"));
	}

	if (isMaskSet(SET_MAX7219_TYPE)) {
		printf(" %s=%d\n", PARAMS_MAX7219_TYPE, m_tLtcParams.tMax7219Type);
	}

	if (isMaskSet(SET_MAX7219_INTENSITY)) {
		printf(" %s=%d\n", PARAMS_MAX7219_INTENSITY, m_tLtcParams.nMax7219Intensity);
	}

#endif
}

void LtcParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((LtcParams *) p)->callbackFunction(s);
}

bool LtcParams::isMaskSet(uint32_t nMask) const {
	return (m_tLtcParams.nSetList & nMask) == nMask;
}
