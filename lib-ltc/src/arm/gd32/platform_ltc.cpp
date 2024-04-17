/**
 * @file platform_ltc.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ltc.h"

extern "C" {
void TIMER6_IRQHandler() {
	__DMB();

	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;

	__DMB();
}

void TIMER7_BRK_TIMER11_IRQHandler() {
	__DMB();
	const auto nIntFlag = TIMER_INTF(TIMER11);

	if ((nIntFlag & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		gv_ltc_bTimeCodeAvailable = true;
		gv_ltc_nTimeCodeCounter++;
	} else {
		assert(0);
	}

	timer_interrupt_flag_clear(TIMER11, nIntFlag);
	__DMB();
}
}

namespace platform {
namespace ltc {

void timer6_config() {
	rcu_periph_clock_enable(RCU_TIMER6);
	timer_deinit(TIMER6);

	timer_parameter_struct timer_initpara;
	timer_initpara.prescaler = TIMER_PSC_10KHZ;
	timer_initpara.period = 10000;		// 1 second
	timer_init(TIMER6, &timer_initpara);

	timer_flag_clear(TIMER6, ~0);
	timer_interrupt_flag_clear(TIMER6, ~0);

	timer_interrupt_enable(TIMER6, TIMER_INT_UP);

//	NVIC_SetPriority(TIMER6_IRQn, 0);
	NVIC_EnableIRQ(TIMER6_IRQn);

	timer_enable(TIMER6);
}

void timer11_config() {
	rcu_periph_clock_enable(RCU_TIMER11);
	timer_deinit(TIMER11);

	timer_parameter_struct timer_initpara;

	timer_initpara.prescaler = TIMER_PSC_1MHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = static_cast<uint32_t>(~0);
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER11, &timer_initpara);

	timer_flag_clear(TIMER11, ~0);
	timer_interrupt_flag_clear(TIMER11, ~0);

	timer_channel_output_fast_config(TIMER11, TIMER_CH_0, TIMER_OC_FAST_ENABLE);
	timer_channel_output_mode_config(TIMER11, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);

	NVIC_SetPriority(TIMER7_BRK_TIMER11_IRQn, 0);
	NVIC_EnableIRQ(TIMER7_BRK_TIMER11_IRQn);
}

}  // namespace ltc
}  // namespace platform
