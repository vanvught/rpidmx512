/**
 * @file ltcetcreader.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "ltcetcreader.h"
#include "hardware.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "ltcsender.h"
#include "tcnetdisplay.h"
#include "ltcoutputs.h"

#include "platform_ltc.h"

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

void LtcEtcReader::Start() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_timer_init();
#elif defined (GD32)
	platform::ltc::timer6_config();
#endif

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
}

void LtcEtcReader::Stop() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif
}

void LtcEtcReader::Handler(const midi::Timecode *pTimeCode) {
	gv_ltc_nUpdates++;

	if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(pTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(pTimeCode));
	}

	if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(pTimeCode);
	}

	memcpy(&m_tMidiTimeCode, pTimeCode, sizeof(struct midi::Timecode));

	LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(pTimeCode));
}

void LtcEtcReader::Run() {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

	__DMB();
	if (gv_ltc_nUpdatesPerSecond != 0) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	}
}
