/**
 * @file  hwclockrtc.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hwclock.h"

#include "hardware.h"
#include "gd32.h"

#include "debug.h"

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	static_cast<char>( (((val) / 10) << 4) + (val) % 10 )

#if defined (GD32F4XX) || defined (GD32H7XX)
# define RTC_CLOCK_SOURCE_LXTAL
static rtc_parameter_struct rtc_initpara;
#endif

#if defined (GD32F4XX) || defined (GD32H7XX)
bool rtc_configuration() {
# if defined (RTC_CLOCK_SOURCE_IRC32K)
	rcu_osci_on (RCU_IRC32K);
	if (SUCCESS != rcu_osci_stab_wait(RCU_IRC32K)) {
		return false;
	}
	rcu_rtc_clock_config (RCU_RTCSRC_IRC32K);
# elif defined (RTC_CLOCK_SOURCE_LXTAL)
	rcu_osci_on(RCU_LXTAL);

	if (SUCCESS != rcu_osci_stab_wait(RCU_LXTAL)) {
		return false;
	}

	rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
# else
#  error RTC clock source should be defined.
# endif
	rcu_periph_clock_enable(RCU_RTC);

	if (SUCCESS != rtc_register_sync_wait()) {
		return false;
	}

	rtc_initpara.date = DEC2BCD(_TIME_STAMP_DAY_);
	rtc_initpara.month = DEC2BCD(_TIME_STAMP_MONTH_ - 1);
	rtc_initpara.year = DEC2BCD(_TIME_STAMP_YEAR_ - 1900);

	if (SUCCESS != rtc_init(&rtc_initpara)) {
		DEBUG_PUTS("RTC time configuration failed!");
		return false;
	}

	DEBUG_PUTS("RTC time configuration success!");
	return true;
}
#else
bool rtc_configuration() {
	rcu_osci_on(RCU_LXTAL);

	if (SUCCESS != rcu_osci_stab_wait(RCU_LXTAL)) {
		return false;
	}

	rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
	rcu_periph_clock_enable(RCU_RTC);
	rtc_register_sync_wait();
	rtc_lwoff_wait();
	rtc_prescaler_set(32767);
	rtc_lwoff_wait();

	return true;
}
#endif

using namespace rtc;

void HwClock::RtcProbe() {
	DEBUG_ENTRY

#if defined (GD32F4XX) || defined (GD32H7XX)
# if defined (RTC_CLOCK_SOURCE_IRC32K)
	rtc_initpara.factor_syn = 0x13F;
	rtc_initpara.factor_asyn = = 0x63;
# elif defined (RTC_CLOCK_SOURCE_LXTAL)
	rtc_initpara.factor_syn = 0xFF;
	rtc_initpara.factor_asyn = 0x7F;
# else
#  error RTC clock source should be defined.
# endif
	rtc_initpara.display_format = RTC_24HOUR;
#endif

	if (bkp_data_read(BKP_DATA_0) != 0xA5A5) {
		DEBUG_PUTS("RTC not yet configured");

		if (!rtc_configuration()) {
			m_bIsConnected = false;
			DEBUG_PUTS("RTC did not start");
			DEBUG_EXIT
			return;
		}

		bkp_data_write(BKP_DATA_0, 0xA5A5);

		struct tm RtcTime;

		RtcTime.tm_hour = 0;
		RtcTime.tm_min = 0;
		RtcTime.tm_sec = 0;
		RtcTime.tm_mday = _TIME_STAMP_DAY_;
		RtcTime.tm_mon = _TIME_STAMP_MONTH_ - 1;
		RtcTime.tm_year = _TIME_STAMP_YEAR_ - 1900;

		RtcSet(&RtcTime);
	} else {
		DEBUG_PUTS("No need to configure RTC");
		rtc_register_sync_wait();
#if defined (GD32F4XX) || defined (GD32H7XX)
#else
		rtc_lwoff_wait();
#endif
	}

	m_Type = rtc::Type::SOC_INTERNAL;
	m_bIsConnected = true;
	m_nLastHcToSysMillis = Hardware::Get()->Millis();

	DEBUG_EXIT
}

bool HwClock::RtcSet(const struct tm *pTime) {
	assert(pTime != nullptr);

	DEBUG_PRINTF("sec=%d, min=%d, hour=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

#if defined (GD32F4XX) || defined (GD32H7XX)
	rtc_initpara.year = DEC2BCD(pTime->tm_year);
	rtc_initpara.month = DEC2BCD(pTime->tm_mon);
	rtc_initpara.date = DEC2BCD(pTime->tm_mday);
	rtc_initpara.day_of_week = DEC2BCD(pTime->tm_wday);
	rtc_initpara.hour = DEC2BCD(pTime->tm_hour);
	rtc_initpara.minute = DEC2BCD(pTime->tm_min);
	rtc_initpara.second = DEC2BCD(pTime->tm_sec);

	return (SUCCESS == rtc_init(&rtc_initpara));
#else
	rtc_counter_set(mktime(const_cast<struct tm *>(pTime)));
#endif
	return true;
}

bool HwClock::RtcGet(struct tm *pTime) {
	assert(pTime != nullptr);

#if defined (GD32F4XX) || defined (GD32H7XX)
   const auto tr = reinterpret_cast<uint32_t>(RTC_TIME);
   const auto dr = reinterpret_cast<uint32_t>(RTC_DATE);

   pTime->tm_year = BCD2DEC(GET_DATE_YR(dr));
   pTime->tm_mon = BCD2DEC(GET_DATE_MON(dr));
   pTime->tm_mday = BCD2DEC(GET_DATE_DAY(dr));
   pTime->tm_wday = BCD2DEC(GET_DATE_DOW(dr));
   pTime->tm_hour = BCD2DEC(GET_TIME_HR(tr));
   pTime->tm_min = BCD2DEC(GET_TIME_MN(tr));
   pTime->tm_sec = BCD2DEC(GET_TIME_SC(tr));
#else
   const auto nSeconds = static_cast<time_t>(rtc_counter_get());
   const auto *pTm = gmtime(&nSeconds);
   memcpy(pTime, pTm, sizeof(struct tm));
#endif

	DEBUG_PRINTF("sec=%d, min=%d, hour=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	return true;
}

bool HwClock::RtcSetAlarm(const struct tm *pTime) {
	DEBUG_ENTRY
	assert(pTime != nullptr);

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d, enabled=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday,
		m_bRtcAlarmEnabled);

#if defined (GD32F4XX) || defined (GD32H7XX)
	rtc_alarm_disable (RTC_ALARM0);
	rtc_alarm_struct rtc_alarm;

	rtc_alarm.alarm_mask = RTC_ALARM_ALL_MASK;
	rtc_alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
	rtc_alarm.am_pm = 0;
	rtc_alarm.alarm_day = DEC2BCD(pTime->tm_mday);
	rtc_alarm.alarm_hour = DEC2BCD(pTime->tm_hour);
	rtc_alarm.alarm_minute = DEC2BCD(pTime->tm_min);
	rtc_alarm.alarm_second = DEC2BCD(pTime->tm_sec);

	rtc_alarm_config(RTC_ALARM0, &rtc_alarm);

	if (m_bRtcAlarmEnabled) {
		rtc_interrupt_enable (RTC_INT_ALARM0);
		rtc_alarm_enable(RTC_ALARM0);
	} else {
		rtc_alarm_disable(RTC_ALARM0);
		rtc_interrupt_disable(RTC_INT_ALARM0);
	}
#else
	rtc_alarm_config(mktime(const_cast<struct tm *>(pTime)));
#endif

	DEBUG_EXIT
	return true;
}

bool HwClock::RtcGetAlarm(struct tm *pTime) {
	DEBUG_ENTRY
	assert(pTime != nullptr);

#if defined (GD32F4XX) || defined (GD32H7XX)
	if (!RtcGet(pTime)) {
		DEBUG_EXIT
		return false;
	}

	rtc_alarm_struct rtc_alarm;
	rtc_alarm_get(RTC_ALARM0, &rtc_alarm);

	pTime->tm_sec= BCD2DEC(rtc_alarm.alarm_second);
	pTime->tm_min = BCD2DEC(rtc_alarm.alarm_minute);
	pTime->tm_hour = BCD2DEC(rtc_alarm.alarm_hour);
	pTime->tm_mday = BCD2DEC(rtc_alarm.alarm_day);
#else
	const auto nSeconds = static_cast<time_t>((RTC_ALRMH << 16U) | RTC_ALRML);
	const auto *pTm = localtime(&nSeconds);
	memcpy(pTime, pTm, sizeof(struct tm));
#endif

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d, enabled=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday,
		m_bRtcAlarmEnabled);

	DEBUG_EXIT
	return true;
}
