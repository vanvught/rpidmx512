/**
 * @file ltc.cpp
 *
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

#include <cstdint>
#include <cstring>
#include <time.h>
#include <cassert>

#include "ltc.h"

struct ltc::DisabledOutputs g_ltc_ptLtcDisabledOutputs;

namespace ltc {
static constexpr char aTypes[5][ltc::timecode::TYPE_MAX_LENGTH + 1] =
	{ "Film 24fps ", "EBU 25fps  ", "DF 29.97fps", "SMPTE 30fps", "----- -----" };


const char* get_type(ltc::Type type) {
	if (type > ltc::Type::UNKNOWN) {
		return aTypes[static_cast<uint32_t>(ltc::Type::UNKNOWN)];
	}

	return aTypes[static_cast<uint32_t>(type)];
}

ltc::Type get_type(uint8_t nFps) {
	switch (nFps) {
		case 24:
			return ltc::Type::FILM;
			break;
		case 25:
			return ltc::Type::EBU;
			break;
		case 29:
			return ltc::Type::DF;
			break;
		case 30:
			return ltc::Type::SMPTE;
			break;
		default:
			break;
	}

	return ltc::Type::UNKNOWN;
}

void init_timecode(char *pTimeCode) {
	assert(pTimeCode != nullptr);

	memset(pTimeCode, ' ', ltc::timecode::CODE_MAX_LENGTH);

	pTimeCode[ltc::timecode::index::COLON_1] = ':';
	pTimeCode[ltc::timecode::index::COLON_2] = ':';
	pTimeCode[ltc::timecode::index::COLON_3] = ':';
}

void init_systemtime(char *pSystemTime) {
	assert(pSystemTime != nullptr);

	memset(pSystemTime, ' ', ltc::timecode::SYSTIME_MAX_LENGTH);

	pSystemTime[ltc::systemtime::index::COLON_1] = ':';
	pSystemTime[ltc::systemtime::index::COLON_2] = ':';
}

static void itoa_base10(int arg, char *pBuffer) {
	auto *p = pBuffer;

	if (arg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = static_cast<char>('0' + (arg / 10));
	*p = static_cast<char>('0' + (arg % 10));
}

void itoa_base10(const struct ltc::TimeCode *ptLtcTimeCode, char *pTimeCode) {
	assert(ptLtcTimeCode != nullptr);
	assert(pTimeCode != nullptr);

	itoa_base10(ptLtcTimeCode->nHours, &pTimeCode[ltc::timecode::index::HOURS]);
	itoa_base10(ptLtcTimeCode->nMinutes, &pTimeCode[ltc::timecode::index::MINUTES]);
	itoa_base10(ptLtcTimeCode->nSeconds, &pTimeCode[ltc::timecode::index::SECONDS]);
	itoa_base10(ptLtcTimeCode->nFrames, &pTimeCode[ltc::timecode::index::FRAMES]);
}

void itoa_base10(const struct tm *pLocalTime, char *pSystemTime) {
	assert(pLocalTime != nullptr);
	assert(pSystemTime != nullptr);

	itoa_base10(pLocalTime->tm_hour, &pSystemTime[ltc::systemtime::index::HOURS]);
	itoa_base10(pLocalTime->tm_min, &pSystemTime[ltc::systemtime::index::MINUTES]);
	itoa_base10(pLocalTime->tm_sec, &pSystemTime[ltc::systemtime::index::SECONDS]);
}

#define DIGIT(x)	((x) - '0')
#define VALUE(x,y)	static_cast<uint8_t>(((x) * 10) + (y))

bool parse_timecode(const char *pTimeCode, uint8_t nFps, struct ltc::TimeCode *ptLtcTimeCode) {
	assert(pTimeCode != nullptr);
	assert(ptLtcTimeCode != nullptr);

	if ((pTimeCode[ltc::timecode::index::COLON_1] != ':') || (pTimeCode[ltc::timecode::index::COLON_2] != ':') || (pTimeCode[ltc::timecode::index::COLON_3] != ':')) {
		return false;
	}

	// Frames first

	auto nTenths = DIGIT(pTimeCode[ltc::timecode::index::FRAMES_TENS]);

	if ((nTenths < 0) || (nTenths > 3)) {
		return false;
	}

	auto nDigit = DIGIT(pTimeCode[ltc::timecode::index::FRAMES_UNITS]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	auto nValue = VALUE(nTenths, nDigit);

	if (nValue >= nFps) {
		return false;
	}

	ptLtcTimeCode->nFrames = nValue;

	// Seconds

	nTenths = DIGIT(pTimeCode[ltc::timecode::index::SECONDS_TENS]);

	if ((nTenths < 0) || (nTenths > 5)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[ltc::timecode::index::SECONDS_UNITS]);
	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	nValue = VALUE(nTenths, nDigit);

	if (nValue > 59) {
		return false;
	}

	ptLtcTimeCode->nSeconds = nValue;

	// Minutes

	nTenths = DIGIT(pTimeCode[ltc::timecode::index::MINUTES_TENS]);

	if ((nTenths < 0) || (nTenths > 5)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[ltc::timecode::index::MINUTES_UNITS]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	nValue = VALUE(nTenths, nDigit);

	if (nValue > 59) {
		return false;
	}

	ptLtcTimeCode->nMinutes = nValue;

	// Hours

	nTenths = DIGIT(pTimeCode[ltc::timecode::index::HOURS_TENS]);

	if ((nTenths < 0) || (nTenths > 2)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[ltc::timecode::index::HOURS_UNITS]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	nValue = VALUE(nTenths, nDigit);

	if (nValue > 23) {
		return false;
	}

	ptLtcTimeCode->nHours = nValue;

	return true;
}

bool parse_timecode_rate(const char *pTimeCodeRate, uint8_t &nFPS, ltc::Type &tType) {
	assert(pTimeCodeRate != nullptr);

	const auto nTenths = DIGIT(pTimeCodeRate[0]);

	if ((nTenths < 0) || (nTenths > 3)) {
		return false;
	}

	const auto nDigit = DIGIT(pTimeCodeRate[1]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	const auto nValue = VALUE(nTenths, nDigit);

	switch (nValue) {
	case 24:
		tType = ltc::Type::FILM;
		break;
	case 25:
		tType = ltc::Type::EBU;
		break;
	case 29:
		tType = ltc::Type::DF;
		break;
	case 30:
		tType = ltc::Type::SMPTE;
		break;
	default:
		return false;
		break;
	}

	nFPS = nValue;

	return true;
}
}  // namespace ltc
