/**
 * @file ltcetcreader.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_ARM_LTCETCREADER)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cassert>

#include "arm/ltcetcreader.h"
#include "ltc.h"
#include "hardware.h"
// Output
#include "artnetnode.h"
#include "ltcsender.h"
#include "tcnetdisplay.h"
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

void LtcEtcReader::Start() {
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

void LtcEtcReader::Stop() {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif

	DEBUG_EXIT
}

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

static inline bool timecode_is_equal(const struct ltc::TimeCode *pLtcTimeCode) {
	auto isEqual = false;
	const auto *pSrc = reinterpret_cast<const uint8_t *>(pLtcTimeCode);
	auto *pDst = reinterpret_cast<uint8_t *>(&g_ltc_LtcTimeCode);

	for (uint32_t i = 0; i < sizeof (struct ltc::TimeCode); i++) {
		isEqual |= (*pSrc == *pDst);
		*pDst++ = *pSrc++;
	}

	return !isEqual;
}

void LtcEtcReader::Handler(const midi::Timecode *pTimeCode) {
	m_nTimestamp = Hardware::Get()->Millis();

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(pTimeCode));
	}

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(pTimeCode));
	}

	if (!timecode_is_equal(reinterpret_cast<const struct ltc::TimeCode *>(pTimeCode))) {
		LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
	}

	gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}
