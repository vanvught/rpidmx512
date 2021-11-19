/**
 * @file  hwclockrtc.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cassert>
#include <cstring>
#include <time.h>

#include "hardware.h"
#include "gd32.h"

#include "debug.h"

void rtc_configuration(void) {
	/* enable PMU and BKPI clocks */
	rcu_periph_clock_enable (RCU_BKPI);
	rcu_periph_clock_enable (RCU_PMU);
	/* allow access to BKP domain */
	pmu_backup_write_enable();
	/* reset backup domain */
	bkp_deinit();
	/* enable LXTAL */
	rcu_osci_on (RCU_LXTAL);
	/* wait till LXTAL is ready */
	rcu_osci_stab_wait(RCU_LXTAL);
	/* select RCU_LXTAL as RTC clock source */
	rcu_rtc_clock_config (RCU_RTCSRC_LXTAL);
	/* enable RTC Clock */
	rcu_periph_clock_enable (RCU_RTC);
	/* wait for RTC registers synchronization */
	rtc_register_sync_wait();
	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
	/* set RTC prescaler: set RTC period to 1s */
	rtc_prescaler_set(32767);
	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
}

using namespace rtc;

void HwClock::RtcProbe() {
	DEBUG_ENTRY

	if (bkp_data_read(BKP_DATA_0) != 0xA5A5) {
		DEBUG_PUTS("RTC not yet configured");
		rtc_configuration();
		/* wait until last write operation on RTC registers has finished */
		rtc_lwoff_wait();
		/* change the current time */
		rtc_counter_set(time(nullptr));
		/* wait until last write operation on RTC registers has finished */
		rtc_lwoff_wait();
		bkp_data_write(BKP_DATA_0, 0xA5A5);
	} else {
		DEBUG_PUTS("No need to configure RTC");
		/* wait for RTC registers synchronization */
		rtc_register_sync_wait();
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
	}

    /* clear reset flags */
    rcu_all_reset_flag_clear();

	m_Type = rtc::Type::SOC_INTERNAL;
	m_bIsConnected = true;
	m_nLastHcToSysMillis = Hardware::Get()->Millis();

	DEBUG_EXIT
}

bool HwClock::RtcSet(const struct rtc_time *pRtcTime) {
	DEBUG_ENTRY
	assert(pRtcTime != nullptr);

	rtc_lwoff_wait();
    rtc_counter_set(mktime(const_cast<struct tm *>(reinterpret_cast<const struct tm *>(pRtcTime))));
    rtc_lwoff_wait();

	DEBUG_EXIT
	return true;
}

bool HwClock::RtcGet(struct rtc_time *pRtcTime) {
	DEBUG_ENTRY
	assert(pRtcTime != nullptr);

	const auto nSeconds = static_cast<time_t>(rtc_counter_get());
	const auto *pTm = localtime(&nSeconds);
	memcpy(pRtcTime, pTm, sizeof(struct rtc_time));

	DEBUG_EXIT
	return true;
}
