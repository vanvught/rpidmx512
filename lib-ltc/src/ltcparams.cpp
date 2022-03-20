/**
 * @file ltcparams.cpp
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <time.h>
#include <cassert>

#include "ltcparams.h"
#include "ltcparamsconst.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace ltc;

static constexpr auto VOLUME_0DBV = 28;

LtcParams::LtcParams(LtcParamsStore *pLtcParamsStore): m_pLTcParamsStore(pLtcParamsStore) {
	memset(&m_tLtcParams, 0, sizeof(struct TLtcParams));

	const auto ltime = time(nullptr);
	const auto *tm = localtime(&ltime);

	m_tLtcParams.tSource = source::LTC;
	m_tLtcParams.nVolume = VOLUME_0DBV;
	m_tLtcParams.nYear = static_cast<uint8_t>(tm->tm_year - 100);
	m_tLtcParams.nMonth = static_cast<uint8_t>(tm->tm_mon + 1);
	m_tLtcParams.nDay = static_cast<uint8_t>(tm->tm_mday);
	m_tLtcParams.nFps = 25;
	m_tLtcParams.nStopFrame = static_cast<uint8_t>(m_tLtcParams.nFps - 1);
	m_tLtcParams.nStopSecond = 59;
	m_tLtcParams.nStopMinute = 59;
	m_tLtcParams.nStopHour = 23;
	m_tLtcParams.nOscPort = 8000;
	m_tLtcParams.nSkipSeconds = 5;
	m_tLtcParams.nTimeCodeIp = Network::Get()->GetBroadcastIp();
}

bool LtcParams::Load() {
	m_tLtcParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(LtcParams::staticCallbackFunction, this);

	if (configfile.Read(LtcParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pLTcParamsStore != nullptr) {
			m_pLTcParamsStore->Update(&m_tLtcParams);
		}
	} else
#endif
	if (m_pLTcParamsStore != nullptr) {
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

void LtcParams::HandleDisabledOutput(const char *pLine, const char *pKeyword, uint8_t nMaskDisabledOutputs) {
	uint8_t nValue8;

	if (Sscan::Uint8(pLine, pKeyword, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcParams.nDisabledOutputs |= nMaskDisabledOutputs;
			m_tLtcParams.nSetList |= LtcParamsMask::DISABLED_OUTPUTS;
		} else {
			m_tLtcParams.nDisabledOutputs &= static_cast<uint8_t>(~nMaskDisabledOutputs);
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
	} else {
		m_tLtcParams.nSetList &= ~nMask;
	}
	return;
}

void LtcParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	char source[16];
	uint32_t nLength = sizeof(source) - 1;

	if (Sscan::Char(pLine, LtcParamsConst::SOURCE, source, nLength) == Sscan::OK) {
		source[nLength] = '\0';
		m_tLtcParams.tSource = GetSourceType(source);
		if (m_tLtcParams.tSource != source::LTC) {
			m_tLtcParams.nSetList |= LtcParamsMask::SOURCE;
		} else {
			m_tLtcParams.nSetList &= ~LtcParamsMask::SOURCE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, LtcParamsConst::VOLUME, nValue8) == Sscan::OK) {
		if ((nValue8 > 1) && (nValue8 < 32)) {
			m_tLtcParams.nVolume = nValue8;
			m_tLtcParams.nSetList |= LtcParamsMask::VOLUME;
		} else {
			m_tLtcParams.nVolume = VOLUME_0DBV;
			m_tLtcParams.nSetList &= ~LtcParamsMask::VOLUME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::AUTO_START, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nAutoStart, LtcParamsMask::AUTO_START);
		return;
	}

	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_DISPLAY, LtcParamsMaskDisabledOutputs::DISPLAY);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MAX7219, LtcParamsMaskDisabledOutputs::MAX7219);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_LTC, LtcParamsMaskDisabledOutputs::LTC);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MIDI, LtcParamsMaskDisabledOutputs::MIDI);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_ARTNET, LtcParamsMaskDisabledOutputs::ARTNET);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_RTPMIDI, LtcParamsMaskDisabledOutputs::RTPMIDI);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_ETC, LtcParamsMaskDisabledOutputs::ETC);

	if (Sscan::Uint8(pLine, LtcParamsConst::SHOW_SYSTIME, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nShowSysTime, LtcParamsMask::SHOW_SYSTIME);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TIMESYNC, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nDisableTimeSync, LtcParamsMask::DISABLE_TIMESYNC);
		return;
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
		return;
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
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_SECONDS, nValue8) == Sscan::OK) {
		SetValue((nValue8 > 0) && (nValue8 <= 99), nValue8, m_tLtcParams.nSkipSeconds, LtcParamsMask::SKIP_SECONDS);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_FREE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nSkipFree, LtcParamsMask::SKIP_FREE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::OSC_ENABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tLtcParams.nEnableOsc, LtcParamsMask::ENABLE_OSC);
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, LtcParamsConst::OSC_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tLtcParams.nOscPort = nValue16;
			m_tLtcParams.nSetList |= LtcParamsMask::OSC_PORT;
		} else {
			m_tLtcParams.nOscPort = 8000;
			m_tLtcParams.nSetList &= ~LtcParamsMask::OSC_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::WS28XX_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tLtcParams.nRgbLedType = static_cast<uint8_t>(TLtcParamsRgbLedType::WS28XX);
			m_tLtcParams.nSetList |= LtcParamsMask::RGBLEDTYPE;
		} else {
			m_tLtcParams.nRgbLedType &= static_cast<uint8_t>(~TLtcParamsRgbLedType::WS28XX);

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
			m_tLtcParams.nRgbLedType &= static_cast<uint8_t>(~TLtcParamsRgbLedType::RGBPANEL);

			if (m_tLtcParams.nRgbLedType == 0) {
				m_tLtcParams.nSetList &= ~LtcParamsMask::RGBLEDTYPE;
			}
		}
		return;
	}

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, LtcParamsConst::TIMECODE_IP, nValue32) == Sscan::OK) {
		if (Network::Get()->IsValidIp(nValue32)) {
			m_tLtcParams.nSetList |= LtcParamsMask::TIMECODE_IP;
			m_tLtcParams.nTimeCodeIp = nValue32;
		} else {
			m_tLtcParams.nSetList &= ~LtcParamsMask::TIMECODE_IP;
			m_tLtcParams.nTimeCodeIp = Network::Get()->GetBroadcastIp();
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

void LtcParams::Builder(const struct TLtcParams *ptLtcParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptLtcParams != nullptr) {
		memcpy(&m_tLtcParams, ptLtcParams, sizeof(struct TLtcParams));
	} else {
		m_pLTcParamsStore->Copy(&m_tLtcParams);
	}

	PropertiesBuilder builder(LtcParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(LtcParamsConst::SOURCE, GetSourceType(static_cast<source>(m_tLtcParams.tSource)), isMaskSet(LtcParamsMask::SOURCE));

	builder.AddComment("Disable outputs");
	builder.Add(LtcParamsConst::DISABLE_DISPLAY, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY));
	builder.Add(LtcParamsConst::DISABLE_MAX7219, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219));
	builder.Add(LtcParamsConst::DISABLE_LTC, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC));
	builder.Add(LtcParamsConst::DISABLE_MIDI, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI));
	builder.Add(LtcParamsConst::DISABLE_ARTNET, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET));
	builder.Add(LtcParamsConst::DISABLE_RTPMIDI, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI));
	builder.Add(LtcParamsConst::DISABLE_ETC, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ETC));

	builder.AddComment("System clock / RTC");
	builder.Add(LtcParamsConst::SHOW_SYSTIME, isMaskSet(LtcParamsMask::SHOW_SYSTIME));
	builder.Add(LtcParamsConst::DISABLE_TIMESYNC, isMaskSet(LtcParamsMask::DISABLE_TIMESYNC));

	builder.AddComment("source=systime");
	builder.Add(LtcParamsConst::AUTO_START, isMaskSet(LtcParamsMask::AUTO_START));

	builder.AddComment("source=internal");
	builder.Add(LtcParamsConst::FPS, m_tLtcParams.nFps, isMaskSet(LtcParamsMask::FPS));
	builder.Add(LtcParamsConst::START_HOUR, m_tLtcParams.nStartHour, isMaskSet(LtcParamsMask::START_HOUR));
	builder.Add(LtcParamsConst::START_MINUTE, m_tLtcParams.nStartMinute, isMaskSet(LtcParamsMask::START_MINUTE));
	builder.Add(LtcParamsConst::START_SECOND, m_tLtcParams.nStartSecond, isMaskSet(LtcParamsMask::START_SECOND));
	builder.Add(LtcParamsConst::START_FRAME, m_tLtcParams.nStartFrame, isMaskSet(LtcParamsMask::START_FRAME));
	builder.Add(LtcParamsConst::STOP_HOUR, m_tLtcParams.nStopHour, isMaskSet(LtcParamsMask::STOP_HOUR));
	builder.Add(LtcParamsConst::STOP_MINUTE,  m_tLtcParams.nStopMinute, isMaskSet(LtcParamsMask::STOP_MINUTE));
	builder.Add(LtcParamsConst::STOP_SECOND, m_tLtcParams.nStopSecond, isMaskSet(LtcParamsMask::STOP_SECOND));
	builder.Add(LtcParamsConst::STOP_FRAME, m_tLtcParams.nStopFrame, isMaskSet(LtcParamsMask::STOP_FRAME));
	builder.Add(LtcParamsConst::SKIP_FREE, m_tLtcParams.nSkipFree, isMaskSet(LtcParamsMask::SKIP_FREE));
	builder.AddComment("MCP buttons");
	builder.Add(LtcParamsConst::ALT_FUNCTION, isMaskSet(LtcParamsMask::ALT_FUNCTION));
	builder.Add(LtcParamsConst::SKIP_SECONDS, m_tLtcParams.nSkipSeconds, isMaskSet(LtcParamsMask::SKIP_SECONDS));

	builder.AddComment("Art-Net output");
	builder.AddIpAddress(LtcParamsConst::TIMECODE_IP, m_tLtcParams.nTimeCodeIp, isMaskSet(LtcParamsMask::TIMECODE_IP));

	builder.AddComment("LTC output");
	builder.Add(LtcParamsConst::VOLUME, m_tLtcParams.nVolume, isMaskSet(LtcParamsMask::VOLUME));

	builder.AddComment("NTP Server");
	builder.Add(LtcParamsConst::NTP_ENABLE, isMaskSet(LtcParamsMask::ENABLE_NTP));
	builder.Add(LtcParamsConst::YEAR, m_tLtcParams.nYear, isMaskSet(LtcParamsMask::YEAR));
	builder.Add(LtcParamsConst::MONTH, m_tLtcParams.nMonth, isMaskSet(LtcParamsMask::MONTH));
	builder.Add(LtcParamsConst::DAY, m_tLtcParams.nDay, isMaskSet(LtcParamsMask::DAY));

	builder.AddComment("OSC Server");
	builder.Add(LtcParamsConst::OSC_ENABLE, isMaskSet(LtcParamsMask::ENABLE_OSC));
	builder.Add(LtcParamsConst::OSC_PORT, m_tLtcParams.nOscPort, isMaskSet(LtcParamsMask::OSC_PORT));

	builder.AddComment("WS28xx display");
	builder.Add(LtcParamsConst::WS28XX_ENABLE, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::WS28XX, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::WS28XX);

	builder.AddComment("RGB panel");
	builder.Add(LtcParamsConst::RGBPANEL_ENABLE, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::RGBPANEL, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::RGBPANEL);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void LtcParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pLTcParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void LtcParams::CopyDisabledOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs) {
	pLtcDisabledOutputs->bOled = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY);
	pLtcDisabledOutputs->bMax7219 = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219);
	pLtcDisabledOutputs->bMidi = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI);
	pLtcDisabledOutputs->bArtNet = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET);
	pLtcDisabledOutputs->bLtc = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC);
	pLtcDisabledOutputs->bEtc = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ETC);
	pLtcDisabledOutputs->bNtp = (m_tLtcParams.nEnableNtp == 0);
	pLtcDisabledOutputs->bRtpMidi = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI);
	pLtcDisabledOutputs->bWS28xx = (m_tLtcParams.nRgbLedType != TLtcParamsRgbLedType::WS28XX);
	pLtcDisabledOutputs->bRgbPanel = (m_tLtcParams.nRgbLedType != TLtcParamsRgbLedType::RGBPANEL);

	assert (pLtcDisabledOutputs->bWS28xx || pLtcDisabledOutputs->bRgbPanel);
}

#include <cstdio>

void LtcParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcParamsConst::FILE_NAME);

	if (isMaskSet(LtcParamsMask::SOURCE)) {
		printf(" %s=%d [%s]\n", LtcParamsConst::SOURCE, m_tLtcParams.tSource, GetSourceType(static_cast<ltc::source>(m_tLtcParams.tSource)));
	}

	if (isMaskSet(LtcParamsMask::VOLUME)) {
		printf(" %s=%d\n", LtcParamsConst::VOLUME, m_tLtcParams.nVolume);
	}

	if (isMaskSet(LtcParamsMask::AUTO_START)) {
		printf(" %s=%d\n", LtcParamsConst::AUTO_START, m_tLtcParams.nAutoStart);
	}

	if (isMaskSet(LtcParamsMask::DISABLED_OUTPUTS)) {
		printf(" Disabled outputs %.2x:\n", m_tLtcParams.nDisabledOutputs);

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY)) {
			printf("  Display\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219)) {
			printf("  Max7219\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI)) {
			printf("  MIDI\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI)) {
			printf("  RtpMIDI\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET)) {
			printf("  Art-Net\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::TCNET)) {
			printf("  TCNet\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ETC)) {
			printf("  ETC\n");
		}

		if (isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC)) {
			printf("  LTC\n");
		}
	}

	if (isMaskSet(LtcParamsMask::YEAR)) {
		printf(" %s=%d\n", LtcParamsConst::YEAR, m_tLtcParams.nYear);
	}

	if (isMaskSet(LtcParamsMask::MONTH)) {
		printf(" %s=%d\n", LtcParamsConst::MONTH, m_tLtcParams.nMonth);
	}

	if (isMaskSet(LtcParamsMask::DAY)) {
		printf(" %s=%d\n", LtcParamsConst::DAY, m_tLtcParams.nDay);
	}

	if (isMaskSet(LtcParamsMask::ENABLE_NTP)) {
		printf(" NTP is enabled\n");
	}

	if (isMaskSet(LtcParamsMask::FPS)) {
		printf(" %s=%d\n", LtcParamsConst::FPS, m_tLtcParams.nFps);
	}

	if (isMaskSet(LtcParamsMask::START_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::START_FRAME, m_tLtcParams.nStartFrame);
	}

	if (isMaskSet(LtcParamsMask::START_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::START_SECOND, m_tLtcParams.nStartSecond);
	}

	if (isMaskSet(LtcParamsMask::START_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::START_MINUTE, m_tLtcParams.nStartMinute);
	}

	if (isMaskSet(LtcParamsMask::START_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::START_HOUR, m_tLtcParams.nStartHour);
	}

	if (isMaskSet(LtcParamsMask::STOP_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_FRAME, m_tLtcParams.nStopFrame);
	}

	if (isMaskSet(LtcParamsMask::STOP_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_SECOND, m_tLtcParams.nStopSecond);
	}

	if (isMaskSet(LtcParamsMask::STOP_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_MINUTE, m_tLtcParams.nStopMinute);
	}

	if (isMaskSet(LtcParamsMask::STOP_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_HOUR, m_tLtcParams.nStopHour);
	}

	if (isMaskSet(LtcParamsMask::ALT_FUNCTION)) {
		printf(" %s=%d\n", LtcParamsConst::ALT_FUNCTION, m_tLtcParams.nAltFunction);
	}

	if (isMaskSet(LtcParamsMask::SKIP_SECONDS)) {
		printf(" %s=%d\n", LtcParamsConst::SKIP_SECONDS, m_tLtcParams.nSkipSeconds);
	}

#if 0
	if (isMaskSet(LtcParamsMask::SET_DATE)) {
		printf(" %s=%d\n", LtcParamsConst::SET_DATE, m_tLtcParams.nSetDate);
	}
#endif

	if (isMaskSet(LtcParamsMask::ENABLE_OSC)) {
		printf(" OSC is enabled\n");

		if (isMaskSet(LtcParamsMask::OSC_PORT)) {
			printf(" %s=%d\n", LtcParamsConst::OSC_PORT, m_tLtcParams.nOscPort);
		}
	}

	if (isMaskSet(LtcParamsMask::RGBLEDTYPE)) {
		if (m_tLtcParams.nRgbLedType == static_cast<uint8_t>(TLtcParamsRgbLedType::WS28XX)) {
			printf(" WS28xx is enabled\n");
		} else if (m_tLtcParams.nRgbLedType == static_cast<uint8_t>(TLtcParamsRgbLedType::RGBPANEL)) {
			printf(" RGB panel is enabled\n");
		} else {
			printf("nRgbLedType=%u\n", m_tLtcParams.nRgbLedType);
		}
	}
#endif
}
