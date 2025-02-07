/**
 * @file ltcparams.cpp
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_LTCPARAMS)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <time.h>
#include <cassert>

#include "ltcparams.h"
#include "ltcparamsconst.h"

#include "configstore.h"

#include "hardware.h"
#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace ltc;

static constexpr auto VOLUME_0DBV = 28;

LtcParams::LtcParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct ltcparams::Params));

	const auto ltime = time(nullptr);
	const auto *tm = localtime(&ltime);

	m_Params.nSource = static_cast<uint8_t>(ltc::Source::LTC);
	m_Params.nVolume = VOLUME_0DBV;
	m_Params.nYear = static_cast<uint8_t>(tm->tm_year - 100);
	m_Params.nMonth = static_cast<uint8_t>(tm->tm_mon + 1);
	m_Params.nDay = static_cast<uint8_t>(tm->tm_mday);
	m_Params.nFps = 25;
	m_Params.nStopFrame = static_cast<uint8_t>(m_Params.nFps - 1);
	m_Params.nStopSecond = 59;
	m_Params.nStopMinute = 59;
	m_Params.nStopHour = 23;
	m_Params.nOscPort = 8000;
	m_Params.nSkipSeconds = 5;
	m_Params.nTimeCodeIp = Network::Get()->GetBroadcastIp();

	ltc::g_Type = get_type(m_Params.nFps);

	DEBUG_EXIT
}

void LtcParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(LtcParams::StaticCallbackFunction, this);

	if (configfile.Read(LtcParamsConst::FILE_NAME)) {
		ltcparams::store::update(&m_Params);
	} else
#endif
		ltcparams::store::copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(LtcParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	ltcparams::store::update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void LtcParams::HandleDisabledOutput(const char *pLine, const char *pKeyword, const ltc::Destination::Output nOutput) {
	uint8_t nValue8;

	if (Sscan::Uint8(pLine, pKeyword, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nDisabledOutputs |= static_cast<uint8_t>(nOutput);
			m_Params.nSetList |= ltcparams::Mask::DISABLED_OUTPUTS;
		} else {
			m_Params.nDisabledOutputs &= ~static_cast<uint8_t>(nOutput);
		}
	}
}

void LtcParams::SetBool(const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask) {
	if (nValue != 0) {
		nProperty = 1;
		m_Params.nSetList |= nMask;
	} else {
		nProperty = 0;
		m_Params.nSetList &= ~nMask;
	}
}

void LtcParams::SetValue(const bool bEvaluate, const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask) {
	if (bEvaluate) {
		nProperty = nValue;
		m_Params.nSetList |= nMask;
	} else {
		m_Params.nSetList &= ~nMask;
	}
	return;
}

void LtcParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char source[16];
	uint32_t nLength = sizeof(source) - 1;

	if (Sscan::Char(pLine, LtcParamsConst::SOURCE, source, nLength) == Sscan::OK) {
		source[nLength] = '\0';
		m_Params.nSource = static_cast<uint8_t>(GetSourceType(source));
		if (m_Params.nSource != static_cast<uint8_t>(ltc::Source::LTC)) {
			m_Params.nSetList |= ltcparams::Mask::SOURCE;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::SOURCE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, LtcParamsConst::VOLUME, nValue8) == Sscan::OK) {
		if ((nValue8 > 1) && (nValue8 < 32)) {
			m_Params.nVolume = nValue8;
			m_Params.nSetList |= ltcparams::Mask::VOLUME;
		} else {
			m_Params.nVolume = VOLUME_0DBV;
			m_Params.nSetList &= ~ltcparams::Mask::VOLUME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SHOW_SYSTIME, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::SHOW_SYSTIME;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::SHOW_SYSTIME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::AUTO_START, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::AUTO_START;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::AUTO_START;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::GPS_START, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::GPS_START;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::GPS_START;
		}
		return;
	}

	if (Sscan::UtcOffset(pLine, LtcParamsConst::UTC_OFFSET, m_Params.nUtcOffsetHours, m_Params.nUtcOffsetMinutes) == Sscan::OK) {
		int32_t nUtcOffset;
		if (hal::utc_validate(m_Params.nUtcOffsetHours, m_Params.nUtcOffsetMinutes, nUtcOffset)) {
			DEBUG_PUTS("UtcOffset OK.");
		} else {
			m_Params.nUtcOffsetHours = 0;
			m_Params.nUtcOffsetMinutes = 0;
			DEBUG_PUTS("UtcOffset failed.");
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DISABLE_TIMESYNC, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::DISABLE_TIMESYNC;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::DISABLE_TIMESYNC;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::NTP_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::NTP_ENABLE;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::NTP_ENABLE;
		}
		return;
	}

	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_DISPLAY_OLED, Destination::Output::DISPLAY_OLED);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MAX7219, Destination::Output::MAX7219);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_LTC, Destination::Output::LTC);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_MIDI, Destination::Output::MIDI);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_ARTNET, Destination::Output::ARTNET);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_RTPMIDI, Destination::Output::RTPMIDI);
	HandleDisabledOutput(pLine, LtcParamsConst::DISABLE_ETC, Destination::Output::ETC);

	if (Sscan::Uint8(pLine, LtcParamsConst::YEAR, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 19), nValue8, m_Params.nYear, ltcparams::Mask::YEAR);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::MONTH, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 1) && (nValue8 <= 12), nValue8, m_Params.nMonth, ltcparams::Mask::MONTH);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::DAY, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 1) && (nValue8 <= 31), nValue8, m_Params.nDay, ltcparams::Mask::DAY);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::FPS, nValue8) == Sscan::OK) {
		SetValue((nValue8 >= 24) && (nValue8 <= 30), nValue8, m_Params.nFps, ltcparams::Mask::FPS);
		ltc::g_Type = ltc::get_type(m_Params.nFps);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_FRAME, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 30), nValue8, m_Params.nStartFrame, ltcparams::Mask::START_FRAME);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_SECOND, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_Params.nStartSecond, ltcparams::Mask::START_SECOND);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_MINUTE, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_Params.nStartMinute, ltcparams::Mask::START_MINUTE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::START_HOUR, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 23), nValue8, m_Params.nStartHour, ltcparams::Mask::START_HOUR);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_FRAME, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 30), nValue8, m_Params.nStopFrame, ltcparams::Mask::STOP_FRAME);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_SECOND, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_Params.nStopSecond, ltcparams::Mask::STOP_SECOND);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_MINUTE, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 59), nValue8, m_Params.nStopMinute, ltcparams::Mask::STOP_MINUTE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::STOP_HOUR, nValue8) == Sscan::OK) {
		SetValue((nValue8 <= 23), nValue8, m_Params.nStopHour, ltcparams::Mask::STOP_HOUR);
		return;
	}

#if 0
	if (Sscan::Uint8(pLine, LtcParamsConst::SET_DATE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_Params.nSetDate, ltcparams::Mask::SET_DATE);
	}
#endif

	if (Sscan::Uint8(pLine, LtcParamsConst::ALT_FUNCTION, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_Params.nAltFunction, ltcparams::Mask::ALT_FUNCTION);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_SECONDS, nValue8) == Sscan::OK) {
		SetValue((nValue8 > 0) && (nValue8 <= 99), nValue8, m_Params.nSkipSeconds, ltcparams::Mask::SKIP_SECONDS);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::SKIP_FREE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_Params.nSkipFree, ltcparams::Mask::SKIP_FREE);
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::IGNORE_START, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::IGNORE_START;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::IGNORE_START;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::IGNORE_STOP, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::IGNORE_STOP;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::IGNORE_STOP;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::OSC_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ltcparams::Mask::OSC_ENABLE;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::OSC_ENABLE;
		}
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, LtcParamsConst::OSC_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nOscPort = nValue16;
			m_Params.nSetList |= ltcparams::Mask::OSC_PORT;
		} else {
			m_Params.nOscPort = 8000;
			m_Params.nSetList &= ~ltcparams::Mask::OSC_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::WS28XX_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nRgbLedType = static_cast<uint8_t>(ltcparams::RgbLedType::WS28XX);
			m_Params.nSetList |= ltcparams::Mask::RGBLEDTYPE;
		} else {
			m_Params.nRgbLedType &= static_cast<uint8_t>(~ltcparams::RgbLedType::WS28XX);

			if (m_Params.nRgbLedType == 0) {
				m_Params.nSetList &= ~ltcparams::Mask::RGBLEDTYPE;
			}
		}
		return;
	}

	if (Sscan::Uint8(pLine, LtcParamsConst::RGBPANEL_ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nRgbLedType = static_cast<uint8_t>(ltcparams::RgbLedType::RGBPANEL);
			m_Params.nSetList |= ltcparams::Mask::RGBLEDTYPE;
		} else {
			m_Params.nRgbLedType &= static_cast<uint8_t>(~ltcparams::RgbLedType::RGBPANEL);

			if (m_Params.nRgbLedType == 0) {
				m_Params.nSetList &= ~ltcparams::Mask::RGBLEDTYPE;
			}
		}
		return;
	}

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, LtcParamsConst::TIMECODE_IP, nValue32) == Sscan::OK) {
		if (Network::Get()->IsValidIp(nValue32)) {
			m_Params.nSetList |= ltcparams::Mask::TIMECODE_IP;
			m_Params.nTimeCodeIp = nValue32;
		} else {
			m_Params.nSetList &= ~ltcparams::Mask::TIMECODE_IP;
			m_Params.nTimeCodeIp = Network::Get()->GetBroadcastIp();
		}
		return;
	}
}

void LtcParams::Set(struct ltc::TimeCode *pStartTimeCode, struct ltc::TimeCode *pStopTimeCode) {
#if 0
	ltc::g_DisabledOutputs.bOled = isDisabledOutputMaskSet(Destination::Output::DISPLAY_OLED);
	ltc::g_DisabledOutputs.bMax7219 = isDisabledOutputMaskSet(Destination::Output::MAX7219);
	ltc::g_DisabledOutputs.bMidi = isDisabledOutputMaskSet(Destination::Output::MIDI);
	ltc::g_DisabledOutputs.bArtNet = isDisabledOutputMaskSet(Destination::Output::ARTNET);
	ltc::g_DisabledOutputs.bLtc = isDisabledOutputMaskSet(Destination::Output::LTC);
	ltc::g_DisabledOutputs.bEtc = isDisabledOutputMaskSet(Destination::Output::ETC);
	ltc::g_DisabledOutputs.bNtp = !isMaskSet(ltcparams::Mask::NTP_ENABLE);
	ltc::g_DisabledOutputs.bRtpMidi = isDisabledOutputMaskSet(Destination::Output::RTPMIDI);
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	ltc::g_DisabledOutputs.bWS28xx = (m_Params.nRgbLedType != ltcparams::RgbLedType::WS28XX);
#else
	ltc::g_DisabledOutputs.bWS28xx = true;
#endif
#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
	ltc::g_DisabledOutputs.bRgbPanel = (m_Params.nRgbLedType != ltcparams::RgbLedType::RGBPANEL);
#else
	ltc::g_DisabledOutputs.bRgbPanel = true;
#endif
#endif
	/*
	 *
	 */

	ltc::g_nDisabledOutputs = m_Params.nDisabledOutputs;
	ltc::Destination::SetDisabled(Destination::Output::NTP_SERVER, !isMaskSet(ltcparams::Mask::NTP_ENABLE));
