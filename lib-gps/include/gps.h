/**
 * @file gps.h
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

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>
#include <time.h>

#include "gpsconst.h"

enum class GPSStatus {
	IDLE,
	WARNING,
	VALID,
	UNDEFINED
};

class GPSDisplay {
public:
	virtual ~GPSDisplay() {
	}

	virtual void ShowSGpstatus(GPSStatus nStatus)=0;
};

class GPS {
public:
	GPS(float fUtcOffset = 0.0, GPSModule module = GPSModule::UNDEFINED);

	void Start();
	void Run();

	void Print();

	bool IsDateUpdated() const {
		return m_IsDateUpdated;
	}

	bool IsTimeUpdated() const {
		return m_IsTimeUpdated;
	}

	uint32_t GetTimeTimestampMillis() const {
		return m_nTimeTimestampMillis;
	}

	const struct tm *GetDateTime() {
		m_IsTimeUpdated = m_IsDateUpdated = false;
		return &m_Tm;
	}

	time_t GetLocalSeconds() {
		m_IsTimeUpdated = m_IsDateUpdated = false;
		return mktime(&m_Tm) + m_nUtcOffset;
	}

	GPSStatus GetStatus() const {
		return m_tStatusCurrent;
	}

	void SetGPSDisplay(GPSDisplay *pGPSDisplay) {
		 m_pGPSDisplay = pGPSDisplay;
	}

	static GPS *Get() {
		return s_pThis;
	}

	static GPSModule GetModule(const char *pName);
	static const char *GetModuleName(GPSModule tModule);

private:
	void UartInit();
	void UartSetBaud(uint32_t nBaud = 9600);
	const char *UartGetSentence();	// Return a valid sentence that is starting with $ and a valid checksum
	void UartSend(const char *pSentence);
	//
	uint32_t GetTag(const char *p);
	int32_t ParseDecimal(const char *p, uint32_t &nLength);
	void SetTime(int32_t nTime);
	void SetDate(int32_t nDate);

	void DumpSentence(const char *pSentence);

private:
	int32_t m_nUtcOffset;
	GPSModule m_tModule;

	uint32_t m_nBaud{9600};
	char *m_pSentence{nullptr};

	struct tm m_Tm;

	bool m_IsTimeUpdated{false};
	bool m_IsDateUpdated{false};

	uint32_t m_nTimeTimestampMillis;
	uint32_t m_nDateTimestampMillis;

	GPSStatus m_tStatusCurrent{GPSStatus::UNDEFINED};
	GPSStatus m_tStatusPrevious{GPSStatus::UNDEFINED};
	GPSDisplay *m_pGPSDisplay{nullptr};

	static GPS *s_pThis;
};

#endif /* GPS_H_ */
