/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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
#include <cassert>

#include "hardware.h"
#include "ledblink.h"

#include "uart0_debug.h"

#include "gd32_board.h"

#include "gd32_i2c.h"

#ifndef NDEBUG
# include "../debug/i2cdetect.h"
#endif

#include "debug.h"

extern "C" {
void systick_config(void);
#include "gd32f20x_rcu.h"
#include "gd32f20x_gpio.h"
#include "gd32f20x_timer.h"
}
#define LED_PIN                         GPIO_PIN_0
#define LED_GPIO_PORT                   GPIOC
#define LED_GPIO_CLK                    RCU_GPIOC

Hardware *Hardware::s_pThis = nullptr;

Hardware::Hardware() {
	assert(s_pThis == nullptr);
	s_pThis = this;

    rcu_periph_clock_enable(LED_GPIO_CLK);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);
    GPIO_BC(LED_GPIO_PORT) = LED_PIN;

	uart0_init();

    systick_config();

	rcu_periph_clock_enable(RCU_TIMER5);

	timer_deinit(TIMER5);
	timer_parameter_struct timer_initpara;
	timer_initpara.prescaler = 119;	///< TIMER1CLK = SystemCoreClock / 120 = 1MHz => us ticker
	timer_initpara.period = static_cast<uint32_t>(~0);
	timer_init(TIMER5, &timer_initpara);
	timer_enable(TIMER5);

	struct tm tmbuf;

	tmbuf.tm_hour = 0;
	tmbuf.tm_min = 0;
	tmbuf.tm_sec = 0;
	tmbuf.tm_mday = _TIME_STAMP_DAY_;			// The day of the month, in the range 1 to 31.
	tmbuf.tm_mon = _TIME_STAMP_MONTH_ - 1;		// The number of months since January, in the range 0 to 11.
	tmbuf.tm_year = _TIME_STAMP_YEAR_ - 1900;	// The number of years since 1900.
	tmbuf.tm_isdst = 0; 						// 0 (DST not in effect, just take RTC time)

	const auto seconds = mktime(&tmbuf);
	const struct timeval tv = { .tv_sec = seconds, .tv_usec = 0 };

	settimeofday(&tv, nullptr);

	gd32_i2c_begin();

#ifndef NDEBUG
	I2cDetect i2cdetect;
#endif

#if !defined(DISABLE_RTC)
	m_HwClock.RtcProbe();
	m_HwClock.Print();
	m_HwClock.HcToSys();
#endif
}

typedef union pcast32 {
	uuid_t uuid;
	uint32_t u32[4];
} _pcast32;

void Hardware::GetUuid(uuid_t out) {
	_pcast32 cast;

	cast.u32[0] = *(volatile uint32_t*)(0x1FFFF7E8);
	cast.u32[1] = *(volatile uint32_t*)(0x1FFFF7EC);
	cast.u32[2] = *(volatile uint32_t*)(0x1FFFF7F0);
	cast.u32[3] = cast.u32[0] + cast.u32[1] + cast.u32[2];

	memcpy(out, cast.uuid, sizeof(uuid_t));
}

bool Hardware::SetTime(__attribute__((unused)) const struct tm *pTime) {
#if !defined(DISABLE_RTC)
	rtc_time rtc_time;

	rtc_time.tm_sec = pTime->tm_sec;
	rtc_time.tm_min = pTime->tm_min;
	rtc_time.tm_hour = pTime->tm_hour;
	rtc_time.tm_mday = pTime->tm_mday;
	rtc_time.tm_mon = pTime->tm_mon;
	rtc_time.tm_year = pTime->tm_year;

	m_HwClock.Set(&rtc_time);

	return true;
#else
	return false;
#endif
}

bool Hardware::Reboot() {
	DEBUG_PUTS("Hardware::Reboot()");

	LedBlink::Get()->SetFrequency(8);

	if (m_pRebootHandler != 0) {
		WatchdogStop();
		m_pRebootHandler->Run();
	}

	WatchdogInit();

	DEBUG_PRINTF("m_bIsWatchdog=%d", m_bIsWatchdog);

	for (;;) {
		LedBlink::Get()->Run();
	}

	__builtin_unreachable();
	return true;
}
