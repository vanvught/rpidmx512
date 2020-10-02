/**
 * @file ltcparamssave.h
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
#include <cassert>

#include "ltcparams.h"

#include "propertiesbuilder.h"

#include "ltcparamsconst.h"

#include "debug.h"

using namespace ltc;

void LtcParams::Builder(const struct TLtcParams *ptLtcParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
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
	builder.Add(LtcParamsConst::DISABLE_DISPLAY, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY));
	builder.Add(LtcParamsConst::DISABLE_MAX7219, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219));
	builder.Add(LtcParamsConst::DISABLE_LTC, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC));
	builder.Add(LtcParamsConst::DISABLE_MIDI, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI));
	builder.Add(LtcParamsConst::DISABLE_ARTNET, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET));
	builder.Add(LtcParamsConst::DISABLE_RTPMIDI, isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI), isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI));

	builder.AddComment("System clock / RTC");
	builder.Add(LtcParamsConst::SHOW_SYSTIME, m_tLtcParams.nShowSysTime, isMaskSet(LtcParamsMask::SHOW_SYSTIME));
	builder.Add(LtcParamsConst::DISABLE_TIMESYNC, m_tLtcParams.nDisableTimeSync, isMaskSet(LtcParamsMask::DISABLE_TIMESYNC));

	builder.AddComment("source=systime");
	builder.Add(LtcParamsConst::AUTO_START, m_tLtcParams.nAutoStart, isMaskSet(LtcParamsMask::AUTO_START));

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
	builder.Add(LtcParamsConst::ALT_FUNCTION, m_tLtcParams.nAltFunction, isMaskSet(LtcParamsMask::ALT_FUNCTION));
	builder.Add(LtcParamsConst::SKIP_SECONDS, m_tLtcParams.nSkipSeconds, isMaskSet(LtcParamsMask::SKIP_SECONDS));

	builder.AddComment("NTP Server");
	builder.Add(LtcParamsConst::NTP_ENABLE, m_tLtcParams.nEnableNtp, isMaskSet(LtcParamsMask::ENABLE_NTP));
	builder.Add(LtcParamsConst::YEAR, m_tLtcParams.nYear, isMaskSet(LtcParamsMask::YEAR));
	builder.Add(LtcParamsConst::MONTH, m_tLtcParams.nMonth, isMaskSet(LtcParamsMask::MONTH));
	builder.Add(LtcParamsConst::DAY, m_tLtcParams.nDay, isMaskSet(LtcParamsMask::DAY));

	builder.AddComment("OSC Server");
	builder.Add(LtcParamsConst::OSC_ENABLE, m_tLtcParams.nEnableOsc, isMaskSet(LtcParamsMask::ENABLE_OSC));
	builder.Add(LtcParamsConst::OSC_PORT, m_tLtcParams.nOscPort, isMaskSet(LtcParamsMask::OSC_PORT));

	builder.AddComment("WS28xx display");
	builder.Add(LtcParamsConst::WS28XX_ENABLE, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::WS28XX, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::WS28XX);

	builder.AddComment("RGB panel");
	builder.Add(LtcParamsConst::RGBPANEL_ENABLE, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::RGBPANEL, m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::RGBPANEL);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void LtcParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pLTcParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
