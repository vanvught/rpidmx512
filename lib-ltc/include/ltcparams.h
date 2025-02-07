/**
 * @file ltcparams.h
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

#ifndef LTCPARAMS_H_
#define LTCPARAMS_H_

#include <cstdint>

#include "ltcdisplaymax7219.h"
#include "ltc.h"

#include "configstore.h"

#include "hardware.h"

namespace ltcparams {
struct Params {
	uint32_t nSetList;			///< 4	4
	uint8_t nSource;			///< 1	5
	uint8_t nFiller1;			///< 1	6
	uint8_t nVolume;			///< 1	7
	uint8_t nDisabledOutputs;	///< 1	8
	int8_t  nUtcOffsetHours;	///< 1	9
	uint8_t nUtcOffsetMinutes;	///< 1	10
	uint8_t nYear;				///< 1	11
	uint8_t nMonth;				///< 1	12
	uint8_t nDay;				///< 1	13
	uint8_t nFiller4;			///< 1	14
	uint8_t nFiller5;			///< 1	15
	uint8_t nFps;				///< 1	16
	uint8_t nStartFrame;		///< 1	17
	uint8_t nStartSecond;		///< 1	18
	uint8_t nStartMinute;		///< 1	19
	uint8_t nStartHour;			///< 1	20
	uint8_t nStopFrame;			///< 1	21
	uint8_t nStopSecond;		///< 1	22
	uint8_t nStopMinute;		///< 1	23
	uint8_t nStopHour;			///< 1	24
	uint8_t nFiller6;			///< 1	25
	uint16_t nOscPort;			///< 2	27
	uint8_t nRgbLedType;		///< 1	28
	uint8_t nAltFunction;		///< 1	29
	uint8_t nSkipSeconds;		///< 1	30
	uint8_t nSkipFree;			///< 1	31
	uint32_t nTimeCodeIp;		///< 4  35
}__attribute__((packed));

static_assert(sizeof(struct ltcparams::Params) <= 64, "struct ltcparams::Params is too large");

struct Mask {
	static constexpr auto SOURCE = (1U << 0);
	static constexpr auto AUTO_START = (1U << 1);
	static constexpr auto VOLUME = (1U << 2);
	static constexpr auto DISABLED_OUTPUTS = (1U << 3);
	static constexpr auto SHOW_SYSTIME = (1U << 4);
	static constexpr auto DISABLE_TIMESYNC = (1U << 5);
	static constexpr auto YEAR = (1U << 6);
	static constexpr auto MONTH = (1U << 7);
	static constexpr auto DAY = (1U << 8);
	static constexpr auto NTP_ENABLE = (1U << 9);
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
	static constexpr auto OSC_ENABLE = (1U << 20);
	static constexpr auto OSC_PORT = (1U << 21);
	static constexpr auto RGBLEDTYPE = (1U << 22);
	static constexpr auto ALT_FUNCTION = (1U << 23);
	static constexpr auto SKIP_SECONDS = (1U << 24);
	static constexpr auto SKIP_FREE = (1U << 25);
	static constexpr auto TIMECODE_IP = (1U << 26);
	static constexpr auto GPS_START = (1U << 27);
	static constexpr auto IGNORE_START = (1U << 28);
	static constexpr auto IGNORE_STOP = (1U << 29);
};

struct RgbLedType {
	static constexpr auto WS28XX = (1U << 0);
	static constexpr auto RGBPANEL = (1U << 1);
};

namespace store {
inline void update(const struct ltcparams::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::LTC, pParams, sizeof(struct ltcparams::Params));
}

inline void copy(struct ltcparams::Params *pParams) {
	ConfigStore::Get()->Copy(configstore::Store::LTC, pParams, sizeof(struct ltcparams::Params));
}
}  // namespace store
}  // namespace ltcparams

class LtcParams {
public:
	LtcParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const ltcparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(struct ltc::TimeCode *ptStartTimeCode, struct ltc::TimeCode *ptStopTimeCode);

	ltc::Source GetSource() const {
		return static_cast<ltc::Source>(m_Params.nSource);
	}

	const char *GetSourceType(ltc::Source tSource);
	ltc::Source GetSourceType(const char *pType);

	uint8_t GetVolume() const {
		return m_Params.nVolume;
	}

	bool IsAutoStart() const {
		return isMaskSet(ltcparams::Mask::AUTO_START);
	}

	bool IsGpsStart() const {
		return isMaskSet(ltcparams::Mask::GPS_START);
	}

	int32_t GetUtcOffset() {
		int32_t nUtcOffset;
		if (hal::utc_validate(m_Params.nUtcOffsetHours, m_Params.nUtcOffsetMinutes, nUtcOffset)) {
			return nUtcOffset;
		}
		return 0;
	}

	bool IsShowSysTime() const {
		return isMaskSet(ltcparams::Mask::SHOW_SYSTIME);
	}

	bool IsTimeSyncDisabled() const {
		return isMaskSet(ltcparams::Mask::DISABLE_TIMESYNC);
	}

	bool IsNtpEnabled() const {
		return isMaskSet(ltcparams::Mask::NTP_ENABLE);
	}

	bool IsOscEnabled() const {
		return isMaskSet(ltcparams::Mask::OSC_ENABLE);
	}

	bool IsSetDate() const {
		return isMaskSet(ltcparams::Mask::SET_DATE);
	}

	bool IsIgnoreStart() const {
		return isMaskSet(ltcparams::Mask::IGNORE_START);
	}

	bool IsIgnoreStop() const {
		return isMaskSet(ltcparams::Mask::IGNORE_STOP);
	}

	uint16_t GetOscPort(bool &bIsSet) {
		bIsSet = isMaskSet(ltcparams::Mask::OSC_PORT);
		return m_Params.nOscPort;
	}

	uint8_t GetYear() const {
		return m_Params.nYear;
	}

	uint8_t GetMonth() const {
		return m_Params.nMonth;
	}

	uint8_t GetDay() const {
		return m_Params.nDay;
	}

	uint8_t GetFps() const {
		return m_Params.nFps;
	}

	bool IsWS28xxEnabled() const {
		return (m_Params.nRgbLedType == ltcparams::RgbLedType::WS28XX);
	}

	bool IsRgbPanelEnabled() const {
		return (m_Params.nRgbLedType == ltcparams::RgbLedType::RGBPANEL);
	}

	bool IsAltFunction() const {
		return (m_Params.nAltFunction == 1);
	}

	uint8_t GetSkipSeconds() const {
		return m_Params.nSkipSeconds;
	}

	uint8_t GetSkipFree() const {
		return (m_Params.nSkipFree == 1);
	}

	uint32_t GetTimecodeIp() const {
		return m_Params.nTimeCodeIp;
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void HandleDisabledOutput(const char *pLine, const char *pKeyword, const ltc::Destination::Output);
	void SetBool(const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask);
	void SetValue(const bool bEvaluate, const uint8_t nValue, uint8_t& nProperty, const uint32_t nMask);

	void Dump();
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}
	bool isDisabledOutputMaskSet(const ltc::Destination::Output nOutput) const {
		 return (m_Params.nDisabledOutputs & static_cast<uint32_t>(nOutput)) == static_cast<uint32_t>(nOutput);
	}

private:
    ltcparams::Params m_Params;
};

#endif /* LTCPARAMS_H_ */
