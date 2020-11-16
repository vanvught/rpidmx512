/**
 * @file ltc.h
 *
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

#ifndef LTC_H_
#define LTC_H_

#include <stdint.h>
#include <time.h>

namespace ltc {
enum source : uint8_t {
	LTC, ARTNET, MIDI, TCNET, INTERNAL, APPLEMIDI, SYSTIME, UNDEFINED
};
enum type : uint8_t {
	FILM, EBU, DF, SMPTE, UNKNOWN, INVALID = 255
};
namespace led_frequency {
static constexpr uint32_t NO_DATA = 1;
static constexpr uint32_t DATA = 3;
}
}  // namespace ltc

static constexpr uint32_t TC_CODE_MAX_LENGTH = 11;
static constexpr uint32_t TC_TYPE_MAX_LENGTH = 11;
static constexpr uint32_t TC_RATE_MAX_LENGTH = 2;
static constexpr uint32_t TC_SYSTIME_MAX_LENGTH = TC_CODE_MAX_LENGTH;

struct TLtcTimeCode {
	uint8_t nFrames;		///< Frames time. 0 – 29 depending on mode.
	uint8_t nSeconds;		///< Seconds. 0 - 59.
	uint8_t nMinutes;		///< Minutes. 0 - 59.
	uint8_t nHours;			///< Hours. 0 - 59.
	uint8_t nType;			///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
}__attribute__((packed));

struct TLtcDisabledOutputs {
	bool bOled;
	bool bMax7219;
	bool bMidi;
	bool bArtNet;
	bool bLtc;
	bool bNtp;
	bool bRtpMidi;
	bool bWS28xx;
	bool bRgbPanel;
};

enum TLtcTimeCodeIndex {
	LTC_TC_INDEX_HOURS = 0,
	LTC_TC_INDEX_HOURS_TENS = 0,
	LTC_TC_INDEX_HOURS_UNITS = 1,
	LTC_TC_INDEX_COLON_1 = 2,
	LTC_TC_INDEX_MINUTES = 3,
	LTC_TC_INDEX_MINUTES_TENS = 3,
	LTC_TC_INDEX_MINUTES_UNITS = 4,
	LTC_TC_INDEX_COLON_2 = 5,
	LTC_TC_INDEX_SECONDS = 6,
	LTC_TC_INDEX_SECONDS_TENS = 6,
	LTC_TC_INDEX_SECONDS_UNITS = 7,
	LTC_TC_INDEX_COLON_3 = 8,
	LTC_TC_INDEX_FRAMES = 9,
	LTC_TC_INDEX_FRAMES_TENS = 9,
	LTC_TC_INDEX_FRAMES_UNITS = 10
};

enum TLtcSystemTimeIndex {
	LTC_ST_INDEX_HOURS = 0,
	LTC_ST_INDEX_HOURS_TENS = 0,
	LTC_ST_INDEX_HOURS_UNITS = 1,
	LTC_ST_INDEX_COLON_1 = 2,
	LTC_ST_INDEX_MINUTES = 3,
	LTC_ST_INDEX_MINUTES_TENS = 3,
	LTC_ST_INDEX_MINUTES_UNITS = 4,
	LTC_ST_INDEX_COLON_2 = 5,
	LTC_ST_INDEX_SECONDS = 6,
	LTC_ST_INDEX_SECONDS_TENS = 6,
	LTC_ST_INDEX_SECONDS_UNITS = 7
};

class Ltc {
public:
	static const char *GetType(ltc::type tTimeCodeType);
	static ltc::type GetType(uint8_t nFps);

	static void InitTimeCode(char *pTimeCode);
	static void InitSystemTime(char *pSystemTime);

	static void ItoaBase10(const struct TLtcTimeCode *ptLtcTimeCode, char *pTimeCode);
	static void ItoaBase10(const struct tm *ptLocalTime, char *pSystemTime);

	static bool ParseTimeCode(const char *pTimeCode, uint8_t nFps, struct TLtcTimeCode *ptLtcTimeCode);
	static bool ParseTimeCodeRate(const char *pTimeCodeRate, uint8_t &nFPS, ltc::type &tType);
};

#endif /* LTC_H_ */
