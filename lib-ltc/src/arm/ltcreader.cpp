/**
 * @file ltcreader.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_ARM_LTCREADER)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "arm/ltcreader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "hardware.h"

// Output
#include "artnetnode.h"
#include "midi.h"
#include "ltcetc.h"
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

#if defined (H3)
# if __GNUC__ > 8
#  pragma GCC target ("general-regs-only")
# endif
#endif

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	380	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63
#define END_SYNC_POSITION	77
#define END_SMPTE_POSITION	80

static volatile uint32_t nFiqUsPrevious = 0;
static volatile uint32_t nFiqUsCurrent = 0;

static volatile uint32_t sv_nBitTime;
static volatile uint32_t sv_nTotalBits;
static volatile uint32_t nCurrentBit = 0;
static volatile uint32_t nSyncCount = 0;
static volatile bool bOnesBitCount = false;
static volatile bool bTimeCodeSync = false;
static volatile bool bTimeCodeValid = false;

static volatile uint8_t aTimeCodeBits[8] ALIGNED;
static volatile bool bIsDropFrameFlagSet = false;

static volatile bool bTimeCodeAvailable;
static volatile struct midi::Timecode s_midiTimeCode = { 0, 0, 0, 0, static_cast<uint8_t>(midi::TimecodeType::EBU) };

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}

static void __attribute__((interrupt("FIQ"))) fiq_handler() {
#elif defined (GD32)
extern "C" {
void EXTI10_15_IRQHandler() {
	if (0 != exti_interrupt_flag_get(EXTI_14)) {
		exti_interrupt_flag_clear(EXTI_14);
# ifndef NDEBUG
		GPIO_TG(GPIOA) = GPIO_PIN_4;
# endif
#endif
	__DMB();

#if defined (H3)
	nFiqUsCurrent = h3_hs_timer_lo_us();

	H3_PIO_PA_INT->STA = static_cast<uint32_t>(~0x0);

	if (nFiqUsPrevious >= nFiqUsCurrent) {
		sv_nBitTime = nFiqUsPrevious - nFiqUsCurrent;
		sv_nBitTime = 42949672 - sv_nBitTime;
	} else {
		sv_nBitTime = nFiqUsCurrent - nFiqUsPrevious;
	}
#elif defined (GD32)
	nFiqUsCurrent = TIMER_CNT(TIMER5);
	if (nFiqUsCurrent >= nFiqUsPrevious) {
		sv_nBitTime = nFiqUsCurrent - nFiqUsPrevious;
	} else {
		sv_nBitTime = UINT16_MAX - (nFiqUsPrevious - nFiqUsCurrent);
	}
#endif

	nFiqUsPrevious = nFiqUsCurrent;

	if ((sv_nBitTime < ONE_TIME_MIN) || (sv_nBitTime > ZERO_TIME_MAX)) {
		sv_nTotalBits = 0;
	} else {
		if (bOnesBitCount) {
			bOnesBitCount = false;
		} else {
			if (sv_nBitTime > ZERO_TIME_MIN) {
				nCurrentBit = 0;
				nSyncCount = 0;
			} else {
				nCurrentBit = 1;
				bOnesBitCount = true;
				nSyncCount = nSyncCount + 1;

				if (nSyncCount == 12) {
					nSyncCount = 0;
					bTimeCodeSync = true;
					sv_nTotalBits = END_SYNC_POSITION;
				}
			}

			if (sv_nTotalBits <= END_DATA_POSITION) {
				aTimeCodeBits[0] = static_cast<uint8_t>(aTimeCodeBits[0] >> 1);
				for (uint32_t n = 1; n < 8; n++) {
					if (aTimeCodeBits[n] & 1) {
						aTimeCodeBits[n - 1] |= 0x80;
					}
					aTimeCodeBits[n] = static_cast<uint8_t>(aTimeCodeBits[n] >> 1);
				}

				if (nCurrentBit == 1) {
					aTimeCodeBits[7] |= 0x80;
				}
			}

			sv_nTotalBits = sv_nTotalBits + 1;
		}

		if (sv_nTotalBits == END_SMPTE_POSITION) {

			sv_nTotalBits = 0;

			if (bTimeCodeSync) {
				bTimeCodeSync = false;
				bTimeCodeValid = true;
			}
		}

		if (bTimeCodeValid) {
			gv_ltc_nUpdates = gv_ltc_nUpdates + 1;

			bTimeCodeValid = false;

			s_midiTimeCode.nFrames  = static_cast<uint8_t>((10 * (aTimeCodeBits[1] & 0x03)) + (aTimeCodeBits[0] & 0x0F));
			s_midiTimeCode.nSeconds = static_cast<uint8_t>((10 * (aTimeCodeBits[3] & 0x07)) + (aTimeCodeBits[2] & 0x0F));
			s_midiTimeCode.nMinutes = static_cast<uint8_t>((10 * (aTimeCodeBits[5] & 0x07)) + (aTimeCodeBits[4] & 0x0F));
			s_midiTimeCode.nHours   = static_cast<uint8_t>((10 * (aTimeCodeBits[7] & 0x03)) + (aTimeCodeBits[6] & 0x0F));

			bIsDropFrameFlagSet = (aTimeCodeBits[1] & (1 << 2));

			bTimeCodeAvailable = true;
		}
	}
#if defined (GD32)
	}
#endif
	__DMB();
}
#if defined (GD32)
}
#endif

void LtcReader::Start() {
	bTimeCodeAvailable = false;

#if defined (H3)
	/**
	 * IRQ Timer
	 */
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_handler_init();

	/**
	 * FIQ GPIO
	 */
	h3_gpio_fsel(GPIO_EXT_26, GPIO_FSEL_EINT);

	arm_install_handler(reinterpret_cast<unsigned>(fiq_handler), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG1 = (GPIO_INT_CFG_DOUBLE_EDGE << 8);
	H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->STA = (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->DEB = 1;

	__enable_fiq();
#elif defined (GD32)
	/**
	 * https://www.gd32-dmx.org/dev-board.html
	 * GPIO_EXT_26 = PA14
	 */
	static_assert(GD32_GPIO_TO_NUMBER(GPIO_EXT_26) == 14, "GPIO PIN is not 14");
	static_assert(GD32_GPIO_TO_PORT(GPIO_EXT_26) == GD32_GPIO_PORTA, "GPIO PORT is not A");

	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_14);
	/* connect key EXTI line to key GPIO pin */
	syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN14);
	/* configure key EXTI line */
	exti_init(EXTI_14, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
	exti_interrupt_flag_clear(EXTI_14);
    NVIC_EnableIRQ(EXTI10_15_IRQn);
#ifndef NDEBUG
	rcu_periph_clock_enable(RCU_GPIOA);
# if defined (GPIO_INIT)
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
# else
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED, GPIO_PIN_4);
# endif
	GPIO_BOP(GPIOA) = GPIO_PIN_4;
