/**
 * @file ltcoutputs.cpp
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

#if defined(DEBUG_ARM_LTCOUTPUTS)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <cassert>

#include "arm/ltcoutputs.h"
#include "ltc.h"
#include "timecodeconst.h"
// Outputs
#include "ltcsender.h"
#include "net/rtpmidi.h"
#include "midi.h"
#include "ntpserver.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#if !(defined(CONFIG_LTC_DISABLE_RGB_PANEL) && defined(CONFIG_LTC_DISABLE_WS28XX))
#include "ltcdisplayrgb.h"
#else
#define LTC_NO_DISPLAY_RGB
#endif
 #include "firmware/debug/debug_debug.h"

static volatile bool sv_is_midi_quarter_frame_message;
static volatile uint32_t sv_n_midi_quarter_frame_piece;

static uint8_t CreateQuarterFrame(const struct midi::Timecode* timecode)
{
    uint8_t data = 0;

    switch (sv_n_midi_quarter_frame_piece)
    {
        case 0:
            data = 0x00 | (timecode->frames & 0x0F);
            break;
        case 1:
            data = 0x10 | static_cast<uint8_t>((timecode->frames & 0x10) >> 4);
            break;
        case 2:
            data = 0x20 | (timecode->seconds & 0x0F);
            break;
        case 3:
            data = 0x30 | static_cast<uint8_t>((timecode->seconds & 0x30) >> 4);
            break;
        case 4:
            data = 0x40 | (timecode->minutes & 0x0F);
            break;
        case 5:
            data = 0x50 | static_cast<uint8_t>((timecode->minutes & 0x30) >> 4);
            break;
        case 6:
            data = 0x60 | (timecode->hours & 0x0F);
            break;
        case 7:
            data = static_cast<uint8_t>(0x70 | (timecode->type << 1) | ((timecode->hours & 0x10) >> 4));
            break;
        default:
            break;
    }

    sv_n_midi_quarter_frame_piece = (sv_n_midi_quarter_frame_piece + 1) & 0x07;

    return data;
}

#if defined(H3)
static void IrqTimer1MidiHandler([[maybe_unused]] uint32_t clo)
{
#elif defined(GD32)
extern "C" void TIMER0_TRG_CMT_TIMER10_IRQHandler()
{
    const auto nIntFlag = TIMER_INTF(TIMER10);
    if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP)
    {
#endif
    if ((sv_n_midi_quarter_frame_piece == 0) || (sv_n_midi_quarter_frame_piece == 4))
    {
        return;
    }

    const auto kData = CreateQuarterFrame(reinterpret_cast<const struct midi::Timecode*>(&g_ltc_LtcTimeCode));

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI))
    {
        RtpMidi::Get()->SendQf(kData);
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
    {
        Midi::Get()->SendQf(kData);
    }

    sv_is_midi_quarter_frame_message = true;
#if defined(GD32)
#if defined(DEBUG_LTC_TIMER10)
    GPIO_TG(DEBUG_TIMER10_GPIOx) = DEBUG_TIMER10_GPIO_PINx;
#endif
}

timer_interrupt_flag_clear(TIMER10, nIntFlag);
#endif
}

LtcOutputs::LtcOutputs(ltc::Source source, bool show_systime) : m_bShowSysTime(show_systime)
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    memset(m_cBPM, 0, sizeof(m_cBPM));
    memcpy(m_cBPM, "BPM: ", 5);

    // Disable source as output
    ltc::Destination::SetDisabled(ltc::Destination::Output::MIDI, source == ltc::Source::MIDI);
    ltc::Destination::SetDisabled(ltc::Destination::Output::ARTNET, source == ltc::Source::ARTNET);
    ltc::Destination::SetDisabled(ltc::Destination::Output::LTC, source == ltc::Source::LTC);
    ltc::Destination::SetDisabled(ltc::Destination::Output::RTPMIDI, source == ltc::Source::APPLEMIDI);
    ltc::Destination::SetDisabled(ltc::Destination::Output::ETC, source == ltc::Source::ETC);
    // Display's
    ltc::Destination::SetDisabled(ltc::Destination::Output::RGBPANEL, (source == ltc::Source::LTC) || (source == ltc::Source::MIDI));
    ltc::Destination::SetDisabled(ltc::Destination::Output::MAX7219, ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) ||
                                                                         ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL));
    ltc::Destination::SetDisabled(ltc::Destination::Output::DISPLAY_OLED, ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL));
    // Do not change the order
    ltc::Destination::SetDisabled(ltc::Destination::Output::MIDI, ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL));
    ltc::Destination::SetDisabled(ltc::Destination::Output::LTC, ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL));

    ltc::init_timecode(timecode_);
    ltc::init_systemtime(m_aSystemTime);

    DEBUG_EXIT();
}

void LtcOutputs::Init(bool disable_rtp_midi)
{
    DEBUG_ENTRY();

    m_bEnableRtpMidi = ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI) && !disable_rtp_midi;
    type_previous_ = ltc::Type::INVALID;
    m_bMidiQuarterFramePieceRunning = false;
    sv_n_midi_quarter_frame_piece = 0;

    if (m_bEnableRtpMidi || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
    {
#if defined(H3)
        irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(IrqTimer1MidiHandler));
#elif defined(GD32)
            platform::ltc::timer10_config();
#endif
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
    {
        Display::Get()->TextLine(2, ltc::get_type(ltc::Type::UNKNOWN), ltc::timecode::TYPE_MAX_LENGTH);
    }

    DEBUG_EXIT();
}

void LtcOutputs::Update(const struct ltc::TimeCode* timecode)
{
    assert(timecode != nullptr);

    if (timecode->type != static_cast<uint8_t>(type_previous_))
    {
        DEBUG_PRINTF("pLtcTimeCode->type=%u, m_TypePrevious=%u", timecode->type, type_previous_);
        type_previous_ = static_cast<ltc::Type>(timecode->type);

#if defined(H3)
        H3_TIMER->TMR1_CTRL &= ~TIMER_CTRL_EN_START;
#elif defined(GD32)
            TIMER_CTL0(TIMER10) &= ~TIMER_CTL0_CEN;
#endif

        sv_is_midi_quarter_frame_message = false;
        m_bMidiQuarterFramePieceRunning = false;
        sv_n_midi_quarter_frame_piece = 0;

        if (m_bEnableRtpMidi)
        {
            RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode*>(timecode));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
        {
            Midi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode*>(timecode));
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
        {
            Display::Get()->TextLine(2, ltc::get_type(static_cast<ltc::Type>(timecode->type)), ltc::timecode::TYPE_MAX_LENGTH);
        }

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            LtcDisplayRgb::Get()->ShowFPS(static_cast<ltc::Type>(timecode->type));
        }
#endif

        timecode_[ltc::timecode::index::COLON_3] = (timecode->type != static_cast<uint8_t>(ltc::Type::DF) ? ':' : ';');
    }

    if (m_bMidiQuarterFramePieceRunning)
    {
#if defined(H3)
        H3_TIMER->TMR1_CTRL &= ~TIMER_CTRL_EN_START;
        H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[timecode->type] / 4;
#elif defined(GD32)
            TIMER_CTL0(TIMER10) &= ~TIMER_CTL0_CEN;
            TIMER_CAR(TIMER10) = TimeCodeConst::TMR_INTV[timecode->type] / 4;
#endif

        __DMB();
        if (!((sv_n_midi_quarter_frame_piece == 0) || (sv_n_midi_quarter_frame_piece == 4)))
        {
            sv_n_midi_quarter_frame_piece = 0;
        }

        const auto kData = CreateQuarterFrame(reinterpret_cast<const struct midi::Timecode*>(timecode));

        if (m_bEnableRtpMidi)
        {
            RtpMidi::Get()->SendQf(kData);
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
        {
            Midi::Get()->SendQf(kData);
        }

#if defined(H3)
        H3_TIMER->TMR1_CUR = 0;
        H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined(GD32)
            TIMER_CNT(TIMER10) = 0;
            TIMER_CTL0(TIMER10) |= TIMER_CTL0_CEN;
#endif
        sv_is_midi_quarter_frame_message = false;
        __DMB();
    }

    m_bMidiQuarterFramePieceRunning = (m_bEnableRtpMidi || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI));

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::NTP_SERVER))
    {
        NtpServer::Get()->SetTimeCode(timecode);
    }

    ltc::itoa_base10(timecode, timecode_);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
    {
        Display::Get()->TextLine(1, timecode_, ltc::timecode::CODE_MAX_LENGTH);
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        LtcDisplayMax7219::Get()->Show(timecode_);
    }

#if !defined(LTC_NO_DISPLAY_RGB)
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->Show(timecode_);
    }
#endif
}

void LtcOutputs::ShowSysTime()
{
    if (m_bShowSysTime)
    {
        const auto kTime = time(nullptr);
        const auto* const kLocaltime = localtime(&kTime);

        if (__builtin_expect((seconds_previous_ == kLocaltime->tm_sec), 1))
        {
            return;
        }

        seconds_previous_ = kLocaltime->tm_sec;

        ltc::itoa_base10(kLocaltime, m_aSystemTime);

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
        {
            Display::Get()->TextLine(1, m_aSystemTime, ltc::timecode::SYSTIME_MAX_LENGTH);
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
        {
            LtcDisplayMax7219::Get()->ShowSysTime(m_aSystemTime);
        }

#if !defined(LTC_NO_DISPLAY_RGB)
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            LtcDisplayRgb::Get()->ShowSysTime(m_aSystemTime);
        }
#endif
        ResetTimeCodeTypePrevious();
    }
}

void LtcOutputs::ShowBPM(uint32_t bpm)
{
    if ((bpm < midi::bpm::MIN) || (bpm > midi::bpm::MAX))
    {
        m_cBPM[5] = '-';
        m_cBPM[6] = '-';
        m_cBPM[7] = '-';
    }
    else
    {
        m_cBPM[7] = static_cast<char>(bpm % 10U + '0');
        bpm /= 10U;
        const uint32_t kDigit1 = bpm % 10U;

        if (bpm != 0)
        {
            m_cBPM[6] = static_cast<char>(kDigit1 + '0');
            bpm /= 10U;
            const uint32_t kDigit2 = bpm % 10U;

            if (bpm != 0)
            {
                m_cBPM[5] = static_cast<char>(kDigit2 + '0');
            }
            else
            {
                m_cBPM[5] = ' ';
            }
        }
        else
        {
            m_cBPM[6] = ' ';
            m_cBPM[5] = ' ';
        }
    }

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
    {
        Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 3U), 1);
        Display::Get()->PutString(&m_cBPM[5]);
    }

#if !defined(LTC_NO_DISPLAY_RGB)
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->ShowInfo(m_cBPM);
    }
#endif
}

static void PrintDisabled(ltc::Destination::Output output)
{
    if (ltc::Destination::is_disabled(output))
    {
        printf(" %s output is disabled\n", ltc::Destination::GetOutputString(output));
    }
}

void LtcOutputs::Print()
{
    PrintDisabled(ltc::Destination::Output::LTC);
    PrintDisabled(ltc::Destination::Output::ARTNET);
    PrintDisabled(ltc::Destination::Output::RTPMIDI);
    PrintDisabled(ltc::Destination::Output::MIDI);
    PrintDisabled(ltc::Destination::Output::ETC);
    PrintDisabled(ltc::Destination::Output::NTP_SERVER);
    PrintDisabled(ltc::Destination::Output::DISPLAY_OLED);
    PrintDisabled(ltc::Destination::Output::MAX7219);
    PrintDisabled(ltc::Destination::Output::WS28XX);
    PrintDisabled(ltc::Destination::Output::RGBPANEL);
}
