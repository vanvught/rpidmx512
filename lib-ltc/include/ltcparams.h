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

enum TLtcParamsMaskDisabledOutputs {
	LTC_PARAMS_DISABLE_DISPLAY = (1 << 0),
	LTC_PARAMS_DISABLE_MAX7219 = (1 << 1),
	LTC_PARAMS_DISABLE_MIDI = (1 << 2),
	LTC_PARAMS_DISABLE_ARTNET = (1 << 3),
	LTC_PARAMS_DISABLE_TCNET = (1 << 4),
	LTC_PARAMS_DISABLE_LTC = (1 << 5),
	LTC_PARAMS_DISABLE_NTP = (1 << 6),		// Not Used. TODO Subject for removal?
	LTC_PARAMS_DISABLE_RTPMIDI = (1 << 7)
};

// Note : This struct is almost size 32 bytes
struct TLtcParams {
	uint32_t nSetList;
	uint8_t tSource;
	uint8_t nReserved1;
	uint8_t nReserved2;
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
	uint8_t nEnableOsc;
	uint16_t nOscPort;
	uint8_t nEnableWS28xx;
}__attribute__((packed));

enum TLtcParamsMask {
	LTC_PARAMS_MASK_SOURCE = (1 << 0),
	LTC_PARAMS_MASK_RESERVED1 = (1 << 1),
	LTC_PARAMS_MASK_RESERVED2 = (1 << 2),
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
	LTC_PARAMS_MASK_STOP_HOUR = (1 << 19),
	LTC_PARAMS_MASK_ENABLE_OSC = (1 << 20),
	LTC_PARAMS_MASK_OSC_PORT = (1 << 21),
	LTC_PARAMS_MASK_ENABLE_WS28XX = (1 << 22)
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

	void Builder(const struct TLtcParams *ptLtcParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	TLtcReaderSource GetSource(void) {
		return (TLtcReaderSource) m_tLtcParams.tSource;
	}

	const char *GetSourceType(enum TLtcReaderSource tSource);
	enum TLtcReaderSource GetSourceType(const char *pType);

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

	bool IsOscEnabled(void) {
		return (m_tLtcParams.nEnableOsc == 1);
	}

	uint16_t GetOscPort(bool& bIsSet) {
		bIsSet = isMaskSet(LTC_PARAMS_MASK_OSC_PORT);
		return m_tLtcParams.nOscPort;
	}

	bool IsWS28xxEnabled(void) {
		return (m_tLtcParams.nEnableWS28xx == 1);
	}

	void StartTimeCodeCopyTo(TLtcTimeCode *ptStartTimeCode);
	void StopTimeCodeCopyTo(TLtcTimeCode *ptStopTimeCode);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;
	bool isDisabledOutputMaskSet(uint8_t nMask) const;
	void HandleDisabledOutput(const char *pLine, const char *pKeyword, TLtcParamsMaskDisabledOutputs tLtcParamsMaskDisabledOutputs);

private:
    LtcParamsStore 	*m_pLTcParamsStore;
    struct TLtcParams m_tLtcParams;
};

#endif /* LTCPARAMS_H_ */
