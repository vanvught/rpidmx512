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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "ltcparams.h"

#include "propertiesbuilder.h"

#include "ltcparamsconst.h"

#include "debug.h"

bool LtcParams::Builder(const struct TLtcParams *ptLtcParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (ptLtcParams != 0) {
		memcpy(&m_tLtcParams, ptLtcParams, sizeof(struct TLtcParams));
	} else {
		m_pLTcParamsStore->Copy(&m_tLtcParams);
	}

	PropertiesBuilder builder(LtcParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(LtcParamsConst::SOURCE, GetSourceType((TLtcReaderSource) m_tLtcParams.tSource));

	isAdded &= builder.Add(LtcParamsConst::MAX7219_TYPE, m_tLtcParams.tMax7219Type == LTC_PARAMS_MAX7219_TYPE_7SEGMENT ? "7segment" : "matrix" , isMaskSet(LTC_PARAMS_MASK_MAX7219_TYPE));
	isAdded &= builder.Add(LtcParamsConst::MAX7219_INTENSITY, (uint32_t) m_tLtcParams.nMax7219Intensity, isMaskSet(LTC_PARAMS_MASK_MAX7219_INTENSITY));

	isAdded &= builder.Add(LtcParamsConst::DISABLE_DISPLAY, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_DISPLAY), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_DISPLAY));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_MAX7219, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MAX7219), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MAX7219));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_LTC, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_LTC), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_LTC));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_MIDI, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MIDI), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_MIDI));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_ARTNET, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_ARTNET), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_ARTNET));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_TCNET, (uint32_t) isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_TCNET), isDisabledOutputMaskSet(LTC_PARAMS_DISABLE_TCNET));

	isAdded &= builder.Add(LtcParamsConst::SHOW_SYSTIME, (uint32_t) m_tLtcParams.nShowSysTime, isMaskSet(LTC_PARAMS_MASK_SHOW_SYSTIME));
	isAdded &= builder.Add(LtcParamsConst::DISABLE_TIMESYNC, (uint32_t) m_tLtcParams.nDisableTimeSync, isMaskSet(LTC_PARAMS_MASK_DISABLE_TIMESYNC));

	isAdded &= builder.Add(LtcParamsConst::YEAR, (uint32_t) m_tLtcParams.nYear, isMaskSet(LTC_PARAMS_MASK_YEAR));
	isAdded &= builder.Add(LtcParamsConst::MONTH, (uint32_t) m_tLtcParams.nMonth, isMaskSet(LTC_PARAMS_MASK_MONTH));
	isAdded &= builder.Add(LtcParamsConst::DAY, (uint32_t) m_tLtcParams.nDay, isMaskSet(LTC_PARAMS_MASK_DAY));

	isAdded &= builder.Add(LtcParamsConst::NTP_ENABLE, (uint32_t) m_tLtcParams.nEnableNtp, isMaskSet(LTC_PARAMS_MASK_ENABLE_NTP));

	isAdded &= builder.Add(LtcParamsConst::FPS, (uint32_t) m_tLtcParams.nFps, isMaskSet(LTC_PARAMS_MASK_FPS));
	isAdded &= builder.Add(LtcParamsConst::START_HOUR, (uint32_t) m_tLtcParams.nStartHour, isMaskSet(LTC_PARAMS_MASK_START_HOUR));
	isAdded &= builder.Add(LtcParamsConst::START_MINUTE, (uint32_t) m_tLtcParams.nStartMinute, isMaskSet(LTC_PARAMS_MASK_START_MINUTE));
	isAdded &= builder.Add(LtcParamsConst::START_SECOND, (uint32_t) m_tLtcParams.nStartSecond, isMaskSet(LTC_PARAMS_MASK_START_SECOND));
	isAdded &= builder.Add(LtcParamsConst::START_FRAME, (uint32_t) m_tLtcParams.nStartFrame, isMaskSet(LTC_PARAMS_MASK_START_FRAME));
	isAdded &= builder.Add(LtcParamsConst::STOP_HOUR, (uint32_t) m_tLtcParams.nStopHour, isMaskSet(LTC_PARAMS_MASK_STOP_HOUR));
	isAdded &= builder.Add(LtcParamsConst::STOP_MINUTE, (uint32_t) m_tLtcParams.nStopMinute, isMaskSet(LTC_PARAMS_MASK_STOP_MINUTE));
	isAdded &= builder.Add(LtcParamsConst::STOP_SECOND, (uint32_t) m_tLtcParams.nStopSecond, isMaskSet(LTC_PARAMS_MASK_STOP_SECOND));
	isAdded &= builder.Add(LtcParamsConst::STOP_FRAME, (uint32_t) m_tLtcParams.nStopFrame, isMaskSet(LTC_PARAMS_MASK_STOP_FRAME));

#if 0
	isAdded &= builder.Add(LtcParamsConst::SET_DATE, (uint32_t) m_tLtcParams.nSetDate, isMaskSet(LTC_PARAMS_MASK_SET_DATE));
#endif

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool LtcParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pLTcParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
