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

#include "h3_gpio.h"
#include "h3_board.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "debug.h"

enum Status {
	NOT_SET,
	WAITING_TIMEOUT,
	WAITING_PPS
};

static volatile Status s_Status;
static volatile uint32_t s_nLastUpdateMillis;

static GPSTimeClient *s_pGPS;

extern "C" {
time_t WrapperGetLocalSeconds(GPSTimeClient *pGPS) {
	return pGPS->GetLocalSeconds();
}
}

static void __attribute__((interrupt("FIQ"))) pps_handler() {
	dmb();
	H3_PIO_PA_INT->STA = static_cast<uint32_t>(~0x0);

	struct timeval tv;
	tv.tv_sec = WrapperGetLocalSeconds(s_pGPS);
	tv.tv_usec = 0;
	settimeofday(&tv, nullptr);

	s_nLastUpdateMillis = H3_TIMER->AVS_CNT0; // --> Hardware::Get()->Millis();
	s_Status = Status::WAITING_TIMEOUT;

	dmb();
}

GPSTimeClient::GPSTimeClient(float fUtcOffset, GPSModule module): GPS(fUtcOffset, module) {
	s_nLastUpdateMillis = m_nWaitPPSMillis = Hardware::Get()->Millis();
	s_Status = Status::NOT_SET;
	s_pGPS = this;
}

void GPSTimeClient::Start() {
	GPS::Start();

	h3_gpio_fsel(GPIO_EXT_18, GPIO_FSEL_EINT);

	arm_install_handler(reinterpret_cast<unsigned>(pps_handler), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG2 = (GPIO_INT_CFG_POS_EDGE << 8);
	H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_18);
	H3_PIO_PA_INT->STA = (1 << GPIO_EXT_18);
	H3_PIO_PA_INT->DEB = 1;
}

void GPSTimeClient::Run() {
	GPS::Run();

	if (s_Status == Status::WAITING_TIMEOUT) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		if (__builtin_expect(((nMillis - s_nLastUpdateMillis) < (3600 * 1000)), 1)) {
			__disable_fiq(); //TODO Can this be done different? I am not happy now ;)
			return;
		}

		if (GPS::GetStatus() == GPSStatus::VALID) {
			m_nWaitPPSMillis = nMillis;

			s_Status = Status::WAITING_PPS;
			__enable_fiq();

			DEBUG_PUTS("(GPS::GetStatus() == GPSStatus::VALID)");
			return;
		}

		DEBUG_PUTS("No Fix");

		if (GPS::IsTimeUpdated()) {
			const uint32_t nElapsedMillis = nMillis - GetTimeTimestampMillis();

			if (nElapsedMillis < (1 * 1000)) {

				struct timeval tv;
				tv.tv_sec = GPS::GetLocalSeconds();
				tv.tv_usec = 0;
				settimeofday(&tv, nullptr);

				s_nLastUpdateMillis = nMillis;
				s_Status = Status::WAITING_TIMEOUT;

				DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nElapsedMillis);

				return;
			}
		}

		s_Status = Status::NOT_SET;

		DEBUG_PUTS("No time update");
		return;
	}

	if (s_Status == Status::WAITING_PPS) {
		const uint32_t nMillis = Hardware::Get()->Millis();

		if (__builtin_expect(((nMillis - m_nWaitPPSMillis) > (1 * 1000)), 0)) {
			// There is no PPS
			if (GPS::IsTimeUpdated()) {

				struct timeval tv;
				tv.tv_sec = GPS::GetLocalSeconds();
				tv.tv_usec = 0;
				settimeofday(&tv, nullptr);

				DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", nMillis - GetTimeTimestampMillis());
			}

			s_nLastUpdateMillis = nMillis;

			s_Status = Status::WAITING_TIMEOUT;
			__disable_fiq();

			DEBUG_PUTS("((tv.tv_sec - nWaitPPS) >= 1 * 1000)");
			return;
		}

		return;
	}

	if ((s_Status == Status::NOT_SET) && GPS::IsTimeUpdated()) {
		m_nWaitPPSMillis = Hardware::Get()->Millis();

		s_Status = Status::WAITING_PPS;
		__enable_fiq();

		DEBUG_PUTS("((Status == Status::NOT_SET) && GPS::IsTimeUpdated())");
		return;
	}
}
