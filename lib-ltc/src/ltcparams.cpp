/**
 * @file ltcparams.cpp
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <time.h>
#include <cassert>

#include "ltcparams.h"
#include "ltcparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace ltc;

LtcParams::LtcParams(LtcParamsStore *pLtcParamsStore): m_pLTcParamsStore(pLtcParamsStore) {
	memset(&m_tLtcParams, 0, sizeof(struct TLtcParams));

	const time_t ltime = time(nullptr);
	const struct tm *tm = localtime(&ltime);

	m_tLtcParams.tSource = source::LTC;
	m_tLtcParams.nYear = tm->tm_year - 100;
	m_tLtcParams.nMonth = tm->tm_mon + 1;
	m_tLtcParams.nDay = tm->tm_mday;
	m_tLtcParams.nFps = 25;
	m_tLtcParams.nStopFrame = m_tLtcParams.nFps - 1;
	m_tLtcParams.nStopSecond = 59;
	m_tLtcParams.nStopMinute = 59;
	m_tLtcParams.nStopHour = 23;
	m_tLtcParams.nOscPort = 8000;
	m_tLtcParams.nSkipSeconds = 5;
}

bool LtcParams::Load() {
	m_tLtcParams.nSetList = 0;

	ReadConfigFile configfile(LtcParams::staticCallbackFunction, this);

	if (configfile.Read(LtcParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pLTcParamsStore != nullptr) {
			m_pLTcParamsStore->Update(&m_tLtcParams);
		}
	} else if (m_pLTcParamsStore != nullptr) {
		m_pLTcParamsStore->Copy(&m_tLtcParams);
	} else {
		return false;
	}

	return true;
}

void LtcParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pLTcParamsStore != nullptr);

	if (m_pLTcParamsStore == nullptr) {
		return;
	}

	m_tLtcParams.nSetList = 0;

	ReadConfigFile config(LtcParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLTcParamsStore->Update(&m_tLtcParams);
}

void LtcParams::HandleDisabledOutput(const char *pLine, const char *pKeyword, unsigned nMaskDisabledOutputs) {
	uint8_t nValue8;

	if (Sscan::Uint8(pLine, pKeyword, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcParams.nDisabledOutputs |= nMaskDisabledOutputs;
			m_tLtcParams.nSetList |= LtcParamsMask::DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= ~(nMaskDisabledOutputs);
		}
	}
}

void LtcParams::SetBool(const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask) {
	if (nValue != 0) {
		nProperty = 1;
		m_tLtcParams.nSetList |= nMask;
	} else {
		nProperty = 0;
		m_tLtcParams.nSetList &= ~nMask;
	}
}

void LtcParams::SetValue(const bool bEvaluate, const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask) {
	if (bEvaluate) {
		nProperty = nValue;
		m_tLtcParams.nSetList |= nMask;
	}
	return;
}

void LtcParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint16_t nValue16;
	char source[16];
	uint32_t nLength = sizeof(source) - 1;

	if (Sscan::Char(pLine, LtcParamsConst::SOURCE, source, nLength) == Sscan::OK) {
		source[nLength] = '\0';
		m_tLtcParams.tSource = GetSourceType(source);
		m_tLtcParams.nSetList |= LtcParamsMask::SOURCE;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::AUTO_START, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nAutoStart, LtcParamsMask::AUTO_START);
	}

	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_DISPLAY, LtcParamsMaskDisabledOutputs::DISPLAY);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MAX7219, LtcParamsMaskDisabledOutputs::MAX7219);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_LTC, LtcParamsMaskDisabledOutputs::LTC);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MIDI, LtcParamsMaskDisabledOutputs::MIDI);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_ARTNET, LtcParamsMaskDisabledOutputs::ARTNET);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_RTPMIDI, LtcParamsMaskDisabledOutputs::RTPMIDI);

	if (Sscan::Uint8(pLine, LtcParamsConst::SHOW_SYSTIME, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nShowSysTime, LtcParamsMask::SHOW_SYSTIME);
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TIMESYNC, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nDisableTimeSync, LtcParamsMask::DISABLE_TIMESYNC);
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::YEAR, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 19), nValue8, m_tLtcParams.nYear, LtcParamsMask::YEAR);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::MONTH, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 1) && (nValue8 <= 12), nValue8, m_tLtcParams.nMonth, LtcParamsMask::MONTH);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DAY, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 1) && (nValue8 <= 31), nValue8, m_tLtcParams.nDay, LtcParamsMask::DAY);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::NTP_ENABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nEnableNtp, LtcParamsMask::ENABLE_NTP);
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::FPS, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 24) && (nValue8 <= 30), nValue8, m_tLtcParams.nFps, LtcParamsMask::FPS);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_FRAME, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 30), nValue8, m_tLtcParams.nStartFrame, LtcParamsMask::START_FRAME);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_SECOND, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_tLtcParams.nStartSecond, LtcParamsMask::START_SECOND);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_MINUTE, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_tLtcParams.nStartMinute, LtcParamsMask::START_MINUTE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_HOUR, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 23), nValue8, m_tLtcParams.nStartHour, LtcParamsMask::START_HOUR);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_FRAME, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 30), nValue8, m_tLtcParams.nStopFrame, LtcParamsMask::STOP_FRAME);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_SECOND, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_tLtcParams.nStopSecond, LtcParamsMask::STOP_SECOND);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_MINUTE, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_tLtcParams.nStopMinute, LtcParamsMask::STOP_MINUTE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_HOUR, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 23), nValue8, m_tLtcParams.nStopHour, LtcParamsMask::STOP_HOUR);
		return;
	}

#if 0
	if (Sscan::Uint8(pLine, LtcParamsConst::SET_DATE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nSetDate, LtcParamsMask::SET_DATE);
	}
#endif

	if (Sscan::Uint8(pLine, LtcParamsConst::ALT_FUNCTION, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nAltFunction, LtcParamsMask::ALT_FUNCTION);
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_SECONDS, nValue8) == Sscan::OK) {
		SetValue((nValue8 > 0) && (nValue8 <= 99), nValue8, m_tLtcParams.nSkipSeconds, LtcParamsMask::SKIP_SECONDS);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_FREE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nSkipFree, LtcParamsMask::SKIP_FREE);
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::OSC_ENABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nEnableOsc, LtcParamsMask::ENABLE_OSC);
	}

	if (Sscan::Uint16(pLine, LtcParamsConst::OSC_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tLtcParams.nOscPort = nValue16;
			m_tLtcParams.nSetList |= LtcParamsMask::OSC_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::WS28XX_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcParams.nRgbLedType = static_cast<uint8_t>(TLtcParamsRgbLedType::WS28XX);
			m_tLtcParams.nSetList |= LtcParamsMask::RGBLEDTYPE;
		} else {
			m_tLtcParams.nRgbLedType &= ~static_cast<uint8_t>(TLtcParamsRgbLedType::WS28XX);

			if (m_tLtcParams.nRgbLedType == 0) {
				m_tLtcParams.nSetList &= ~LtcParamsMask::RGBLEDTYPE;
			}
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::RGBPANEL_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcParams.nRgbLedType = static_cast<uint8_t>(TLtcParamsRgbLedType::RGBPANEL);
			m_tLtcParams.nSetList |= LtcParamsMask::RGBLEDTYPE;
		} else {
			m_tLtcParams.nRgbLedType &= ~static_cast<uint8_t>(TLtcParamsRgbLedType::RGBPANEL);

			if (m_tLtcParams.nRgbLedType == 0) {
				m_tLtcParams.nSetList &= ~LtcParamsMask::RGBLEDTYPE;
			}
		}
		return;
	}
}

void LtcParams::StartTimeCodeCopyTo(TLtcTimeCode* ptStartTimeCode) {
	assert(ptStartTimeCode != nullptr);

	if ((isMaskSet(LtcParamsMask::START_FRAME)) || (isMaskSet(LtcParamsMask::START_SECOND)) || (isMaskSet(LtcParamsMask::START_MINUTE)) || (isMaskSet(LtcParamsMask::START_HOUR)) ) {
		memset(ptStartTimeCode, 0, sizeof(struct TLtcTimeCode));

		if (isMaskSet(LtcParamsMask::START_FRAME)) {
			ptStartTimeCode->nFrames = m_tLtcParams.nStartFrame;
		}

		if (isMaskSet(LtcParamsMask::START_SECOND)) {
			ptStartTimeCode->nSeconds = m_tLtcParams.nStartSecond;
		}

		if (isMaskSet(LtcParamsMask::START_MINUTE)) {
			ptStartTimeCode->nMinutes = m_tLtcParams.nStartMinute;
		}

		if (isMaskSet(LtcParamsMask::START_HOUR)) {
			ptStartTimeCode->nHours = m_tLtcParams.nStartHour;
		}
	} else {
		ptStartTimeCode->nFrames = m_tLtcParams.nStartFrame;
		ptStartTimeCode->nSeconds = m_tLtcParams.nStartSecond;
		ptStartTimeCode->nMinutes = m_tLtcParams.nStartMinute;
		ptStartTimeCode->nHours = m_tLtcParams.nStartHour;
	}

	ptStartTimeCode->nType = Ltc::GetType(m_tLtcParams.nFps);
}

void LtcParams::StopTimeCodeCopyTo(TLtcTimeCode* ptStopTimeCode) {
	assert(ptStopTimeCode != nullptr);

	if ((isMaskSet(LtcParamsMask::STOP_FRAME)) || (isMaskSet(LtcParamsMask::STOP_SECOND)) || (isMaskSet(LtcParamsMask::STOP_MINUTE)) || (isMaskSet(LtcParamsMask::STOP_HOUR)) ) {
		memset(ptStopTimeCode, 0, sizeof(struct TLtcTimeCode));

		if (isMaskSet(LtcParamsMask::STOP_FRAME)) {
			ptStopTimeCode->nFrames = m_tLtcParams.nStopFrame;
		}

		if (isMaskSet(LtcParamsMask::STOP_SECOND)) {
			ptStopTimeCode->nSeconds = m_tLtcParams.nStopSecond;
		}

		if (isMaskSet(LtcParamsMask::STOP_MINUTE)) {
			ptStopTimeCode->nMinutes = m_tLtcParams.nStopMinute;
		}

		if (isMaskSet(LtcParamsMask::STOP_HOUR)) {
			ptStopTimeCode->nHours = m_tLtcParams.nStopHour;
		}
	} else {
		ptStopTimeCode->nFrames = m_tLtcParams.nStopFrame;
		ptStopTimeCode->nSeconds = m_tLtcParams.nStopSecond;
		ptStopTimeCode->nMinutes = m_tLtcParams.nStopMinute;
		ptStopTimeCode->nHours = m_tLtcParams.nStopHour;
	}

	ptStopTimeCode->nType = Ltc::GetType(m_tLtcParams.nFps);
}

void LtcParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcParams*>(p))->callbackFunction(s);
}
