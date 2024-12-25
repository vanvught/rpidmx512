/**
 * @file platform_ltc.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (CONFIG_TIMER6_HAVE_NO_IRQ_HANDLER)
# error
#endif

#include <cstdint>
#include <cassert>

#include "gd32_platform_ltc.h"
#include "gd32.h"
#include "ltc.h"
#include "timecodeconst.h"

extern struct HwTimersSeconds g_Seconds;

extern "C" {
/**
 * Timer 3
 */
#if defined DEBUG_LTC_TIMER3
void TIMER3_IRQHandler() { // Slave
	const auto nIntFlag = TIMER_INTF(TIMER3);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		GPIO_TG(DEBUG_TIMER3_GPIOx) = DEBUG_TIMER3_GPIO_PINx;
	}

	TIMER_INTF(TIMER3) = static_cast<uint32_t>(~nIntFlag);
}
#endif
/**
 * Timer 6
 */
void TIMER6_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER6);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
		gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
		g_Seconds.nUptime++;
	}

	TIMER_INTF(TIMER6) = static_cast<uint32_t>(~nIntFlag);
}
/**
 * Timer 11
 */
void TIMER7_BRK_TIMER11_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER11);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		gv_ltc_bTimeCodeAvailable = true;
		gv_ltc_nTimeCodeCounter++;

#if defined DEBUG_LTC_TIMER11
		GPIO_TG(DEBUG_TIMER11_GPIOx) = DEBUG_TIMER11_GPIO_PINx;
#endif
	}

	TIMER_INTF(TIMER11) = static_cast<uint32_t>(~nIntFlag);
}
}


namespace platform::ltc {

/**
 * Timer 11 is Master -> TIMER3_TRGO
 * Timer 3 is Slave -> ITI0
 */

void timer3_config() {
	rcu_periph_clock_enable(RCU_TIMER3);
	timer_deinit(TIMER3);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PRESCALER;
	timer_initpara.period = static_cast<uint32_t>(~0);;
	timer_init(TIMER3, &timer_initpara);

	timer_counter_value_config(TIMER3, 0);

	timer_master_slave_mode_config(TIMER3, TIMER_MASTER_SLAVE_MODE_DISABLE);
	timer_slave_mode_select(TIMER3, TIMER_SLAVE_MODE_RESTART);
	timer_input_trigger_source_select(TIMER3, TIMER_SMCFG_TRGSEL_ITI0);

    timer_dma_enable(TIMER3, TIMER_DMA_UPD);

#if defined DEBUG_LTC_TIMER3
	timer_interrupt_flag_clear(TIMER3, ~0);
	timer_interrupt_enable(TIMER3, TIMER_INT_UP);

	NVIC_SetPriority(TIMER3_IRQn, 0); // Highest priority
	NVIC_EnableIRQ(TIMER3_IRQn);

	rcu_periph_clock_enable(DEBUG_TIMER3_RCU_GPIOx);
#if !defined (GD32F4XX)
	gpio_init(DEBUG_TIMER3_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER3_GPIO_PINx);
#else
    gpio_mode_set(DEBUG_TIMER3_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, DEBUG_TIMER3_GPIO_PINx);
    gpio_output_options_set(DEBUG_TIMER3_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER3_GPIO_PINx);
#endif
	GPIO_BOP(DEBUG_TIMER3_GPIOx) = DEBUG_TIMER3_GPIO_PINx;
#endif
}

/*
 * sv_isMidiQuarterFrameMessage
 */
void timer10_config() {
	rcu_periph_clock_enable(RCU_TIMER10);
	timer_deinit(TIMER10);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PRESCALER;
	timer_initpara.period = static_cast<uint32_t>(~0);;
	timer_init(TIMER10, &timer_initpara);

	timer_counter_value_config(TIMER10, 0);

	timer_interrupt_flag_clear(TIMER10, ~0);
	timer_interrupt_enable(TIMER10, TIMER_INT_UP);

	NVIC_SetPriority(TIMER0_TRG_CMT_TIMER10_IRQn, 0);
	NVIC_EnableIRQ(TIMER0_TRG_CMT_TIMER10_IRQn);

#if defined DEBUG_LTC_TIMER10
	rcu_periph_clock_enable(DEBUG_TIMER10_RCU_GPIOx);
#if !defined (GD32F4XX)
	gpio_init(DEBUG_TIMER10_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER10_GPIO_PINx);
#else
    gpio_mode_set(DEBUG_TIMER10_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, DEBUG_TIMER10_GPIO_PINx);
    gpio_output_options_set(DEBUG_TIMER10_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER10_GPIO_PINx);
#endif
	GPIO_BOP(DEBUG_TIMER10_GPIOx) = DEBUG_TIMER10_GPIO_PINx;
#endif
}

/*
 * gv_ltc_bTimeCodeAvailable
 */
void timer11_config() {
	rcu_periph_clock_enable(RCU_TIMER11);
	timer_deinit(TIMER11);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PRESCALER;
	timer_initpara.period = static_cast<uint32_t>(~0);
	timer_init(TIMER11, &timer_initpara);

	timer_counter_value_config(TIMER11, 0);

	timer_master_slave_mode_config(TIMER11, TIMER_MASTER_SLAVE_MODE_DISABLE);
	timer_master_output_trigger_source_select(TIMER11, TIMER_TRI_OUT_SRC_UPDATE);

	timer_interrupt_flag_clear(TIMER11, ~0);
	timer_interrupt_enable(TIMER11, TIMER_INT_UP);

	NVIC_SetPriority(TIMER7_BRK_TIMER11_IRQn, 0);
	NVIC_EnableIRQ(TIMER7_BRK_TIMER11_IRQn);

#if defined DEBUG_LTC_TIMER11
	rcu_periph_clock_enable(DEBUG_TIMER11_RCU_GPIOx);
#if !defined (GD32F4XX)
	gpio_init(DEBUG_TIMER11_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER11_GPIO_PINx);
#else
    gpio_mode_set(DEBUG_TIMER11_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, DEBUG_TIMER11_GPIO_PINx);
    gpio_output_options_set(DEBUG_TIMER11_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DEBUG_TIMER11_GPIO_PINx);
#endif
	GPIO_BOP(DEBUG_TIMER11_GPIOx) = DEBUG_TIMER11_GPIO_PINx;
#endif
}

/*
 * BPM
 */
void timer13_config() {

}

} // namespace platform::ltc

