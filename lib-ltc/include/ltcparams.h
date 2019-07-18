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

#ifndef LTCPARAMS_H_
#define LTCPARAMS_H_

#include <stdint.h>

#include "ltc.h"
#include "displaymax7219.h"

enum TLtcReaderSource {
	LTC_READER_SOURCE_LTC,
	LTC_READER_SOURCE_ARTNET,
	LTC_READER_SOURCE_MIDI,
	LTC_READER_SOURCE_TCNET,
	LTC_READER_SOURCE_INTERNAL,
	LTC_READER_SOURCE_UNDEFINED
};

enum TLtcParamsMax7219Type {
	LTC_PARAMS_MAX7219_TYPE_MATRIX,
	LTC_PARAMS_MAX7219_TYPE_7SEGMENT
};

enum TLtcParamsMaskDisabledOutputs {
	LTC_PARAMS_DISABLE_DISPLAY = (1 << 0),
	LTC_PARAMS_DISABLE_MAX7219 = (1 << 1),
	LTC_PARAMS_DISABLE_MIDI = (1 << 2),
	LTC_PARAMS_DISABLE_ARTNET = (1 << 3),
	LTC_PARAMS_DISABLE_TCNET = (1 << 4),
	LTC_PARAMS_DISABLE_LTC = (1 << 5),
	LTC_PARAMS_DISABLE_NTP = (1 << 6)
};

struct TLtcParams {
	uint32_t nSetList;
	uint8_t tSource;
	uint8_t tMax7219Type;
	uint8_t nMax7219Intensity;
	uint8_t nDisabledOutputs;
	uint8_t nShowSysTime;
	uint8_t nDisableTimeSync;
	uint8_t nYear;
	uint8_t nMonth;
	uint8_t nDay;
	uint8_t nEnableNtp;
	uint8_t nSetDate;
	uint8_t nFps;
	uint8_t nStartFrame;
	uint8_t nStartSecond;
	uint8_t nStartMinute;
	uint8_t nStartHour;
	uint8_t nStopFrame;
	uint8_t nStopSecond;
	uint8_t nStopMinute;
	uint8_t nStopHour;
};

enum TLtcParamsMask {
	LTC_PARAMS_MASK_SOURCE = (1 << 0),
	LTC_PARAMS_MASK_MAX7219_TYPE = (1 << 1),
	LTC_PARAMS_MASK_MAX7219_INTENSITY = (1 << 2),
	LTC_PARAMS_MASK_DISABLED_OUTPUTS = (1 << 3),
	LTC_PARAMS_MASK_SHOW_SYSTIME = (1 << 4),
	LTC_PARAMS_MASK_DISABLE_TIMESYNC = (1 << 5),
	LTC_PARAMS_MASK_YEAR = (1 << 6),
	LTC_PARAMS_MASK_MONTH = (1 << 7),
	LTC_PARAMS_MASK_DAY = (1 << 8),
	LTC_PARAMS_MASK_ENABLE_NTP = (1 << 9),
	LTC_PARAMS_MASK_SET_DATE = (1 << 10),
	LTC_PARAMS_MASK_FPS = (1 << 11),
	LTC_PARAMS_MASK_START_FRAME = (1 << 12),
	LTC_PARAMS_MASK_START_SECOND = (1 << 13),
	LTC_PARAMS_MASK_START_MINUTE = (1 << 14),
	LTC_PARAMS_MASK_START_HOUR = (1 << 15),
	LTC_PARAMS_MASK_STOP_FRAME = (1 << 16),
	LTC_PARAMS_MASK_STOP_SECOND = (1 << 17),
	LTC_PARAMS_MASK_STOP_MINUTE = (1 << 18),
	LTC_PARAMS_MASK_STOP_HOUR = (1 << 19)
};

class LtcParamsStore {
public:
	virtual ~LtcParamsStore(void);

	virtual void Update(const struct TLtcParams *pTLtcParams)=0;
	virtual void Copy(struct TLtcParams *pTLtcParams)=0;
};

class LtcParams {
public:
	LtcParams(LtcParamsStore *pLtcParamsStore = 0);
	~LtcParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TLtcParams *ptLtcParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	TLtcReaderSource GetSource(void) {
		return (TLtcReaderSource) m_tLtcParams.tSource;
	}

	const char *GetSourceType(enum TLtcReaderSource tSource);
	enum TLtcReaderSource GetSourceType(const char *pType);

	uint8_t GetMax7219Intensity(void) {
		return m_tLtcParams.nMax7219Intensity;
	}

	tMax7219Types GetMax7219Type(void) {
		return (tMax7219Types) m_tLtcParams.tMax7219Type;
	}

	void CopyDisabledOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs);

	bool IsShowSysTime(void) {
		return (m_tLtcParams.nShowSysTime != 0);
	}

	bool IsTimeSyncDisabled(void) {
		return  (m_tLtcParams.nDisableTimeSync == 1);
	}

	uint8_t GetYear(void) {
		return m_tLtcParams.nYear;
	}

	uint8_t GetMonth(void) {
		return m_tLtcParams.nMonth;
	}

	uint8_t GetDay(void) {
		return m_tLtcParams.nDay;
	}

	bool IsNtpEnabled(void) {
		return (m_tLtcParams.nEnableNtp == 1);
	}

	bool IsSetDate(void) {
		return (m_tLtcParams.nSetDate == 1);
	}

	uint8_t GetFps(void) {
		return m_tLtcParams.nFps;
	}

	uint8_t GetStartFrame(void) {
		return m_tLtcParams.nStartFrame;
	}

	uint8_t GetStartSecond(void) {
		return m_tLtcParams.nStartSecond;
	}

	uint8_t GetStartMinute(void) {
		return m_tLtcParams.nStartMinute;
	}

	uint8_t GetStartHour(void) {
		return m_tLtcParams.nStartHour;
	}

	uint8_t GetStopFrame(void) {
		return m_tLtcParams.nStopFrame;
	}

	uint8_t GetStopSecond(void) {
		return m_tLtcParams.nStopSecond;
	}

	uint8_t GetStopMinute(void) {
		return m_tLtcParams.nStopMinute;
	}

	uint8_t GetStopHour(void) {
		return m_tLtcParams.nStopHour;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;
	bool isDisabledOutputMaskSet(uint8_t nMask) const;

private:
    LtcParamsStore 	*m_pLTcParamsStore;
    struct TLtcParams m_tLtcParams;
};

#endif /* LTCPARAMS_H_ */
