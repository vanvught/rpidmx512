/**
 * @file artnetreader.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_ARM_ARTNETREADER)
# undef NDEBUG
#endif

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "arm/artnetreader.h"
#include "ltc.h"
#include "hardware.h"
// Input
#include "artnettimecode.h"
// Output
#include "ltcetc.h"
#include "ltcsender.h"
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

#include "debug.h"

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

void ArtNetReader::Start() {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_handler_init();
#elif defined (GD32)
#endif

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void ArtNetReader::Stop() {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif

	DEBUG_EXIT
}

void ArtNetReader::Handler(const struct artnet::TimeCode *ArtNetTimeCode) {
	gv_ltc_nUpdates = gv_ltc_nUpdates + 1;

	if (!ltc::g_DisabledOutputs.bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(ArtNetTimeCode));
	}

	if (!ltc::g_DisabledOutputs.bEtc) {
		LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(ArtNetTimeCode));
	}

	memcpy(&m_MidiTimeCode, ArtNetTimeCode, sizeof(struct midi::Timecode));

	LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(ArtNetTimeCode));
}
