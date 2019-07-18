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

#include "ltcparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

LtcParams::LtcParams(LtcParamsStore* pLtcParamsStore): m_pLTcParamsStore(pLtcParamsStore) {
	uint8_t *p = (uint8_t *) &m_tLtcParams;

	for (uint32_t i = 0; i < sizeof(struct TLtcParams); i++) {
		*p++ = 0;
	}

	m_tLtcParams.tSource = (uint8_t) LTC_READER_SOURCE_LTC;
	m_tLtcParams.nMax7219Intensity = 4;
	m_tLtcParams.nYear = 19;
	m_tLtcParams.nMonth = 1;
	m_tLtcParams.nDay = 1;
	m_tLtcParams.nFps = 25;
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

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_LTC, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_LTC;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_LTC;
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

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TCNET, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nDisabledOutputs |= LTC_PARAMS_DISABLE_TCNET;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~LTC_PARAMS_DISABLE_TCNET;
		}
	}

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

	if (Sscan::Uint8(pLine, LtcParamsConst::YEAR, &value8) == SSCAN_OK) {
		if (value8 >= 19) {
			m_tLtcParams.nYear = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_YEAR;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::MONTH, &value8) == SSCAN_OK) {
		if ((value8 >= 1) && (value8 <= 12)) {
			m_tLtcParams.nMonth = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_MONTH;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DAY, &value8) == SSCAN_OK) {
		if ((value8 >= 1) && (value8 <= 31)) {
			m_tLtcParams.nDay = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_DAY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::NTP_ENABLE, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nEnableNtp = 1;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_ENABLE_NTP;
		} else {
			m_tLtcParams.nEnableNtp = 0;
			m_tLtcParams.nSetList &= ~LTC_PARAMS_MASK_ENABLE_NTP;
		}
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::FPS, &value8) == SSCAN_OK) {
		if ((value8 >= 24) && (value8 <= 30)) {
			m_tLtcParams.nFps = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_FPS;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_FRAME, &value8) == SSCAN_OK) {
		if (value8 <= 30) {
			m_tLtcParams.nStartFrame = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_START_FRAME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_SECOND, &value8) == SSCAN_OK) {
		if (value8 <= 59) {
			m_tLtcParams.nStartSecond = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_START_SECOND;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_MINUTE, &value8) == SSCAN_OK) {
		if (value8 <= 59) {
			m_tLtcParams.nStartMinute = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_START_MINUTE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_HOUR, &value8) == SSCAN_OK) {
		if (value8 <= 23) {
			m_tLtcParams.nStartHour = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_START_HOUR;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_FRAME, &value8) == SSCAN_OK) {
		if (value8 <= 30) {
			m_tLtcParams.nStopFrame = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_STOP_FRAME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_SECOND, &value8) == SSCAN_OK) {
		if (value8 <= 59) {
			m_tLtcParams.nStopSecond = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_STOP_SECOND;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_MINUTE, &value8) == SSCAN_OK) {
		if (value8 <= 59) {
			m_tLtcParams.nStopMinute = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_STOP_MINUTE;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_HOUR, &value8) == SSCAN_OK) {
		if (value8 <= 99) {
			m_tLtcParams.nStopHour = value8;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_STOP_HOUR;
		}
		return;
	}

#if 0
	if (Sscan::Uint8(pLine, LtcParamsConst::SET_DATE, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_tLtcParams.nSetDate = 1;
			m_tLtcParams.nSetList |= LTC_PARAMS_MASK_SET_DATE;
		} else {
			m_tLtcParams.nSetDate = 0;
			m_tLtcParams.nSetList &= ~LTC_PARAMS_MASK_SET_DATE;
		}
	}
#endif

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

	if (isMaskSet(LTC_PARAMS_MASK_YEAR)) {
		printf(" %s=%d\n", LtcParamsConst::YEAR, m_tLtcParams.nYear);
	}

	if (isMaskSet(LTC_PARAMS_MASK_MONTH)) {
		printf(" %s=%d\n", LtcParamsConst::MONTH, m_tLtcParams.nMonth);
	}

	if (isMaskSet(LTC_PARAMS_MASK_DAY)) {
		printf(" %s=%d\n", LtcParamsConst::DAY, m_tLtcParams.nDay);
	}

	if (isMaskSet(LTC_PARAMS_MASK_ENABLE_NTP)) {
		printf(" NTP is enabled\n");
	}

	if (isMaskSet(LTC_PARAMS_MASK_FPS)) {
		printf(" %s=%d\n", LtcParamsConst::FPS, m_tLtcParams.nFps);
	}

	if (isMaskSet(LTC_PARAMS_MASK_START_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::START_FRAME, m_tLtcParams.nStartFrame);
	}

	if (isMaskSet(LTC_PARAMS_MASK_START_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::START_SECOND, m_tLtcParams.nStartSecond);
	}

	if (isMaskSet(LTC_PARAMS_MASK_START_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::START_MINUTE, m_tLtcParams.nStartMinute);
	}

	if (isMaskSet(LTC_PARAMS_MASK_START_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::START_HOUR, m_tLtcParams.nStartHour);
	}

	if (isMaskSet(LTC_PARAMS_MASK_STOP_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_FRAME, m_tLtcParams.nStopFrame);
	}

	if (isMaskSet(LTC_PARAMS_MASK_STOP_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_SECOND, m_tLtcParams.nStopSecond);
	}

	if (isMaskSet(LTC_PARAMS_MASK_STOP_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_MINUTE, m_tLtcParams.nStopMinute);
	}

	if (isMaskSet(LTC_PARAMS_MASK_STOP_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_HOUR, m_tLtcParams.nStopHour);
	}

#if 0
	if (isMaskSet(LTC_PARAMS_MASK_SET_DATE)) {
		printf(" %s=%d\n", LtcParamsConst::SET_DATE, m_tLtcParams.nSetDate);
	}
#endif
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
