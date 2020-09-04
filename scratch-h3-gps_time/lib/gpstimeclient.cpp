/**
 * @file gpstimeclient.cpp
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

#include <stdint.h>
#include <sys/time.h>

#include "gpstimeclient.h"

#include "hardware.h"

#include "debug.h"

enum Status {
	NOT_SET,
	WAITING_TIMEOUT,
	WAITING_PPS
};

static Status Status;

GPSTimeClient::GPSTimeClient(float fUtcOffset): GPS(fUtcOffset) {
	m_nLastUpdateMillis = m_nWaitPPSMillis = Hardware::Get()->Millis();
	Status = Status::NOT_SET;
}

void GPSTimeClient::Run() {
	GPS::Run();

	if (Status == Status::WAITING_TIMEOUT) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		if (__builtin_expect(((nMillis - m_nLastUpdateMillis) < 150 * 1000), 1)) {
			return;
		}

		if (GPS::GetStatus() == GPSStatus::VALID) {
			m_nWaitPPSMillis = nMillis;
			Status = Status::WAITING_PPS;

			DEBUG_PUTS("(GPS::GetStatus() == GPSStatus::VALID)");
			return;
		}

		DEBUG_PUTS("No Fix");

		if (GPS::IsTimeUpdated()) {
			const uint32_t nElapsedMillis = nMillis - GetTimeTimestampMillis();

			if (nElapsedMillis < 1000) {

				struct timeval tv;
				tv.tv_sec = GPS::GetLocalSeconds();
				tv.tv_usec = 0;
				settimeofday(&tv, nullptr);

				m_nLastUpdateMillis = nMillis;
				Status = Status::WAITING_TIMEOUT;

				DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nElapsedMillis);

				return;
			}
		}

		Status = Status::NOT_SET;

		DEBUG_PUTS("No time update");
		return;
	}

	if (Status == Status::WAITING_PPS) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		// TODO if PPS interrupt then set time and Status = Status::WAITING_TIMEOUT and return

		if (__builtin_expect(((nMillis - m_nWaitPPSMillis) > 1 * 1000), 0)) {
			// There is no PPS
			if (GPS::IsTimeUpdated()) {

				struct timeval tv;
				tv.tv_sec = GPS::GetLocalSeconds();
				tv.tv_usec = 0;
				settimeofday(&tv, nullptr);

				DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nMillis - GetTimeTimestampMillis());
			}

			m_nLastUpdateMillis = nMillis;
			Status = Status::WAITING_TIMEOUT;

			DEBUG_PUTS("((tv.tv_sec - nWaitPPS) >= 2)");
			return;
		}

		return;
	}

	if ((Status == Status::NOT_SET) && GPS::IsTimeUpdated()) {
		m_nWaitPPSMillis = Hardware::Get()->Millis();
		Status = Status::WAITING_PPS;

		DEBUG_PUTS("((Status == Status::NOT_SET) && GPS::IsTimeUpdated())");
		return;
	}
}
