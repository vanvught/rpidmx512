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

#include "ltcparams.h"

#include  "ltcparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#define IS_DISABLED(x)	(x) ? "Yes" : "No"

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

	if (configfile.Read(LtcParamsConst::FILE_NAME)) {
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

	if (Sscan::Char(pLine, LtcParamsConst::SOURCE, source, &len) == SSCAN_OK) {
		m_tLtcParams.tSource = GetSourceType((const char *) source);
		m_tLtcParams.nSetList |= LTC_PARAMS_MASK_SOURCE;
	}

	len = sizeof(source);

	if (Sscan::Char(pLine, LtcParamsConst::MAX7219_TYPE, source, &len) == SSCAN_OK) {
		if (strncasecmp(source, "7segment", len) == 0) {
			m_tLtcParams.tMax7219Type = LTC_PARAMS_MAX7219_TYPE_7SEGMENT;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_MAX7219_TYPE;
		} else if (strncasecmp(source, "matrix", len) == 0) {
			m_tLtcParams.tMax7219Type = LTC_PARAMS_MAX7219_TYPE_MATRIX;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_MAX7219_TYPE;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::MAX7219_INTENSITY, &value8) == SSCAN_OK) {
		if (value8 <= 0x0F) {
			m_tLtcParams.nMax7219Intensity = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_MAX7219_INTENSITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_DISPLAY, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_DISPLAY;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_DISPLAY;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_MAX7219, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_MAX7219;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_DISPLAY;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_MIDI, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_MIDI;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_DISPLAY;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_ARTNET, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_ARTNET;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_ARTNET;
		}
	}

#if 0 //TODO When MASTER is implemented
	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TCNET, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_TCNET;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_TCNET;
		}
	}
#endif

	if (Sscan::Uint8(pLine, LtcParamsConst::SHOW_SYSTIME, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nShowSysTime = 1;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_SHOW_SYSTIME;
		} else {
			m_tLtcParams.nShowSysTime = 0;
			m_tLtcParams.nSetList &= ~LTC_PARAMS_MASK_SHOW_SYSTIME;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TIMESYNC, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisableTimeSync = 1;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLE_TIMESYNC;
		} else {
			m_tLtcParams.nDisableTimeSync = 0;
			m_tLtcParams.nSetList &= ~LTC_PARAMS_MASK_DISABLE_TIMESYNC;
		}
	}
}

void LtcParams::Dump(void) {
#ifndef NDEBUG
	if (m_tLtcParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcParamsConst::FILE_NAME);

	if (isMaskSet(LTC_PARAMS_MASK_SOURCE)) {
		printf(" %s=%d [%s]\n", LtcParamsConst::SOURCE, m_tLtcParams.tSource, GetSourceType((TLtcReaderSource) m_tLtcParams.tSource));
	}

	if (isMaskSet(LTC_PARAMS_MASK_MAX7219_TYPE)) {
		printf(" %s=%d [%s]\n", LtcParamsConst::MAX7219_TYPE, m_tLtcParams.tMax7219Type, m_tLtcParams.tMax7219Type == LTC_PARAMS_MAX7219_TYPE_7SEGMENT ? "7segment" : "matrix");
	}

	if (isMaskSet(LTC_PARAMS_MASK_MAX7219_INTENSITY)) {
		printf(" %s=%d\n", LtcParamsConst::MAX7219_INTENSITY, m_tLtcParams.nMax7219Intensity);
	}

	if (isMaskSet(LTC_PARAMS_MASK_DISABLED_OUTPUTS)) {
		assert(m_tLtcParams.nDisabledOutputs != 0);

		printf(" Disabled outputs %.2x:\n", m_tLtcParams.nDisabledOutputs);

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_DISPLAY)) {
			printf("  Display\n");
		}

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MAX7219)) {
			printf("  Max7219\n");
		}

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MIDI)) {
			printf("  MIDI\n");
		}

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_ARTNET)) {
			printf("  Art-Net\n");
		}

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_TCNET)) {
			printf("  TCNet\n");
		}

		if (isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_LTC)) {
			printf("  LTC\n");
		}
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

bool LtcParams::isDisabledOutputMaskSet(uint8_t nMask) const {
	return (m_tLtcParams.nDisabledOutputs & nMask) == nMask;
}
