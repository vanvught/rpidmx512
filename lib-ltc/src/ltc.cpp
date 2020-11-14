/**
 * @file ltc.cpp
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

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <cassert>

#include "ltc.h"

using namespace ltc;

static constexpr char aTypes[5][TC_TYPE_MAX_LENGTH + 1] =
	{ "Film 24fps ", "EBU 25fps  ", "DF 29.97fps", "SMPTE 30fps", "----- -----" };


const char* Ltc::GetType(type tTimeCodeType) {
	if (tTimeCodeType > type::UNKNOWN) {
		return aTypes[type::UNKNOWN];
	}

	return aTypes[tTimeCodeType];
}

type Ltc::GetType(uint8_t nFps) {
	switch (nFps) {
		case 24:
			return type::FILM;
			break;
		case 25:
			return type::EBU;
			break;
		case 29:
			return type::DF;
			break;
		case 30:
			return type::SMPTE;
			break;
		default:
			break;
	}

	return type::UNKNOWN;
}

static void itoa_base10(int arg, char *pBuffer) {
	auto *p = pBuffer;

	if (arg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = ('0' + (arg / 10));
	*p = ('0' + (arg % 10));
}

void Ltc::ItoaBase10(const struct TLtcTimeCode *ptLtcTimeCode, char *pTimeCode) {
	assert(ptLtcTimeCode != nullptr);
	assert(pTimeCode != nullptr);

	itoa_base10(ptLtcTimeCode->nHours, &pTimeCode[LTC_TC_INDEX_HOURS]);
	itoa_base10(ptLtcTimeCode->nMinutes, &pTimeCode[LTC_TC_INDEX_MINUTES]);
	itoa_base10(ptLtcTimeCode->nSeconds, &pTimeCode[LTC_TC_INDEX_SECONDS]);
	itoa_base10(ptLtcTimeCode->nFrames, &pTimeCode[LTC_TC_INDEX_FRAMES]);
}

void Ltc::ItoaBase10(const struct tm *pLocalTime, char *pSystemTime) {
	assert(pLocalTime != nullptr);
	assert(pSystemTime != nullptr);

	itoa_base10(pLocalTime->tm_hour, &pSystemTime[LTC_ST_INDEX_HOURS]);
	itoa_base10(pLocalTime->tm_min, &pSystemTime[LTC_ST_INDEX_MINUTES]);
	itoa_base10(pLocalTime->tm_sec, &pSystemTime[LTC_ST_INDEX_SECONDS]);
}

#define DIGIT(x)	((x) - '0')
#define VALUE(x,y)	(((x) * 10) + (y))

bool Ltc::ParseTimeCode(const char *pTimeCode, uint8_t nFps, struct TLtcTimeCode *ptLtcTimeCode) {
	assert(pTimeCode != nullptr);
	assert(ptLtcTimeCode != nullptr);

	if ((pTimeCode[LTC_TC_INDEX_COLON_1] != ':') || (pTimeCode[LTC_TC_INDEX_COLON_2] != ':') || (pTimeCode[LTC_TC_INDEX_COLON_3] != ':')) {
		return false;
	}

	// Frames first

	auto nTenths = DIGIT(pTimeCode[LTC_TC_INDEX_FRAMES_TENS]);

	if ((nTenths < 0) || (nTenths > 3)) {
		return false;
	}

	auto nDigit = DIGIT(pTimeCode[LTC_TC_INDEX_FRAMES_UNITS]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	auto nValue = VALUE(nTenths, nDigit);

	if (nValue >= nFps) {
		return false;
	}

	ptLtcTimeCode->nFrames = nValue;

	// Seconds

	nTenths = DIGIT(pTimeCode[LTC_TC_INDEX_SECONDS_TENS]);

	if ((nTenths < 0) || (nTenths > 5)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[LTC_TC_INDEX_SECONDS_UNITS]);
	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	nValue = VALUE(nTenths, nDigit);

	if (nValue > 59) {
		return false;
	}

	ptLtcTimeCode->nSeconds = nValue;

	// Minutes

	nTenths = DIGIT(pTimeCode[LTC_TC_INDEX_MINUTES_TENS]);

	if ((nTenths < 0) || (nTenths > 5)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[LTC_TC_INDEX_MINUTES_UNITS]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	nValue = VALUE(nTenths, nDigit);

	if (nValue > 59) {
		return false;
	}

	ptLtcTimeCode->nMinutes = nValue;

	// Hours

	nTenths = DIGIT(pTimeCode[LTC_TC_INDEX_HOURS_TENS]);

	if ((nTenths < 0) || (nTenths > 2)) {
		return false;
	}

	nDigit = DIGIT(pTimeCode[LTC_TC_INDEX_HOURS_UNITS]);

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

void Ltc::InitTimeCode(char *pTimeCode) {
	assert(pTimeCode != nullptr);

	memset(pTimeCode, ' ', TC_CODE_MAX_LENGTH);

	pTimeCode[LTC_TC_INDEX_COLON_1] = ':';
	pTimeCode[LTC_TC_INDEX_COLON_2] = ':';
	pTimeCode[LTC_TC_INDEX_COLON_3] = ':';
}

void Ltc::InitSystemTime(char *pSystemTime) {
	assert(pSystemTime != nullptr);

	memset(pSystemTime, ' ', TC_SYSTIME_MAX_LENGTH);

	pSystemTime[LTC_ST_INDEX_COLON_1] = ':';
	pSystemTime[LTC_ST_INDEX_COLON_2] = ':';
}

bool Ltc::ParseTimeCodeRate(const char *pTimeCodeRate, uint8_t &nFPS, type &tType) {
	assert(pTimeCodeRate != nullptr);

	auto nTenths = DIGIT(pTimeCodeRate[0]);

	if ((nTenths < 0) || (nTenths > 3)) {
		return false;
	}

	auto nDigit = DIGIT(pTimeCodeRate[1]);

	if ((nDigit < 0) || (nDigit > 9)) {
		return false;
	}

	auto nValue = VALUE(nTenths, nDigit);

	switch (nValue) {
	case 24:
		tType = type::FILM;
		break;
	case 25:
		tType = type::EBU;
		break;
	case 29:
		tType = type::DF;
		break;
	case 30:
		tType = type::SMPTE;
		break;
	default:
		return false;
		break;
	}

	nFPS = nValue;

	return true;
}

inline uint32_t _tc_round_up(double x)
{
	return (static_cast<uint32_t>((x + 0.5)));
}

inline uint32_t _tc_round_down(double x)
{
	return (static_cast<uint32_t>((x - 0.5)));
}

inline uint32_t _tc_round(double x)
{
	if (x < 0.0)
		return (static_cast<uint32_t>((x - 0.5)));
	else
		return (static_cast<uint32_t>((x + 0.5)));
}

// convert current hr,mn,sc,fr to milliseconds and return.
uint32_t Ltc::TCtoMS(const struct TLtcTimeCode *ptLtcTimeCode)
{
	bool drop = false;
	double div = 0.0;

	switch (ptLtcTimeCode->nType)
	{
	case type::FILM:
		div = 24.0;
		break;
	case type::EBU:
		div = 25.0;
		break;
	case type::DF:
		div = 30.0;
		drop = true;
		break;
	case type::SMPTE:
		div = 30.0;
		break;
	default:
		return __UINT32_MAX__; // 4294967295
		break;
	}

	double fps_d = div;
	double fps_i = _tc_round_up(div);
	double fpf_d = (1000 / fps_d);
	double sample = 0;
	if (drop)
	{
		double tMin = 60 * ptLtcTimeCode->nHours + ptLtcTimeCode->nMinutes;
		double frameNumber = fps_i * 3600 * ptLtcTimeCode->nHours + fps_i * 60 * ptLtcTimeCode->nMinutes + fps_i * ptLtcTimeCode->nSeconds + ptLtcTimeCode->nFrames - 2 * (tMin - tMin / 10);
		sample = frameNumber * 1000 / fps_d;
	}
	else
	{
		sample = _tc_round((((ptLtcTimeCode->nHours * 60 * 60) + (ptLtcTimeCode->nMinutes * 60) + ptLtcTimeCode->nSeconds) * (fps_i * fpf_d)) + (ptLtcTimeCode->nFrames * fpf_d));
	}
	return static_cast<uint32_t>(sample);
}

// take a value in ms and set ptLtcTimeCode h,m,s,f based on TC type already in ptLtcTimeCode->nType
// return non-zero on fail
uint32_t Ltc::MStoTC(const uint32_t ms, struct TLtcTimeCode *ptLtcTimeCode) {
	bool drop = false;
	double div = 0.0;

	switch (ptLtcTimeCode->nType) {
	case type::FILM:
		div = 24.0;
		break;
	case type::EBU:
		div = 25.0;
		break;
	case type::DF:
		div = 30.0;
		drop = true;
		break;
	case type::SMPTE:
		div = 30.0;
		break;
	default:
		return __UINT32_MAX__; // 4294967295
		break;
	}

	uint32_t fps_i = _tc_round_up(div);
	if (fps_i == 0)	{
		return 1; // invalid TC rate
	}

	if (ms == 0) {
		ptLtcTimeCode->nHours = 0;
		ptLtcTimeCode->nMinutes = 0;
		ptLtcTimeCode->nSeconds = 0;
		ptLtcTimeCode->nFrames = 0;
		return 1;
	}

	if (drop) {
		uint32_t frameNumber = _tc_round_down(ms * div / 1000);
		uint32_t D = frameNumber / 17982; // there are 17982 frames in 10 min @ 29.97df
		uint32_t M = frameNumber % 17982;

		frameNumber += (18 * (D + (2 * ((M - 2) / 1798))));
		ptLtcTimeCode->nFrames = frameNumber % 30;
		ptLtcTimeCode->nSeconds = (frameNumber / 30) % 60;
		ptLtcTimeCode->nMinutes = ((frameNumber / 30) / 60) % 60;
		ptLtcTimeCode->nHours = (((frameNumber / 30) / 60) / 60);
	}
	else {
		static double timecode_frames_left_exact;
		static uint32_t timecode_frames_left;
		double fpf_d = 1000 / div;
		double frames_per_hour = 3600 * fps_i * fpf_d;
		double sample_d = (ms % static_cast<uint32_t>(frames_per_hour));

		timecode_frames_left_exact = sample_d / fpf_d;
		timecode_frames_left = _tc_round(timecode_frames_left_exact);
		ptLtcTimeCode->nHours = _tc_round(ms / frames_per_hour);
		ptLtcTimeCode->nMinutes = timecode_frames_left / (fps_i * 60);
		if (ptLtcTimeCode->nMinutes == 60) {
			ptLtcTimeCode->nMinutes = 0;
		}
		timecode_frames_left = timecode_frames_left % (fps_i * 60);
		ptLtcTimeCode->nSeconds = static_cast<uint8_t>(timecode_frames_left / fps_i);
		ptLtcTimeCode->nFrames = static_cast<uint8_t>(timecode_frames_left % fps_i);
	}

	if (ptLtcTimeCode->nHours > 23) { // don't allow >= 24hrs
		ptLtcTimeCode->nHours = 0;
		ptLtcTimeCode->nMinutes = 0;
		ptLtcTimeCode->nSeconds = 0;
		ptLtcTimeCode->nFrames = 0;
	}

	return 0;
}
