/**
 * @file ltcparams.h
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

#ifndef LTCPARAMS_H_
#define LTCPARAMS_H_

#include <stdint.h>

#include "ltcdisplaymax7219.h"

#include "ltc.h"

struct TLtcParamsRgbLedType {
	static constexpr auto WS28XX = (1U << 0);
	static constexpr auto RGBPANEL = (1U << 1);
};

struct TLtcParams {
	uint32_t nSetList;
	uint8_t tSource;
	uint8_t nAutoStart;
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
	uint8_t nRgbLedType;
	uint8_t nAltFunction;
	uint8_t nSkipSeconds;
	uint8_t nSkipFree;
}__attribute__((packed));

static_assert(sizeof(struct TLtcParams) <= 32, "struct TLtcParams is too large");

struct LtcParamsMask {
	static constexpr auto SOURCE = (1U << 0);
	static constexpr auto AUTO_START = (1U << 1);
	static constexpr auto RESERVED2 = (1U << 2);
	static constexpr auto DISABLED_OUTPUTS = (1U << 3);
	static constexpr auto SHOW_SYSTIME = (1U << 4);
	static constexpr auto DISABLE_TIMESYNC = (1U << 5);
	static constexpr auto YEAR = (1U << 6);
	static constexpr auto MONTH = (1U << 7);
	static constexpr auto DAY = (1U << 8);
	static constexpr auto ENABLE_NTP = (1U << 9);
	static constexpr auto SET_DATE = (1U << 10);
	static constexpr auto FPS = (1U << 11);
	static constexpr auto START_FRAME = (1U << 12);
	static constexpr auto START_SECOND = (1U << 13);
	static constexpr auto START_MINUTE = (1U << 14);
	static constexpr auto START_HOUR = (1U << 15);
	static constexpr auto STOP_FRAME = (1U << 16);
	static constexpr auto STOP_SECOND = (1U << 17);
	static constexpr auto STOP_MINUTE = (1U << 18);
	static constexpr auto STOP_HOUR = (1U << 19);
	static constexpr auto ENABLE_OSC = (1U << 20);
	static constexpr auto OSC_PORT = (1U << 21);
	static constexpr auto RGBLEDTYPE = (1U << 22);
	static constexpr auto ALT_FUNCTION = (1U << 23);
	static constexpr auto SKIP_SECONDS = (1U << 24);
	static constexpr auto SKIP_FREE = (1U << 25);
};

class LtcParamsStore {
public:
	virtual ~LtcParamsStore() {}

	virtual void Update(const struct TLtcParams *pTLtcParams)=0;
	virtual void Copy(struct TLtcParams *pTLtcParams)=0;

	virtual void SaveSource(uint8_t nSource)=0;
};

class LtcParams {
public:
	LtcParams(LtcParamsStore *pLtcParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TLtcParams *ptLtcParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump();

	ltc::source GetSource() {
		return static_cast<ltc::source>(m_tLtcParams.tSource);
	}

	const char *GetSourceType(ltc::source tSource);
	ltc::source GetSourceType(const char *pType);

	bool IsAutoStart() {
		return ((m_tLtcParams.nAutoStart != 0) && isMaskSet(LtcParamsMask::AUTO_START));
	}

	void CopyDisabledOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs);

	bool IsShowSysTime() const {
		return (m_tLtcParams.nShowSysTime != 0);
	}

	bool IsTimeSyncDisabled() const {
		return (m_tLtcParams.nDisableTimeSync == 1);
	}

	uint8_t GetYear() const {
		return m_tLtcParams.nYear;
	}

	uint8_t GetMonth() const {
		return m_tLtcParams.nMonth;
	}

	uint8_t GetDay() const {
		return m_tLtcParams.nDay;
	}

	bool IsNtpEnabled() const {
		return (m_tLtcParams.nEnableNtp == 1);
	}

	bool IsSetDate() const {
		return (m_tLtcParams.nSetDate == 1);
	}

	uint8_t GetFps() const {
		return m_tLtcParams.nFps;
	}

	bool IsOscEnabled() const {
		return (m_tLtcParams.nEnableOsc == 1);
	}

	uint16_t GetOscPort(bool &bIsSet) {
		bIsSet = isMaskSet(LtcParamsMask::OSC_PORT);
		return m_tLtcParams.nOscPort;
	}

	bool IsWS28xxEnabled() const {
		return (m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::WS28XX);
	}

	bool IsRgbPanelEnabled() const {
		return (m_tLtcParams.nRgbLedType == TLtcParamsRgbLedType::RGBPANEL);
	}

	bool IsAltFunction() const {
		return (m_tLtcParams.nAltFunction == 1);
	}

	uint8_t GetSkipSeconds() const {
		return m_tLtcParams.nSkipSeconds;
	}

	uint8_t GetSkipFree() const {
		return (m_tLtcParams.nSkipFree == 1);
	}

	void StartTimeCodeCopyTo(TLtcTimeCode *ptStartTimeCode);
	void StopTimeCodeCopyTo(TLtcTimeCode *ptStopTimeCode);

    static void staticCallbackFunction(void *p, const char *s);

private:
    struct LtcParamsMaskDisabledOutputs {
    	static constexpr auto DISPLAY = (1U << 0);
    	static constexpr auto MAX7219 = (1U << 1);
    	static constexpr auto MIDI = (1U << 2);
    	static constexpr auto ARTNET = (1U << 3);
    	static constexpr auto TCNET = (1U << 4);
    	static constexpr auto LTC = (1U << 5);
    	static constexpr auto NTP = (1U << 6);	// Not Used. TODO Subject for removal?
    	static constexpr auto RTPMIDI = (1U << 7);
    };

	void HandleDisabledOutput(const char *pLine, const char *pKeyword, unsigned nMaskDisabledOutputs);
	void SetBool(const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask);
	void SetValue(const bool bEvaluate, const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask);

	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tLtcParams.nSetList & nMask) == nMask;
	}
	bool isDisabledOutputMaskSet(uint8_t nMask) const {
		return (m_tLtcParams.nDisabledOutputs & nMask) == nMask;
	}

private:
    LtcParamsStore 	*m_pLTcParamsStore;
    struct TLtcParams m_tLtcParams;
};

#endif /* LTCPARAMS_H_ */
