/**
 * @file artnetreader.cpp
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

#if defined(DEBUG_ARM_ARTNETREADER)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>

#include "artnet.h"
#include "network_udp.h"
#include "arm/artnetreader.h"
#include "ltc.h"
#include "hal.h"
#include "hal_statusled.h"
// Input
#include "artnettimecode.h"
// Output
#include "ltcetc.h"
#include "ltcsender.h"
#include "arm/ltcoutputs.h"

 #include "firmware/debug/debug_debug.h"

#if defined(H3)
static void arm_timer_handler()
{
    gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
    gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

void ArtNetReader::Start()
{
    DEBUG_ENTRY();

#if defined(H3)
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
    irq_handler_init();
#elif defined(GD32)
#endif

    LtcOutputs::Get()->Init();
    hal::statusled::SetMode(hal::statusled::Mode::kNormal);

    DEBUG_EXIT();
}

void ArtNetReader::Stop()
{
    DEBUG_ENTRY();
	
	assert(handle_ != -1);	
	handle_ = network::udp::End(artnet::kUdpPort);
	handle_ = -1;

#if defined(H3)
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined(GD32)
#endif

    DEBUG_EXIT();
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

static bool TimecodeIsEqual(const struct ltc::TimeCode* timecode)
{
    auto is_equal = false;
    const auto* src = reinterpret_cast<const uint8_t*>(timecode);
    auto* dst = reinterpret_cast<uint8_t*>(&g_ltc_LtcTimeCode);

    for (uint32_t i = 0; i < sizeof(struct ltc::TimeCode); i++)
    {
        is_equal |= (*src == *dst);
        *dst++ = *src++;
    }

    return !is_equal;
}

void ArtNetReader::Handler(const struct artnet::TimeCode* timecode)
{
    timestamp_ = hal::Millis();

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
    {
        LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(timecode));
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
    {
        LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(timecode));
    }

    if (!TimecodeIsEqual(reinterpret_cast<const struct ltc::TimeCode*>(timecode)))
    {
        LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
    }

    gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}
