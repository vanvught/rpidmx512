/**
 * @file ltcgenerator.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcgenerator.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "network.h"

// Buttons
#include "h3_board.h"
#include "h3_gpio.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "ltcoutputs.h"

#include "../arm/platform_ltc.h"

#include "debug.h"

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1 << x)) == (1 << x))

#define BUTTON0_GPIO		GPIO_EXT_22		// PA2 Start
#define BUTTON1_GPIO		GPIO_EXT_15		// PA3 Stop
#define BUTTON2_GPIO		GPIO_EXT_7		// PA6 Resume

#define BUTTONS_MASK		((1 << BUTTON0_GPIO) |  (1 << BUTTON1_GPIO) | (1 << BUTTON2_GPIO))

namespace udp {
static constexpr auto PORT = 0x5443;
}

static void irq_timer0_handler(__attribute__((unused)) uint32_t clo) {
	if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(static_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
	}

	gv_ltc_bTimeCodeAvailable = true;
}

void LtcGenerator::Start() {
	DEBUG_ENTRY

	// Buttons
	h3_gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_EINT); // PA2
	h3_gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_EINT);	// PA3
	h3_gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_EINT);	// PA6

	h3_gpio_pud(BUTTON0_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON1_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON2_GPIO, GPIO_PULL_UP);

	h3_gpio_int_cfg(BUTTON0_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON1_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON2_GPIO, GPIO_INT_CFG_NEG_EDGE);

	H3_PIO_PA_INT->CTL |= BUTTONS_MASK;
	H3_PIO_PA_INT->STA = BUTTONS_MASK;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);

	// UDP Request
	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

	// Generator
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	LtcOutputs::Get()->Init();

	if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
	}

	if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&g_ltc_LtcTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
		LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);

	DEBUG_EXIT
}

void LtcGenerator::Stop() {
	DEBUG_ENTRY

	__disable_irq();
	irq_timer_set(IRQ_TIMER_0, nullptr);

	m_nHandle = Network::Get()->End(udp::PORT);

	DEBUG_EXIT
}

void LtcGenerator::HandleButtons() {
	m_nButtons = H3_PIO_PA_INT->STA & BUTTONS_MASK;

	if (__builtin_expect((m_nButtons != 0), 0)) {
		H3_PIO_PA_INT->STA = BUTTONS_MASK;

		DEBUG_PRINTF("%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO));

		if (BUTTON_STATE(BUTTON0_GPIO)) {
			ActionStart();
		} else if (BUTTON_STATE(BUTTON1_GPIO)) {
			ActionStop();
		} else if (BUTTON_STATE(BUTTON2_GPIO)) {
			ActionResume();
		}
	}
}
