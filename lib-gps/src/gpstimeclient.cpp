/**
 * @file gpstimeclient.cpp
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <sys/time.h>

#include "gpstimeclient.h"

#include "platform_gpio.h"
#include "hardware.h"

#include "debug.h"

enum Status {
	NOT_SET, WAITING_TIMEOUT, WAITING_PPS
};

static Status s_Status;

GPSTimeClient::GPSTimeClient(float fUtcOffset, GPSModule module): GPS(fUtcOffset, module) {
	m_nWaitPPSMillis = Hardware::Get()->Millis();
	s_Status = Status::NOT_SET;
}

void GPSTimeClient::Start() {
	GPS::Start();

	platform_gpio_init();
}

void GPSTimeClient::Run() {
	GPS::Run();

	if (s_Status == Status::WAITING_TIMEOUT) {
		const auto nMillis = Hardware::Get()->Millis();

		if (GPS::GetStatus() == GPSStatus::VALID) {
			m_nWaitPPSMillis = nMillis;

			s_Status = Status::WAITING_PPS;

			DEBUG_PUTS("(GPS::GetStatus() == GPSStatus::VALID)");
			return;
		}

		DEBUG_PUTS("No Fix");

		if (GPS::IsTimeUpdated()) {
			const auto nElapsedMillis = nMillis - GetTimeTimestampMillis();

			if (nElapsedMillis <= 1) {
				struct timeval tv;
				tv.tv_sec = GPS::GetLocalSeconds();
				tv.tv_usec = 0;
				settimeofday(&tv, nullptr);

				DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nElapsedMillis);
				return;
			}
		}

		s_Status = Status::NOT_SET;

		DEBUG_PUTS("No time update");
		return;
	}

	if (s_Status == Status::WAITING_PPS) {
		if (platform_is_pps()) {
			struct timeval tv;
			tv.tv_sec = GetLocalSeconds() + 1U;
			tv.tv_usec = 0;
			settimeofday(&tv, nullptr);

			s_Status = Status::WAITING_TIMEOUT;

			DEBUG_PUTS("PPS handled");
			return;
		}

		const auto nMillis = Hardware::Get()->Millis();

		if (__builtin_expect(((nMillis - m_nWaitPPSMillis) > (10 * 1000)), 0)) {
			// There is no PPS
			if (GPS::IsTimeUpdated()) {
				const auto nElapsedMillis = nMillis - GetTimeTimestampMillis();

				if (nElapsedMillis <= 1) {
					struct timeval tv;
					tv.tv_sec = GPS::GetLocalSeconds();
					tv.tv_usec = 0;
					settimeofday(&tv, nullptr);

					DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nElapsedMillis);
				}
			}

			s_Status = Status::WAITING_TIMEOUT;

			DEBUG_PUTS("((tv.tv_sec - nWaitPPS) >= 10 * 1000)");
			return;
		}

		return;
	}

	if ((s_Status == Status::NOT_SET) && GPS::IsTimeUpdated()) {
		m_nWaitPPSMillis = Hardware::Get()->Millis();

		s_Status = Status::WAITING_PPS;

		DEBUG_PUTS("((Status == Status::NOT_SET) && GPS::IsTimeUpdated())");
		return;
	}
}
