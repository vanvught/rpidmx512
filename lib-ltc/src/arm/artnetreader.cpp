/**
 * @file artnetreader.cpp
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
#include <cassert>

#include "artnetreader.h"

#include "ltc.h"
#include "timecodeconst.h"
#include "ledblink.h"

// Input
#include "artnettimecode.h"
// Output
#include "rtpmidi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "ltcoutputs.h"

#include "platform_ltc.h"

#if defined (H3)
static volatile uint32_t sv_nUpdatesPerSecond;
static volatile uint32_t sv_nUpdatesPrevious;
static volatile uint32_t sv_nUpdates;

static void arm_timer_handler() {
	sv_nUpdatesPerSecond = sv_nUpdates - sv_nUpdatesPrevious;
	sv_nUpdatesPrevious = sv_nUpdates;
}
#elif defined (GD32)
#endif

ArtNetReader::ArtNetReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) : m_ptLtcDisabledOutputs(pLtcDisabledOutputs) {
	assert(m_ptLtcDisabledOutputs != nullptr);
}

void ArtNetReader::Start() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_timer_init();
#elif defined (GD32)
#endif

	LtcOutputs::Get()->Init();
	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
}

void ArtNetReader::Stop() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif
}

void ArtNetReader::Handler(const struct TArtNetTimeCode *ArtNetTimeCode) {
#if defined (H3)
	sv_nUpdates++;
#elif defined (GD32)
#endif

	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(ArtNetTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(ArtNetTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bEtc) {
		LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(ArtNetTimeCode));
	}

	memcpy(&m_tMidiTimeCode, ArtNetTimeCode, sizeof(struct midi::Timecode));

	LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(ArtNetTimeCode));
}

void ArtNetReader::Run() {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode));

	__DMB();
	if (sv_nUpdatesPerSecond != 0) {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
