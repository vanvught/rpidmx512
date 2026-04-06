/**
 * @file systimereader.cpp
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARM_SYSTIMEREADER)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <sys/time.h>
#include <cassert>

#include "arm/systimereader.h"
#include "network_udp.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "hal_statusled.h"
// Output
#include "ltcetc.h"
#include "ltcsender.h"
#include "display.h"
#include "arm/ltcoutputs.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

static constexpr char kCmdStart[] = "start";
static constexpr auto kStartLength = sizeof(kCmdStart) - 1;

static constexpr char kCmdStop[] = "stop";
static constexpr auto kStopLength = sizeof(kCmdStop) - 1;

static constexpr char kCmdRate[] = "rate#";
static constexpr auto kRateLength = sizeof(kCmdRate) - 1;

static constexpr uint16_t kUdpPort = 0x5443;

#if defined(H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo)
{
    gv_ltc_bTimeCodeAvailable = true;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

SystimeReader::SystimeReader(uint8_t fps, int32_t utc_offset) : fps_(fps), utc_offset_(utc_offset)
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    g_ltc_LtcTimeCode.type = static_cast<uint8_t>(ltc::g_Type);

    DEBUG_EXIT();
}

void SystimeReader::Start(bool auto_start)
{
    DEBUG_ENTRY();

#if defined(H3)
    irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

    H3_TIMER->TMR0_CUR = 0;
    H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[static_cast<uint8_t>(ltc::g_Type)];
    H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

    __enable_irq();
    __DMB();
#elif defined(GD32)
    platform::ltc::timer11_config();
    platform::ltc::timer11_set_type(static_cast<uint8_t>(ltc::g_Type));
#endif

    handle_ = network::udp::Begin(kUdpPort, StaticCallbackFunction);
    assert(handle_ != -1);

    LtcOutputs::Get()->Init();
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    if (auto_start)
    {
        ActionStart();
    }

    DEBUG_EXIT();
}

void SystimeReader::SetFps(uint8_t fps)
{
    if (fps != fps_)
    {
        fps_ = fps;

        if (g_ltc_LtcTimeCode.frames >= fps_)
        {
            g_ltc_LtcTimeCode.frames = static_cast<uint8_t>(fps_ - 1);
        }

        const auto kType = static_cast<uint8_t>(ltc::g_Type);
        g_ltc_LtcTimeCode.type = kType;

#if defined(H3)
        H3_TIMER->TMR0_CUR = 0;
        H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[kType];
        H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
        platform::ltc::timer11_set_type(kType);
#endif

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
        {
            LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET))
        {
            artnet::SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&g_ltc_LtcTimeCode));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
        {
            LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));
        }

        LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
    }
}

void SystimeReader::ActionStart()
{
    DEBUG_ENTRY();

    if (started_)
    {
        DEBUG_EXIT();
        return;
    }

    started_ = true;

    LtcOutputs::Get()->ResetTimeCodeTypePrevious();

    hal::statusled::SetMode(hal::statusled::Mode::DATA);

    DEBUG_EXIT();
}

void SystimeReader::ActionStop()
{
    DEBUG_ENTRY();

    started_ = false;

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    DEBUG_EXIT();
}

void SystimeReader::ActionSetRate(const char* timecode_rate)
{
    DEBUG_ENTRY();

    uint8_t fps;

    if (ltc::parse_timecode_rate(timecode_rate, fps))
    {
        SetFps(fps);
    }

    DEBUG_EXIT();
}

void SystimeReader::HandleRequest(char* buffer, uint16_t buffer_length)
{
    if (buffer != nullptr)
    {
        assert(buffer_length >= 8);
        udp_buffer_ = buffer;
        bytes_received_ = buffer_length;
    }

    if (memcmp("ltc!", udp_buffer_, 4) != 0)
    {
        return;
    }

    if (udp_buffer_[bytes_received_ - 1] == '\n')
    {
        DEBUG_PUTS("\'\\n\'");
        bytes_received_--;
    }

    debug::Dump(udp_buffer_, bytes_received_);

    if (bytes_received_ == (4 + kStartLength))
    {
        if (memcmp(&udp_buffer_[4], kCmdStart, kStartLength) == 0)
        {
            ActionStart();
            return;
        }

        DEBUG_PUTS("Invalid !start command");
    }

    if (bytes_received_ == (4 + kStopLength))
    {
        if (memcmp(&udp_buffer_[4], kCmdStop, kStopLength) == 0)
        {
            ActionStop();
            return;
        }

        DEBUG_PUTS("Invalid !stop command");
    }

    if (bytes_received_ == (4 + kRateLength + ltc::timecode::RATE_MAX_LENGTH))
    {
        if (memcmp(&udp_buffer_[4], kCmdRate, kRateLength) == 0)
        {
            ActionSetRate(&udp_buffer_[(4 + kRateLength)]);
            return;
        }
    }

    DEBUG_PUTS("Invalid command");
}

void SystimeReader::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    udp_buffer_ = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));
    bytes_received_ = size;

    HandleRequest();
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

void SystimeReader::Run()
{
    // If not started, return early
    if (__builtin_expect((started_), 0))
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        auto time_seconds = tv.tv_sec + utc_offset_;

        // Calculate frames
        g_ltc_LtcTimeCode.frames = (tv.tv_usec * TimeCodeConst::FPS[g_ltc_LtcTimeCode.type]) / 1000000U;

        // Drop-frame adjustments BEFORE time updates
        if (ltc::g_Type == ltc::Type::DF)
        {
            // Skip frames 00 and 01 in non-10th minutes
            if ((g_ltc_LtcTimeCode.minutes % 10 != 0) && (g_ltc_LtcTimeCode.seconds == 0) && (g_ltc_LtcTimeCode.frames < 2))
            {
                g_ltc_LtcTimeCode.frames = 2;
            }
        }

        // Update timecode components if the time has changed
        if (__builtin_expect((time_previous_ != time_seconds), 0))
        {
            time_previous_ = time_seconds;

            g_ltc_LtcTimeCode.seconds = static_cast<uint8_t>(time_seconds % 60U);
            time_seconds /= 60U;
            g_ltc_LtcTimeCode.minutes = static_cast<uint8_t>(time_seconds % 60U);
            time_seconds /= 60U;
            g_ltc_LtcTimeCode.hours = static_cast<uint8_t>(time_seconds % 24U);

            // Trigger timecode availability at the start of a second
            if (tv.tv_usec == 0)
            {
#if defined(H3)
                H3_TIMER->TMR0_CUR = 0;
                H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
                TIMER_CNT(TIMER11) = 0;
                TIMER_CTL0(TIMER11) |= TIMER_CTL0_CEN;
#endif
                gv_ltc_bTimeCodeAvailable = true;
            }
        }
    }

    // Update timecode outputs if available
    __DMB(); // Data memory barrier to ensure memory consistency
    if (__builtin_expect((gv_ltc_bTimeCodeAvailable), 0))
    {
        gv_ltc_bTimeCodeAvailable = false;

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
        {
            LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode), false);
        }

        if (__builtin_expect((!started_), 0))
        {
            return;
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET))
        {
            artnet::SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&g_ltc_LtcTimeCode));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
        {
            LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));
        }

        if (__builtin_expect((started_), 0))
        {
            LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
        }
    }
}
