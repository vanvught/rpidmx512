/**
 * @file midimonitor.cpp
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

#include <algorithm>
#include <cstdint>
#include <string.h>
#include <cstdio>

#include "midimonitor.h"
#include "midi.h"
#include "mididescription.h"
#include "console.h"
#include "h3_hs_timer.h"

using namespace midi;

static char s_aTimecode[] __attribute__((aligned(4))) = "--:--:--;-- -----";
static uint8_t s_Qf[8] __attribute__((aligned(4))) = {0, 0, 0, 0, 0, 0, 0, 0};

static constexpr auto kTcLength = sizeof(s_aTimecode) - 1;
static constexpr char kTcTypes[4][8] __attribute__((aligned(4))) = {"Film ", "EBU  ", "DF   ", "SMPTE"};

static void Itoa(uint32_t nValue, char* pBuffer)
{
    auto* p = pBuffer;

    if (nValue == 0)
    {
        *p++ = '0';
        *p = '0';
        return;
    }

    *p++ = static_cast<char>('0' + (nValue / 10U));
    *p = static_cast<char>('0' + (nValue % 10U));
}

MidiMonitor::MidiMonitor() : m_nMillisPrevious(H3_TIMER->AVS_CNT0), m_pMidiMessage(const_cast<struct Message*>(Midi::Get()->GetMessage())) {}

void MidiMonitor::Init()
{
    console::SetCursor(70, 1);
    console::Puts("Timecode :");
    console::SetCursor(75, 2);
    console::Puts("BPM :");
    //                                       1         2         3         4
    //                              1234567890123456789012345678901234567890
    constexpr char aHeaderLine[] = "TIMESTAMP ST D1 D2 CHL NOTE EVENT";

    SetFgBgColour(console::Colours::kConsoleBlack, console::Colours::kConsoleWhite);
    console::SetCursor(0, 3);
    console::Puts(aHeaderLine);

    for (uint32_t i = sizeof(aHeaderLine); i <= console::GetLineWidth(); i++)
    {
        console::Putc(' ');
    }

    console::SetFgBgColour(console::Colours::kConsoleWhite, console::Colours::kConsoleBlack);
    console::SetTopRow(4);

    m_nInitTimestamp = H3_TIMER->AVS_CNT0;
}

void MidiMonitor::UpdateTimecode(uint8_t type)
{
    console::SaveCursor();
    console::SetCursor(81, 1);
    console::SetFgColour(console::Colours::kConsoleCyan);
    console::Write(s_aTimecode, kTcLength);
    console::RestoreCursor();

    if (type != type_previous_)
    {
        type_previous_ = type;
        memcpy(&s_aTimecode[12], kTcTypes[type], 5);
    }
}

void MidiMonitor::HandleMtc()
{
    Itoa((m_pMidiMessage->system_exclusive[5] & 0x1F), &s_aTimecode[0]);
    Itoa(m_pMidiMessage->system_exclusive[6], &s_aTimecode[3]);
    Itoa(m_pMidiMessage->system_exclusive[7], &s_aTimecode[6]);
    Itoa(m_pMidiMessage->system_exclusive[8], &s_aTimecode[9]);

    UpdateTimecode(static_cast<uint8_t>(m_pMidiMessage->system_exclusive[5] >> 5));
}

void MidiMonitor::HandleQf()
{
    const uint8_t nPart = (m_pMidiMessage->data1 & 0x70) >> 4;
    const uint8_t nValue = m_pMidiMessage->data1 & 0x0F;

    s_Qf[nPart] = nValue;

    if ((nPart == 7) || (part_previous_ == 7))
    {
    }
    else
    {
        direction_ = (part_previous_ < nPart);
    }

    if ((direction_ && (nPart == 7)) || (!direction_ && (nPart == 0)))
    {
        Itoa(s_Qf[6] | ((s_Qf[7] & 0x1) << 4), &s_aTimecode[0]);
        Itoa(s_Qf[4] | (s_Qf[5] << 4), &s_aTimecode[3]);
        Itoa(s_Qf[2] | (s_Qf[3] << 4), &s_aTimecode[6]);
        Itoa(s_Qf[0] | (s_Qf[1] << 4), &s_aTimecode[9]);

        const auto type = static_cast<uint8_t>(s_Qf[7] >> 1);

        UpdateTimecode(type);
    }

    part_previous_ = nPart;
}

void MidiMonitor::HandleMessage()
{
    if (Midi::Get()->Read(static_cast<uint8_t>(Channel::OMNI)))
    {
        // Handle Active Sensing messages
        if (m_pMidiMessage->type == Types::ACTIVE_SENSING)
        {
            // This is handled in ShowActiveSense
            return;
        }

        // Time stamp
        //		const uint32_t nDeltaUs = m_pMidiMessage->timestamp - m_nInitTimestamp;
        //		uint32_t nTime = nDeltaUs / 1000;
        uint32_t nTime = m_pMidiMessage->timestamp - m_nInitTimestamp;
        const uint32_t hours = nTime / 3600000;
        nTime -= hours * 3600000;
        const uint32_t minutes = nTime / 60000;
        nTime -= minutes * 60000;
        const uint32_t seconds = nTime / 1000;
        const uint32_t millis = nTime - seconds * 1000;

        printf("%02d:%02d.%03d ", (hours * 60) + minutes, seconds, millis);

        console::ConsolePuthex(static_cast<uint8_t>(m_pMidiMessage->type));
        console::Putc(' ');

        switch (m_pMidiMessage->bytes_count)
        {
            case 1:
                console::Puts("-- -- ");
                break;
            case 2:
                console::ConsolePuthex(m_pMidiMessage->data1);
                console::Puts(" -- ");
                break;
            case 3:
                console::ConsolePuthex(m_pMidiMessage->data1);
                console::Putc(' ');
                console::ConsolePuthex(m_pMidiMessage->data2);
                console::Putc(' ');
                break;
            default:
                console::Puts("-- -- ");
                break;
        }

        if (m_pMidiMessage->channel != 0)
        {
            // Channel messages
            printf("%2d  ", m_pMidiMessage->channel);
            if (m_pMidiMessage->type == Types::NOTE_OFF || m_pMidiMessage->type == Types::NOTE_ON)
            {
                console::Puts(MidiDescription::GetKeyName(m_pMidiMessage->data1));
                auto i = strlen(MidiDescription::GetKeyName(m_pMidiMessage->data1));
                while ((5 - i++) > 0)
                {
                    console::Putc(' ');
                }
            }
            else
            {
                console::Puts("---- ");
            }
        }
        else
        {
            console::Puts("--  ---- ");
        }

        console::Puts(MidiDescription::GetType(m_pMidiMessage->type));

        if (m_pMidiMessage->channel != 0)
        {
            // Channel messages
            switch (m_pMidiMessage->type)
            {
                // Channel message
                case Types::NOTE_OFF:
                case Types::NOTE_ON:
                    printf(" %d, Velocity %d\n", m_pMidiMessage->data1, m_pMidiMessage->data2);
                    break;
                case Types::AFTER_TOUCH_POLY:
                    printf(" %d, Pressure %d\n", m_pMidiMessage->data1, m_pMidiMessage->data2);
                    break;
                case Types::CONTROL_CHANGE:
                    // https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
                    if (m_pMidiMessage->data1 < 120)
                    {
                        // Control Change
                        printf(", %s, Value %d\n", MidiDescription::GetControlFunction(static_cast<control::Function>(m_pMidiMessage->data1)),
                               m_pMidiMessage->data2);
                    }
                    else
                    {
                        // Controller numbers 120-127 are reserved for Channel Mode Messages, which rather than controlling sound parameters, affect the
                        // channel's operating mode. Channel Mode Messages
                        printf(", %s", MidiDescription::GetControlChange(static_cast<control::Change>(m_pMidiMessage->data1)));

                        if (m_pMidiMessage->data1 == static_cast<uint8_t>(control::Change::LOCAL_CONTROL))
                        {
                            printf(" %s\n", m_pMidiMessage->data2 == 0 ? "OFF" : "ON");
                        }
                        else
                        {
                            console::Putc('\n');
                        }
                    }
                    break;
                case Types::PROGRAM_CHANGE:
                    if (m_pMidiMessage->channel == 10)
                    {
                        printf(", %s {%d}\n", MidiDescription::GetDrumKitName(m_pMidiMessage->data1), m_pMidiMessage->data1);
                    }
                    else
                    {
                        printf(", %s {%d}\n", MidiDescription::GetInstrumentName(m_pMidiMessage->data1), m_pMidiMessage->data1);
                    }
                    break;
                case Types::AFTER_TOUCH_CHANNEL:
                    printf(", Pressure %d\n", m_pMidiMessage->data1);
                    break;
                case Types::PITCH_BEND:
                    printf(", Bend %d\n", (m_pMidiMessage->data1 | (m_pMidiMessage->data2 << 7)));
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch (m_pMidiMessage->type)
            {
                // 1 byte message
                case Types::CLOCK:
                    if (midi_bpm_.Get(m_pMidiMessage->timestamp * 10, m_nBPM))
                    {
                        console::SaveCursor();
                        console::SetCursor(81, 2);
                        console::SetFgColour(console::Colours::kConsoleCyan);
                        printf("%3d", m_nBPM);
                        console::RestoreCursor();
                    }
                    console::Putc('\n');
                    break;
                case Types::START:
                case Types::CONTINUE:
                case Types::STOP:
                case Types::ACTIVE_SENSING:
                case Types::SYSTEM_RESET:
                case Types::TUNE_REQUEST:
                    console::Putc('\n');
                    break;
                    // 2 bytes messages
                case Types::TIME_CODE_QUARTER_FRAME:
                    printf(", Message number %d, Data %d\n", ((m_pMidiMessage->data1 & 0x70) >> 4), (m_pMidiMessage->data1 & 0x0F));
                    HandleQf();
                    break;
                case Types::SONG_SELECT:
                    printf(", Song id number %d\n", m_pMidiMessage->data1);
                    break;
                    // 3 bytes messages
                case Types::SONG_POSITION:
                    printf(", Song position %d\n", (m_pMidiMessage->data1 | (m_pMidiMessage->data2 << 7)));
                    break;
                    // > 3 bytes messages
                case Types::SYSTEM_EXCLUSIVE:
                    printf(", [%d] ", m_pMidiMessage->bytes_count);
                    {
                        uint8_t c;
                        for (c = 0; c < std::min(m_pMidiMessage->bytes_count, static_cast<uint8_t>(16)); c++)
                        {
                            console::ConsolePuthex(m_pMidiMessage->system_exclusive[c]);
                            console::Putc(' ');
                        }
                        if (c < m_pMidiMessage->bytes_count)
                        {
                            console::Puts("..");
                        }
                    }
                    console::Putc('\n');
                    if ((m_pMidiMessage->system_exclusive[1] == 0x7F) && (m_pMidiMessage->system_exclusive[2] == 0x7F) &&
                        (m_pMidiMessage->system_exclusive[3] == 0x01))
                    {
                        HandleMtc();
                    }
                    break;
                case Types::INVALIDE_TYPE:
                default:
                    console::Puts(", Invalid MIDI message\n");
                    break;
            }
        }
    }
}

void MidiMonitor::ShowActiveSenseAndUpdatesPerSecond()
{
    const auto nNow = H3_TIMER->AVS_CNT0;

    if (__builtin_expect(((nNow - m_nMillisPrevious) < 1000), 0))
    {
        return;
    }

    m_nMillisPrevious = nNow;

    const auto tState = Midi::Get()->GetActiveSenseState();

    if (tState == ActiveSenseState::ENABLED)
    {
        console::SaveCursor();
        console::SetCursor(70, 3);
        console::SetFgBgColour(console::Colours::kConsoleBlack, console::Colours::kConsoleCyan);
        console::Puts("ACTIVE SENSING          ");
        console::RestoreCursor();
    }
    else if (tState == ActiveSenseState::FAILED)
    {
        console::SaveCursor();
        console::SetCursor(70, 3);
        console::SetFgBgColour(console::Colours::kConsoleRed, console::Colours::kConsoleWhite);
        console::Puts("ACTIVE SENSING - Failed!");
        console::RestoreCursor();
    }

    console::SaveCursor();
    console::SetCursor(96, 3);

    const auto nUpdatePerSeconds = Midi::Get()->GetUpdatesPerSecond();
    if (nUpdatePerSeconds != 0)
    {
        console::SetFgBgColour(console::Colours::kConsoleBlack, console::Colours::kConsoleGreen);
        printf("%3d", nUpdatePerSeconds);
    }
    else
    {
        console::SetFgBgColour(console::Colours::kConsoleGreen, console::Colours::kConsoleWhite);
        console::Puts("--");
    }

    console::RestoreCursor();
}
