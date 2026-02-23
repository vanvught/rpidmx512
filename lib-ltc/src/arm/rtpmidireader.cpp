/**
 * @file rtpmidireader.cpp
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

#if defined(DEBUG_ARM_RTPMIDIREADER)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "arm/rtpmidireader.h"
#include "timecodeconst.h"
#include "hal.h"
#include "hal_statusled.h"
// Output
#include "artnetnode.h"
#include "midi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "arm/ltcoutputs.h"

static uint8_t s_qf[8] __attribute__((aligned(4))) = {0, 0, 0, 0, 0, 0, 0, 0};

#if defined(H3)
static void arm_timer_handler()
{
    gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
    gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}

static void irq_timer0_handler([[maybe_unused]] uint32_t clo)
{
    gv_ltc_bTimeCodeAvailable = true;
    gv_ltc_nTimeCodeCounter = gv_ltc_nTimeCodeCounter + 1;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

void RtpMidiReader::Start()
{
#if defined(H3)
    irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
    irq_handler_init();
#elif defined(GD32)
    platform::ltc::timer11_config();
#endif

    LtcOutputs::Get()->Init();
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
}

void RtpMidiReader::Stop()
{
#if defined(H3)
    irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(nullptr));
    irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined(GD32)
#endif
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

void RtpMidiReader::MidiMessage(const struct midi::Message* message)
{
    switch (static_cast<midi::Types>(message->type))
    {
        case midi::Types::TIME_CODE_QUARTER_FRAME:
            HandleMtcQf(message);
            break;
        case midi::Types::SYSTEM_EXCLUSIVE:
        {
            const auto* const kSystemExclusive = message->system_exclusive;
            if ((kSystemExclusive[1] == 0x7F) && (kSystemExclusive[2] == 0x7F) && (kSystemExclusive[3] == 0x01))
            {
                HandleMtc(message);
            }
        }
        break;
        case midi::Types::CLOCK:
        {
            uint32_t bpm;
            if (midi_bpm_.Get(message->timestamp, bpm))
            {
                LtcOutputs::Get()->ShowBPM(bpm);
            }
        }
        break;
        default:
            break;
    }
}

void RtpMidiReader::HandleMtc(const struct midi::Message* message)
{
    const auto* const kSystemExclusive = message->system_exclusive;

    timecode_.frames = kSystemExclusive[8];
    timecode_.seconds = kSystemExclusive[7];
    timecode_.minutes = kSystemExclusive[6];
    timecode_.hours = kSystemExclusive[5] & 0x1F;
    timecode_.type = static_cast<uint8_t>(kSystemExclusive[5] >> 5);

    Update();

    gv_ltc_bTimeCodeAvailable = false;
    gv_ltc_nTimeCodeCounter = 0;
}

void RtpMidiReader::HandleMtcQf(const struct midi::Message* midi_message)
{
    const auto kData1 = midi_message->data1;
    const auto kPart = static_cast<uint8_t>((kData1 & 0x70) >> 4);

    s_qf[kPart] = kData1 & 0x0F;

    if ((kPart == 7) || (part_previous_ == 7))
    {
    }
    else
    {
        direction_ = (part_previous_ < kPart);
    }

    if ((direction_ && (kPart == 7)) || (!direction_ && (kPart == 0)))
    {
        timecode_.frames = static_cast<uint8_t>(s_qf[0] | (s_qf[1] << 4));
        timecode_.seconds = static_cast<uint8_t>(s_qf[2] | (s_qf[3] << 4));
        timecode_.minutes = static_cast<uint8_t>(s_qf[4] | (s_qf[5] << 4));
        timecode_.hours = static_cast<uint8_t>(s_qf[6] | ((s_qf[7] & 0x1) << 4));
        timecode_.type = static_cast<uint8_t>((s_qf[7] >> 1));

        if (timecode_.frames < mtc_qf_previous_)
        {
            mtc_qf_delta_ = mtc_qf_previous_ - timecode_.frames;
        }
        else
        {
            mtc_qf_delta_ = timecode_.frames - mtc_qf_previous_;
        }

        mtc_qf_previous_ = timecode_.frames;

        if (mtc_qf_delta_ >= static_cast<uint32_t>(TimeCodeConst::FPS[timecode_.type] - 2))
        {
            mtc_qf_delta_ = 2;
        }

        __DMB();

        if (gv_ltc_nTimeCodeCounter < mtc_qf_delta_)
        {
            Update();
        }

#if defined(H3)
        H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
        H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[timecode_.type];
        H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
        platform::ltc::timer11_set_type(timecode_.type);
#endif
        gv_ltc_bTimeCodeAvailable = false;
        gv_ltc_nTimeCodeCounter = 0;
    }

    part_previous_ = kPart;
}

void RtpMidiReader::Update()
{
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
    {
        LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&timecode_));
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET))
    {
        ArtNetNode::Get()->SendTimeCode(reinterpret_cast<struct artnet::TimeCode*>(&timecode_));
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
    {
        LtcEtc::Get()->Send(reinterpret_cast<const midi::Timecode*>(&timecode_));
    }

    memcpy(&g_ltc_LtcTimeCode, &timecode_, sizeof(struct midi::Timecode));

    LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

    gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}

void RtpMidiReader::Run()
{
    __DMB();

    if (gv_ltc_bTimeCodeAvailable)
    {
        gv_ltc_bTimeCodeAvailable = false;

        const auto kFps = TimeCodeConst::FPS[timecode_.type];

        if (direction_)
        {
            timecode_.frames++;
            if (kFps == timecode_.frames)
            {
                timecode_.frames = 0;

                timecode_.seconds++;
                if (timecode_.seconds == 60)
                {
                    timecode_.seconds = 0;

                    timecode_.minutes++;
                    if (timecode_.minutes == 60)
                    {
                        timecode_.minutes = 0;

                        timecode_.hours++;
                        if (timecode_.hours == 24)
                        {
                            timecode_.hours = 0;
                        }
                    }
                }
            }
        }
        else
        {
            if (timecode_.frames == kFps - 1)
            {
                if (timecode_.seconds > 0)
                {
                    timecode_.seconds--;
                }
                else
                {
                    timecode_.seconds = 59;
                }

                if (timecode_.seconds == 59)
                {
                    if (timecode_.minutes > 0)
                    {
                        timecode_.minutes--;
                    }
                    else
                    {
                        timecode_.minutes = 59;
                    }

                    if (timecode_.minutes == 59)
                    {
                        if (timecode_.hours > 0)
                        {
                            timecode_.hours--;
                        }
                        else
                        {
                            timecode_.hours = 23;
                        }
                    }
                }
            }
        }

        Update();

        if (mtc_qf_delta_ == 2)
        {
            mtc_qf_delta_ = 0;
#if defined(H3)
            H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
            H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[timecode_.type];
            H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
            platform::ltc::timer11_set_type(timecode_.type);
#endif
        }
    }

    __DMB();
    if (gv_ltc_nUpdatesPerSecond != 0)
    {
        hal::statusled::SetMode(hal::statusled::Mode::DATA);
    }
    else
    {
        LtcOutputs::Get()->ShowSysTime();
        hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    }
}