# endif
#endif

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
}

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void LtcReader::Run() {
	__DMB();
	if (bTimeCodeAvailable) {
		__DMB();
		bTimeCodeAvailable = false;
		[[maybe_unused]] auto TimeCodeType = ltc::Type::UNKNOWN;

		__DMB();
		if (bIsDropFrameFlagSet) {
			TimeCodeType = ltc::Type::DF;
		} else {
			if (gv_ltc_nUpdatesPerSecond <= 24) {
				TimeCodeType = ltc::Type::FILM;
			} else if (gv_ltc_nUpdatesPerSecond <= 26) {
				TimeCodeType = ltc::Type::EBU;
			} else if (gv_ltc_nUpdatesPerSecond >= 28) {
				TimeCodeType = ltc::Type::SMPTE;
			} else {
				return;
			}
		}

		s_midiTimeCode.nType = static_cast<uint8_t>(TimeCodeType);

		g_ltc_LtcTimeCode.nFrames = s_midiTimeCode.nFrames;
		g_ltc_LtcTimeCode.nSeconds = s_midiTimeCode.nSeconds;
		g_ltc_LtcTimeCode.nMinutes = s_midiTimeCode.nMinutes;
		g_ltc_LtcTimeCode.nHours = s_midiTimeCode.nHours;
		g_ltc_LtcTimeCode.nType = s_midiTimeCode.nType;

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&g_ltc_LtcTimeCode));
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(const_cast<struct midi::Timecode *>(&s_midiTimeCode)));
		}

		LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
	}

	__DMB();
	if ((gv_ltc_nUpdatesPerSecond >= 24) && (gv_ltc_nUpdatesPerSecond <= 30)) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	}
}