#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	ltc::Destination::SetDisabled(Destination::Output::WS28XX, (m_Params.nRgbLedType != ltcparams::RgbLedType::WS28XX));
#else
	ltc::Destination::SetDisabled(Destination::Output::WS28XX);
#endif
#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
	ltc::Destination::SetDisabled(Destination::Output::RGBPANEL, (m_Params.nRgbLedType != ltcparams::RgbLedType::RGBPANEL));
#else
	ltc::Destination::SetDisabled(Destination::Output::RGBPANEL);
#endif

	/*
	 *
	 */

	assert(pStartTimeCode != nullptr);

	if ((isMaskSet(ltcparams::Mask::START_FRAME)) || (isMaskSet(ltcparams::Mask::START_SECOND)) || (isMaskSet(ltcparams::Mask::START_MINUTE)) || (isMaskSet(ltcparams::Mask::START_HOUR)) ) {
		memset(pStartTimeCode, 0, sizeof(struct ltc::TimeCode));

		if (isMaskSet(ltcparams::Mask::START_FRAME)) {
			pStartTimeCode->nFrames = m_Params.nStartFrame;
		}

		if (isMaskSet(ltcparams::Mask::START_SECOND)) {
			pStartTimeCode->nSeconds = m_Params.nStartSecond;
		}

		if (isMaskSet(ltcparams::Mask::START_MINUTE)) {
			pStartTimeCode->nMinutes = m_Params.nStartMinute;
		}

		if (isMaskSet(ltcparams::Mask::START_HOUR)) {
			pStartTimeCode->nHours = m_Params.nStartHour;
		}
	} else {
		pStartTimeCode->nFrames = m_Params.nStartFrame;
		pStartTimeCode->nSeconds = m_Params.nStartSecond;
		pStartTimeCode->nMinutes = m_Params.nStartMinute;
		pStartTimeCode->nHours = m_Params.nStartHour;
	}

	pStartTimeCode->nType = static_cast<uint8_t>(ltc::g_Type);

	assert(pStopTimeCode != nullptr);

	if ((isMaskSet(ltcparams::Mask::STOP_FRAME)) || (isMaskSet(ltcparams::Mask::STOP_SECOND)) || (isMaskSet(ltcparams::Mask::STOP_MINUTE)) || (isMaskSet(ltcparams::Mask::STOP_HOUR)) ) {
		memset(pStopTimeCode, 0, sizeof(struct ltc::TimeCode));

		if (isMaskSet(ltcparams::Mask::STOP_FRAME)) {
			pStopTimeCode->nFrames = m_Params.nStopFrame;
		}

		if (isMaskSet(ltcparams::Mask::STOP_SECOND)) {
			pStopTimeCode->nSeconds = m_Params.nStopSecond;
		}

		if (isMaskSet(ltcparams::Mask::STOP_MINUTE)) {
			pStopTimeCode->nMinutes = m_Params.nStopMinute;
		}

		if (isMaskSet(ltcparams::Mask::STOP_HOUR)) {
			pStopTimeCode->nHours = m_Params.nStopHour;
		}
	} else {
		pStopTimeCode->nFrames = m_Params.nStopFrame;
		pStopTimeCode->nSeconds = m_Params.nStopSecond;
		pStopTimeCode->nMinutes = m_Params.nStopMinute;
		pStopTimeCode->nHours = m_Params.nStopHour;
	}

	pStopTimeCode->nType = static_cast<uint8_t>(ltc::g_Type);
}

void LtcParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<LtcParams*>(p))->callbackFunction(s);
}

void LtcParams::Builder(const struct ltcparams::Params *ptLtcParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptLtcParams != nullptr) {
		memcpy(&m_Params, ptLtcParams, sizeof(struct ltcparams::Params));
	} else {
		ltcparams::store::copy(&m_Params);
	}

	PropertiesBuilder builder(LtcParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(LtcParamsConst::SOURCE, GetSourceType(static_cast<Source>(m_Params.nSource)), isMaskSet(ltcparams::Mask::SOURCE));

	builder.AddComment("Disable outputs");
	builder.Add(LtcParamsConst::DISABLE_DISPLAY_OLED, isDisabledOutputMaskSet(Destination::Output::DISPLAY_OLED));
	builder.Add(LtcParamsConst::DISABLE_MAX7219, isDisabledOutputMaskSet(Destination::Output::MAX7219));
	builder.Add(LtcParamsConst::DISABLE_LTC, isDisabledOutputMaskSet(Destination::Output::LTC));
	builder.Add(LtcParamsConst::DISABLE_MIDI, isDisabledOutputMaskSet(Destination::Output::MIDI));
	builder.Add(LtcParamsConst::DISABLE_ARTNET, isDisabledOutputMaskSet(Destination::Output::ARTNET));
	builder.Add(LtcParamsConst::DISABLE_RTPMIDI, isDisabledOutputMaskSet(Destination::Output::RTPMIDI));
	builder.Add(LtcParamsConst::DISABLE_ETC, isDisabledOutputMaskSet(Destination::Output::ETC));

	builder.AddComment("System clock / RTC");
	builder.Add(LtcParamsConst::SHOW_SYSTIME, isMaskSet(ltcparams::Mask::SHOW_SYSTIME));
	builder.Add(LtcParamsConst::DISABLE_TIMESYNC, isMaskSet(ltcparams::Mask::DISABLE_TIMESYNC));

	builder.AddComment("source=systime");
	builder.Add(LtcParamsConst::AUTO_START, isMaskSet(ltcparams::Mask::AUTO_START));
	builder.Add(LtcParamsConst::GPS_START, isMaskSet(ltcparams::Mask::GPS_START));
	builder.AddUtcOffset(LtcParamsConst::UTC_OFFSET, m_Params.nUtcOffsetHours, m_Params.nUtcOffsetMinutes);

	builder.AddComment("source=internal");
	builder.Add(LtcParamsConst::FPS, m_Params.nFps, isMaskSet(ltcparams::Mask::FPS));
	builder.Add(LtcParamsConst::START_HOUR, m_Params.nStartHour, isMaskSet(ltcparams::Mask::START_HOUR));
	builder.Add(LtcParamsConst::START_MINUTE, m_Params.nStartMinute, isMaskSet(ltcparams::Mask::START_MINUTE));
	builder.Add(LtcParamsConst::START_SECOND, m_Params.nStartSecond, isMaskSet(ltcparams::Mask::START_SECOND));
	builder.Add(LtcParamsConst::START_FRAME, m_Params.nStartFrame, isMaskSet(ltcparams::Mask::START_FRAME));
	builder.Add(LtcParamsConst::STOP_HOUR, m_Params.nStopHour, isMaskSet(ltcparams::Mask::STOP_HOUR));
	builder.Add(LtcParamsConst::STOP_MINUTE,  m_Params.nStopMinute, isMaskSet(ltcparams::Mask::STOP_MINUTE));
	builder.Add(LtcParamsConst::STOP_SECOND, m_Params.nStopSecond, isMaskSet(ltcparams::Mask::STOP_SECOND));
	builder.Add(LtcParamsConst::STOP_FRAME, m_Params.nStopFrame, isMaskSet(ltcparams::Mask::STOP_FRAME));
	builder.Add(LtcParamsConst::SKIP_FREE, m_Params.nSkipFree, isMaskSet(ltcparams::Mask::SKIP_FREE));
	builder.Add(LtcParamsConst::IGNORE_START, isMaskSet(ltcparams::Mask::IGNORE_START));
	builder.Add(LtcParamsConst::IGNORE_STOP, isMaskSet(ltcparams::Mask::IGNORE_STOP));
	builder.AddComment("MCP buttons");
	builder.Add(LtcParamsConst::ALT_FUNCTION, isMaskSet(ltcparams::Mask::ALT_FUNCTION));
	builder.Add(LtcParamsConst::SKIP_SECONDS, m_Params.nSkipSeconds, isMaskSet(ltcparams::Mask::SKIP_SECONDS));

	builder.AddComment("Art-Net output");
	builder.AddIpAddress(LtcParamsConst::TIMECODE_IP, m_Params.nTimeCodeIp, isMaskSet(ltcparams::Mask::TIMECODE_IP));

	builder.AddComment("LTC output");
	builder.Add(LtcParamsConst::VOLUME, m_Params.nVolume, isMaskSet(ltcparams::Mask::VOLUME));

	builder.AddComment("NTP Server");
	builder.Add(LtcParamsConst::NTP_ENABLE, isMaskSet(ltcparams::Mask::NTP_ENABLE));
	builder.Add(LtcParamsConst::YEAR, m_Params.nYear, isMaskSet(ltcparams::Mask::YEAR));
	builder.Add(LtcParamsConst::MONTH, m_Params.nMonth, isMaskSet(ltcparams::Mask::MONTH));
	builder.Add(LtcParamsConst::DAY, m_Params.nDay, isMaskSet(ltcparams::Mask::DAY));

	builder.AddComment("OSC Server");
	builder.Add(LtcParamsConst::OSC_ENABLE, isMaskSet(ltcparams::Mask::OSC_ENABLE));
	builder.Add(LtcParamsConst::OSC_PORT, m_Params.nOscPort, isMaskSet(ltcparams::Mask::OSC_PORT));

	builder.AddComment("WS28xx display");
	builder.Add(LtcParamsConst::WS28XX_ENABLE, m_Params.nRgbLedType == ltcparams::RgbLedType::WS28XX, m_Params.nRgbLedType == ltcparams::RgbLedType::WS28XX);

	builder.AddComment("RGB panel");
	builder.Add(LtcParamsConst::RGBPANEL_ENABLE, m_Params.nRgbLedType == ltcparams::RgbLedType::RGBPANEL, m_Params.nRgbLedType == ltcparams::RgbLedType::RGBPANEL);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

#include <cstdio>

void LtcParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcParamsConst::FILE_NAME);

	if (isMaskSet(ltcparams::Mask::SOURCE)) {
		printf(" %s=%d [%s]\n", LtcParamsConst::SOURCE, m_Params.nSource, GetSourceType(static_cast<ltc::Source>(m_Params.nSource)));
	}

	if (isMaskSet(ltcparams::Mask::VOLUME)) {
		printf(" %s=%d\n", LtcParamsConst::VOLUME, m_Params.nVolume);
	}

	printf(" %s=%d\n", LtcParamsConst::AUTO_START, isMaskSet(ltcparams::Mask::AUTO_START));
	printf(" %s=%d\n", LtcParamsConst::GPS_START, isMaskSet(ltcparams::Mask::GPS_START));

	if (isMaskSet(ltcparams::Mask::DISABLED_OUTPUTS)) {
		printf(" Disabled outputs %.2x:\n", m_Params.nDisabledOutputs);

		if (isDisabledOutputMaskSet(Destination::Output::DISPLAY_OLED)) {
			printf("  Display OLED\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::MAX7219)) {
			printf("  Max7219\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::MIDI)) {
			printf("  MIDI\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::RTPMIDI)) {
			printf("  RtpMIDI\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::ARTNET)) {
			printf("  Art-Net\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::ETC)) {
			printf("  ETC\n");
		}

		if (isDisabledOutputMaskSet(Destination::Output::LTC)) {
			printf("  LTC\n");
		}
	}

	if (isMaskSet(ltcparams::Mask::YEAR)) {
		printf(" %s=%d\n", LtcParamsConst::YEAR, m_Params.nYear);
	}

	if (isMaskSet(ltcparams::Mask::MONTH)) {
		printf(" %s=%d\n", LtcParamsConst::MONTH, m_Params.nMonth);
	}

	if (isMaskSet(ltcparams::Mask::DAY)) {
		printf(" %s=%d\n", LtcParamsConst::DAY, m_Params.nDay);
	}

	printf(" %s=%d\n", LtcParamsConst::NTP_ENABLE, isMaskSet(ltcparams::Mask::NTP_ENABLE));

	if (isMaskSet(ltcparams::Mask::FPS)) {
		printf(" %s=%d\n", LtcParamsConst::FPS, m_Params.nFps);
	}

	if (isMaskSet(ltcparams::Mask::START_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::START_FRAME, m_Params.nStartFrame);
	}

	if (isMaskSet(ltcparams::Mask::START_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::START_SECOND, m_Params.nStartSecond);
	}

	if (isMaskSet(ltcparams::Mask::START_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::START_MINUTE, m_Params.nStartMinute);
	}

	if (isMaskSet(ltcparams::Mask::START_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::START_HOUR, m_Params.nStartHour);
	}

	if (isMaskSet(ltcparams::Mask::STOP_FRAME)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_FRAME, m_Params.nStopFrame);
	}

	if (isMaskSet(ltcparams::Mask::STOP_SECOND)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_SECOND, m_Params.nStopSecond);
	}

	if (isMaskSet(ltcparams::Mask::STOP_MINUTE)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_MINUTE, m_Params.nStopMinute);
	}

	if (isMaskSet(ltcparams::Mask::STOP_HOUR)) {
		printf(" %s=%d\n", LtcParamsConst::STOP_HOUR, m_Params.nStopHour);
	}

	if (isMaskSet(ltcparams::Mask::ALT_FUNCTION)) {
		printf(" %s=%d\n", LtcParamsConst::ALT_FUNCTION, m_Params.nAltFunction);
	}

	if (isMaskSet(ltcparams::Mask::SKIP_SECONDS)) {
		printf(" %s=%d\n", LtcParamsConst::SKIP_SECONDS, m_Params.nSkipSeconds);
	}

	printf(" %s=%d\n", LtcParamsConst::IGNORE_START, isMaskSet(ltcparams::Mask::IGNORE_START));
	printf(" %s=%d\n", LtcParamsConst::IGNORE_STOP, isMaskSet(ltcparams::Mask::IGNORE_STOP));

#if 0
	if (isMaskSet(ltcparams::Mask::SET_DATE)) {
		printf(" %s=%d\n", LtcParamsConst::SET_DATE, m_Params.nSetDate);
	}
#endif

	if (isMaskSet(ltcparams::Mask::OSC_ENABLE)) {
		printf(" OSC is enabled\n");

		if (isMaskSet(ltcparams::Mask::OSC_PORT)) {
			printf(" %s=%d\n", LtcParamsConst::OSC_PORT, m_Params.nOscPort);
		}
	}

	if (isMaskSet(ltcparams::Mask::RGBLEDTYPE)) {
		if (m_Params.nRgbLedType == static_cast<uint8_t>(ltcparams::RgbLedType::WS28XX)) {
			printf(" WS28xx is enabled\n");
		} else if (m_Params.nRgbLedType == static_cast<uint8_t>(ltcparams::RgbLedType::RGBPANEL)) {
			printf(" RGB panel is enabled\n");
		} else {
			printf("nRgbLedType=%u\n", m_Params.nRgbLedType);
		}
	}
}
