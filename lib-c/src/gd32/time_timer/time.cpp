/**
 * @file time.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")

#include <cstddef>
#include <sys/time.h>
#include <cstdint>
#include <cassert>

#include "gd32.h"

#if defined(GD32H7XX)
# define TIMERx			TIMER16
# define RCU_TIMERx		RCU_TIMER16
# define TIMERx_IRQn	TIMER16_IRQn
#else
# define TIMERx			TIMER7
# define RCU_TIMERx		RCU_TIMER7
# if defined (GD32F10X) || defined (GD32F30X)
#  define TIMERx_IRQn	TIMER7_IRQn
# else
#  define TIMERx_IRQn	TIMER7_UP_TIMER12_IRQn
# endif
#endif

extern struct HwTimersSeconds g_Seconds;

extern "C" {
#if !defined (CONFIG_ENET_ENABLE_PTP)
# if defined (CONFIG_TIME_USE_TIMER)
#  if defined(GD32H7XX)
void TIMER16_IRQHandler() {
#  elif defined (GD32F10X) || defined (GD32F30X)
void TIMER7_IRQHandler() {
#  else
void TIMER7_UP_TIMER12_IRQHandler() {
#  endif
	const auto nIntFlag = TIMER_INTF(TIMERx);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		g_Seconds.nTimeval++;
	}

	TIMER_INTF(TIMERx) = static_cast<uint32_t>(~nIntFlag);
}
# endif
#endif
}

#if defined(GD32H7XX)
void timer16_config() {
#else
void timer7_config() {
#endif
	g_Seconds.nTimeval = 0;

	rcu_periph_clock_enable(RCU_TIMERx);
	timer_deinit(TIMERx);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_10KHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = (10000 - 1);		// 1 second
	timer_init(TIMERx, &timer_initpara);

	timer_interrupt_flag_clear(TIMERx, ~0);

	timer_interrupt_enable(TIMERx, TIMER_INT_UP);

	NVIC_SetPriority(TIMERx_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); // Lowest priority
	NVIC_EnableIRQ(TIMERx_IRQn);

	timer_enable(TIMERx);
}

extern "C" {
/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, __attribute__((unused))    struct timezone *tz) {
	assert(tv != 0);

#if __CORTEX_M == 7
	__DMB();
#endif

	tv->tv_sec = g_Seconds.nTimeval;
	tv->tv_usec = TIMER_CNT(TIMERx) * 100U;

#if __CORTEX_M == 7
	__ISB();
#endif

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused)) const struct timezone *tz) {
    assert(tv != 0);

    // Disable the timer interrupt to prevent it from triggering while we adjust the counter
    TIMER_DMAINTEN(TIMERx) &= static_cast<uint32_t>(~TIMER_INT_UP);
    TIMER_CTL0(TIMERx) &= static_cast<uint32_t>(~TIMER_CTL0_CEN);

    g_Seconds.nTimeval = tv->tv_sec;
    TIMER_CNT(TIMERx) = (tv->tv_usec / 100U) % 10000;

    TIMER_INTF(TIMERx) = static_cast<uint32_t>(~0);
    TIMER_DMAINTEN(TIMERx) |= TIMER_INT_UP;
    TIMER_CTL0(TIMERx) |= TIMER_CTL0_CEN;

    return 0;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	if (__timer != nullptr) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}
}
