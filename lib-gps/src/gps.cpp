/**
 * @file gps.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

/**
 * https://gpsd.gitlab.io/gpsd/NMEA.html
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <cassert>

#include "gps.h"

#include "utc.h"

#include "hardware.h"

#include "debug.h"

// Maximum sentence length, including the $ and <CR><LF> is 82 bytes.

namespace gps {
namespace nmea{
namespace length {
static constexpr uint32_t TALKER_ID = 2;
static constexpr uint32_t TAG = 3;
}  // namespace length
enum {
	RMC,
	GGA,
	ZDA,
	UNDEFINED
};
}  // namespace nmea
}  // namespace gps

using namespace gps;

constexpr char aTag[static_cast<int>(nmea::UNDEFINED)][nmea::length::TAG] =
	{ { 'R', 'M', 'C' },	// Recommended Minimum Navigation Information
	  { 'G', 'G', 'A' }, 	// Global Positioning System Fix Data
	  { 'Z', 'D', 'A' }		// Time & Date - UTC, day, month, year and local time zone
	};

GPS *GPS::s_pThis = nullptr;

GPS::GPS(float fUtcOffset, GPSModule module): m_nUtcOffset(Utc::Validate(fUtcOffset)), m_tModule(module) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(&m_Tm, 0, sizeof(struct tm));

	m_Tm.tm_mday = _TIME_STAMP_DAY_;			// The day of the month, in the range 1 to 31.
	m_Tm.tm_mon = _TIME_STAMP_MONTH_ - 1;		// The number of months since January, in the range 0 to 11.
	m_Tm.tm_year = _TIME_STAMP_YEAR_ - 1900;	// The number of years since 1900.

	DEBUG_EXIT
}

uint32_t GPS::GetTag(const char *pTag) {
	for (uint32_t i = 0; i < nmea::UNDEFINED; i++) {
		if (memcmp(aTag[i], pTag, nmea::length::TAG) == 0) {
			return i;
		}
	}

	return static_cast<uint32_t>(nmea::UNDEFINED);
}

int32_t GPS::ParseDecimal(const char *p, uint32_t &nLength) {
	const bool bIsNegative = (*p == '-');

	nLength = bIsNegative ? 1 : 0;
	int32_t nValue = 0;

	while ((p[nLength] != '.') && (p[nLength] != ',')) {
		nValue = nValue * 10 + p[nLength] - '0';
		nLength++;
	}

	if (p[nLength] == '.') {
		nLength++;
		nValue = nValue * 10 + p[nLength] - '0';
		nLength++;
		nValue = nValue * 10 + p[nLength] - '0';
		nLength++;
	}

	return bIsNegative ? -nValue : nValue;
}

void GPS::SetTime(int32_t nTime) {
	if (nTime != 0) {
		m_nTimeTimestampMillis = Hardware::Get()->Millis();
		m_IsTimeUpdated = true;

		nTime /= 100;
		m_Tm.tm_sec = nTime % 100;
		nTime /= 100;
		m_Tm.tm_min = nTime % 100;
		m_Tm.tm_hour = nTime / 100;

#ifndef NDEBUG
//	printf("%.2d:%.2d:%.2d\n", m_Tm.tm_hour, m_Tm.tm_min, m_Tm.tm_sec);
#endif
	}
}

void GPS::SetDate(int32_t nDate) {
	if (nDate != 0) {
		m_nDateTimestampMillis = Hardware::Get()->Millis();
		m_IsDateUpdated = true;

		m_Tm.tm_year = 100 + (nDate % 100);	// The number of years since 1900.
		nDate /= 100;
		m_Tm.tm_mon = (nDate % 100) - 1;	// The number of months since January, in the range 0 to 11.
		m_Tm.tm_mday = nDate / 100;			// The day of the month, in the range 1 to 31.

#ifndef NDEBUG
//	printf("%.2d/%.2d/%.2d\n", m_Tm.tm_mday, 1 + m_Tm.tm_mon, 1900 + m_Tm.tm_year);
#endif
	}
}

void GPS::Start() {
	DEBUG_ENTRY

	UartInit();

	if (m_tModule < GPSModule::UNDEFINED) {
		UartSend(GPSConst::BAUD_115200[static_cast<unsigned>(m_tModule)]);
		UartSetBaud(115200);

		const uint32_t nMillis = Hardware::Get()->Millis();

		while ((Hardware::Get()->Millis() - nMillis) < 1000) {
			m_pSentence = const_cast<char *>(UartGetSentence());

			if (m_pSentence != nullptr) {
				DumpSentence(m_pSentence);
#ifndef NDEBUG
				printf("[%u]\n", Hardware::Get()->Millis() - nMillis);
#endif
				break;
			}
		}

		if (m_pSentence == nullptr) {
			UartSetBaud(9600);
		}
	}

	m_tStatusCurrent = GPSStatus::IDLE;

	if (m_pGPSDisplay != nullptr) {
		m_pGPSDisplay->ShowSGpstatus(GPSStatus::IDLE);
	}

	DEBUG_EXIT
}

void GPS::Run() {
	if (__builtin_expect(((m_pSentence = const_cast<char *>(UartGetSentence())) == nullptr), 1)) {
		return;
	}

	//DumpSentence(m_pSentence);

	uint32_t nTag;

	if (__builtin_expect(((nTag = GetTag(&m_pSentence[1 + nmea::length::TALKER_ID])) == nmea::UNDEFINED), 0)) {
		return;
	}

	uint32_t nOffset = 1 + nmea::length::TALKER_ID + nmea::length::TAG + 1; // $ and ,
	uint32_t nFieldIndex = 1;

	do {
		uint32_t nLenght = 0;
		switch (nTag | nFieldIndex << 8) {
		case nmea::RMC | (1 << 8):	// UTC Time of position, hhmmss.ss
		case nmea::GGA | (1 << 8):
		case nmea::ZDA | (1 << 8):
			SetTime(ParseDecimal(&m_pSentence[nOffset], nLenght));
			nOffset += nLenght;
			break;
		case nmea::RMC | (2 << 8):	// Status, A = Valid, V = Warning
			m_tStatusCurrent = (m_pSentence[nOffset] == 'A' ? GPSStatus::VALID : GPSStatus::WARNING);
//			printf("(%c) : %d\n", m_pSentence[nOffset], nOffset	);
			nOffset += 1;
			break;
		case nmea::RMC | (9 << 8):	// Date, ddmmyy
			SetDate(ParseDecimal(&m_pSentence[nOffset], nLenght));
			nOffset += nLenght;
			break;
		default:
			break;
		}

		nFieldIndex++;

		while ((m_pSentence[nOffset] != '*') && (m_pSentence[nOffset] != ',')) {
			nOffset++;
		}

	} while (m_pSentence[nOffset++] != '*');

	if (m_tStatusCurrent != m_tStatusPrevious) {
		m_tStatusPrevious = m_tStatusCurrent;

		if (m_pGPSDisplay != nullptr) {
			m_pGPSDisplay->ShowSGpstatus(m_tStatusCurrent);
		}
	}
}
