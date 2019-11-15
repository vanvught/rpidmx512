/**
 * @file ltcparamssave.h
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "ltcparams.h"

#include "propertiesbuilder.h"

#include "ltcparamsconst.h"

#include "debug.h"

void LtcParams::Builder(const struct TLtcParams *ptLtcParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (ptLtcParams != 0) {
		memcpy(&m_tLtcParams, ptLtcParams, sizeof(struct TLtcParams));
	} else {
		m_pLTcParamsStore->Copy(&m_tLtcParams);
	}

	PropertiesBuilder builder(LtcParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(LtcParamsConst::SOURCE, GetSourceType((TLtcReaderSource) m_tLtcParams.tSource), isMaskSet(LTC_PARAMS_MASK_SOURCE));

	builder.Add(LtcParamsConst::DISABLE_DISPLAY, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_DISPLAY), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_DISPLAY));
	builder.Add(LtcParamsConst::DISABLE_MAX7219, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MAX7219), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MAX7219));
	builder.Add(LtcParamsConst::DISABLE_LTC, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_LTC), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_LTC));
	builder.Add(LtcParamsConst::DISABLE_MIDI, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MIDI), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MIDI));
	builder.Add(LtcParamsConst::DISABLE_ARTNET, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_ARTNET), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_ARTNET));
	builder.Add(LtcParamsConst::DISABLE_TCNET, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_TCNET), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_TCNET));
	builder.Add(LtcParamsConst::DISABLE_RTPMIDI, isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_RTPMIDI), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_RTPMIDI));

	builder.Add(LtcParamsConst::SHOW_SYSTIME, m_tLtcParams.nShowSysTime, isMaskSet(LTC_PARAMS_MASK_SHOW_SYSTIME));
	builder.Add(LtcParamsConst::DISABLE_TIMESYNC, m_tLtcParams.nDisableTimeSync, isMaskSet(LTC_PARAMS_MASK_DISABLE_TIMESYNC));

	builder.Add(LtcParamsConst::YEAR, m_tLtcParams.nYear, isMaskSet(LTC_PARAMS_MASK_YEAR));
	builder.Add(LtcParamsConst::MONTH, m_tLtcParams.nMonth, isMaskSet(LTC_PARAMS_MASK_MONTH));
	builder.Add(LtcParamsConst::DAY, m_tLtcParams.nDay, isMaskSet(LTC_PARAMS_MASK_DAY));

	builder.Add(LtcParamsConst::NTP_ENABLE, m_tLtcParams.nEnableNtp, isMaskSet(LTC_PARAMS_MASK_ENABLE_NTP));

	builder.Add(LtcParamsConst::FPS, m_tLtcParams.nFps, isMaskSet(LTC_PARAMS_MASK_FPS));
	builder.Add(LtcParamsConst::START_HOUR, m_tLtcParams.nStartHour, isMaskSet(LTC_PARAMS_MASK_START_HOUR));
	builder.Add(LtcParamsConst::START_MINUTE, m_tLtcParams.nStartMinute, isMaskSet(LTC_PARAMS_MASK_START_MINUTE));
	builder.Add(LtcParamsConst::START_SECOND, m_tLtcParams.nStartSecond, isMaskSet(LTC_PARAMS_MASK_START_SECOND));
	builder.Add(LtcParamsConst::START_FRAME, m_tLtcParams.nStartFrame, isMaskSet(LTC_PARAMS_MASK_START_FRAME));
	builder.Add(LtcParamsConst::STOP_HOUR, m_tLtcParams.nStopHour, isMaskSet(LTC_PARAMS_MASK_STOP_HOUR));
	builder.Add(LtcParamsConst::STOP_MINUTE,  m_tLtcParams.nStopMinute, isMaskSet(LTC_PARAMS_MASK_STOP_MINUTE));
	builder.Add(LtcParamsConst::STOP_SECOND, m_tLtcParams.nStopSecond, isMaskSet(LTC_PARAMS_MASK_STOP_SECOND));
	builder.Add(LtcParamsConst::STOP_FRAME, m_tLtcParams.nStopFrame, isMaskSet(LTC_PARAMS_MASK_STOP_FRAME));
	//builder.Add(LtcParamsConst::SET_DATE, (uint32_t) m_tLtcParams.nSetDate, isMaskSet(LTC_PARAMS_MASK_SET_DATE));


	builder.Add(LtcParamsConst::OSC_ENABLE, m_tLtcParams.nEnableOsc, isMaskSet(LTC_PARAMS_MASK_ENABLE_OSC));
	builder.Add(LtcParamsConst::OSC_PORT, m_tLtcParams.nOscPort, isMaskSet(LTC_PARAMS_MASK_OSC_PORT));

	builder.Add(LtcParamsConst::WS28XX_ENABLE, m_tLtcParams.nEnableWS28xx, isMaskSet(LTC_PARAMS_MASK_ENABLE_WS28XX));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);

	DEBUG_EXIT
	return;
}

void LtcParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pLTcParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	return;
}
