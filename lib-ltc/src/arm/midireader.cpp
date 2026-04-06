/**
 * @file midireader.cpp
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

#if defined(DEBUG_ARM_MIDIREADER)
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

#include "arm/midireader.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "hal.h"
#include "hal_statusled.h"
// Input
#include "midi.h"
// Output
#include "ltcsender.h"
#include "ltcetc.h"
#include "net/rtpmidi.h"
#include "arm/ltcmidisystemrealtime.h"
#include "arm/ltcoutputs.h"

 #include "firmware/debug/debug_debug.h"

static uint8_t s_qf[8] __attribute__((aligned(4))) = {0, 0, 0, 0, 0, 0, 0, 0};

#if defined(H3)
static void irq_timer1_handler()
{
    gv_ltc_bTimeCodeAvailable = true;
    gv_ltc_nTimeCodeCounter = gv_ltc_nTimeCodeCounter + 1;
}
#elif defined(GD32)
// Defined in platform_ltc.cpp
#endif

void MidiReader::Start()
{
    DEBUG_ENTRY();

#if defined(H3)
    Midi::Get()->SetIrqTimer1(irq_timer1_handler);
#elif defined(GD32)
    platform::ltc::timer11_config();
#endif
    Midi::Get()->Init(midi::Direction::INPUT);

    LtcOutputs::Get()->Init(true);
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    DEBUG_EXIT();
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

void MidiReader::HandleMtc()
{
    uint8_t nSystemExclusiveLength;
    const auto* pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

    g_ltc_LtcTimeCode.hours = pSystemExclusive[5] & 0x1F;
    g_ltc_LtcTimeCode.minutes = pSystemExclusive[6];
    g_ltc_LtcTimeCode.seconds = pSystemExclusive[7];
    g_ltc_LtcTimeCode.frames = pSystemExclusive[8];
    g_ltc_LtcTimeCode.type = static_cast<uint8_t>(pSystemExclusive[5] >> 5);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI))
    {
        RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));
    }

    Update();

    gv_ltc_bTimeCodeAvailable = false;
}

void MidiReader::HandleMtcQf()
{
    uint8_t data1, data2;
    Midi::Get()->GetMessageData(data1, data2);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI))
    {
        RtpMidi::Get()->SendQf(data1);
    }

    const auto nPart = static_cast<uint8_t>((data1 & 0x70) >> 4);

    s_qf[nPart] = data1 & 0x0F;

    if ((nPart == 7) || (part_previous_ == 7))
    {
    }
    else
    {
        direction_ = (part_previous_ < nPart);
    }

    if ((direction_ && (nPart == 7)) || (!direction_ && (nPart == 0)))
    {
        g_ltc_LtcTimeCode.hours = static_cast<uint8_t>(s_qf[6] | ((s_qf[7] & 0x1) << 4));
        g_ltc_LtcTimeCode.minutes = static_cast<uint8_t>(s_qf[4] | (s_qf[5] << 4));
        g_ltc_LtcTimeCode.seconds = static_cast<uint8_t>(s_qf[2] | (s_qf[3] << 4));
        g_ltc_LtcTimeCode.frames = static_cast<uint8_t>(s_qf[0] | (s_qf[1] << 4));
        g_ltc_LtcTimeCode.type = static_cast<uint8_t>(s_qf[7] >> 1);

        if (g_ltc_LtcTimeCode.frames < mtc_qf_previous_)
        {
            mtc_qf_delta_ = mtc_qf_previous_ - g_ltc_LtcTimeCode.frames;
        }
        else
        {
            mtc_qf_delta_ = g_ltc_LtcTimeCode.frames - mtc_qf_previous_;
        }

        mtc_qf_previous_ = g_ltc_LtcTimeCode.frames;

        if (mtc_qf_delta_ >= static_cast<uint32_t>(TimeCodeConst::FPS[g_ltc_LtcTimeCode.type] - 2))
        {
            mtc_qf_delta_ = 2;
        }

        __DMB();

        if (gv_ltc_nTimeCodeCounter < mtc_qf_delta_)
        {
            Update();
        }

#if defined(H3)
        H3_TIMER->TMR1_CTRL |= TIMER_CTRL_SINGLE_MODE;
        H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[g_ltc_LtcTimeCode.type];
        H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START);
#elif defined(GD32)
        platform::ltc::timer11_set_type(g_ltc_LtcTimeCode.type);
#endif
        gv_ltc_bTimeCodeAvailable = false;
        gv_ltc_nTimeCodeCounter = 0;
    }

    part_previous_ = nPart;
}

void MidiReader::Update()
{
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
    {
        LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));
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

void MidiReader::Run()
{
    if (Midi::Get()->Read(static_cast<uint8_t>(midi::Channel::OMNI)))
    {
        if (Midi::Get()->GetChannel() == 0)
        {
            switch (Midi::Get()->GetMessageType())
            {
                case midi::Types::TIME_CODE_QUARTER_FRAME:
                    HandleMtcQf();
                    break;
                case midi::Types::SYSTEM_EXCLUSIVE:
                {
                    uint8_t nSystemExclusiveLength;
                    const auto* pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);
                    if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01))
                    {
                        HandleMtc();
                    }
                }
                break;
                case midi::Types::CLOCK:
                    uint32_t nBPM;
                    if (midi_bpm_.Get(Midi::Get()->GetMessageTimeStamp() * 10, nBPM))
                    {
                        LtcOutputs::Get()->ShowBPM(nBPM);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    __DMB();

    if (gv_ltc_bTimeCodeAvailable)
    {
        gv_ltc_bTimeCodeAvailable = false;
        const auto nFps = TimeCodeConst::FPS[g_ltc_LtcTimeCode.type];

        if (direction_)
        {
            g_ltc_LtcTimeCode.frames++;
            if (nFps == g_ltc_LtcTimeCode.frames)
            {
                g_ltc_LtcTimeCode.frames = 0;

                g_ltc_LtcTimeCode.seconds++;
                if (g_ltc_LtcTimeCode.seconds == 60)
                {
                    g_ltc_LtcTimeCode.seconds = 0;

                    g_ltc_LtcTimeCode.minutes++;
                    if (g_ltc_LtcTimeCode.minutes == 60)
                    {
                        g_ltc_LtcTimeCode.minutes = 0;

                        g_ltc_LtcTimeCode.hours++;
                        if (g_ltc_LtcTimeCode.hours == 24)
                        {
                            g_ltc_LtcTimeCode.hours = 0;
                        }
                    }
                }
            }
        }
        else
        {
            if (g_ltc_LtcTimeCode.frames == nFps - 1)
            {
                if (g_ltc_LtcTimeCode.seconds > 0)
                {
                    g_ltc_LtcTimeCode.seconds--;
                }
                else
                {
                    g_ltc_LtcTimeCode.seconds = 59;
                }

                if (g_ltc_LtcTimeCode.seconds == 59)
                {
                    if (g_ltc_LtcTimeCode.minutes > 0)
                    {
                        g_ltc_LtcTimeCode.minutes--;
                    }
                    else
                    {
                        g_ltc_LtcTimeCode.minutes = 59;
                    }

                    if (g_ltc_LtcTimeCode.minutes == 59)
                    {
                        if (g_ltc_LtcTimeCode.hours > 0)
                        {
                            g_ltc_LtcTimeCode.hours--;
                        }
                        else
                        {
                            g_ltc_LtcTimeCode.hours = 23;
                        }
                    }
                }
            }
        }

        Update();
    }

    if (Midi::Get()->GetUpdatesPerSecond() != 0)
    {
        hal::statusled::SetMode(hal::statusled::Mode::DATA);
    }
    else
    {
        hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    }
}
