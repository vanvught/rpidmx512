/**
 * @file midimonitor.h
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "midimonitor.h"

#include "midi.h"
#include "mididescription.h"

#include "hardware.h"
#include "console.h"

#include "h3_hs_timer.h"

using namespace midi;

static char s_aTimecode[] __attribute__ ((aligned (4))) =  "--:--:--;-- -----";
static uint8_t s_Qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

static constexpr auto ROW = 1;
static constexpr auto COLUMN = 80;
static constexpr auto TC_LENGTH = sizeof(s_aTimecode) - 1;
static constexpr char TC_TYPES[4][8] __attribute__ ((aligned (4))) = {"Film " , "EBU  " , "DF   " , "SMPTE" };

inline static void itoa_base10(int nArg, char *pBuffer) {
	auto *p = pBuffer;

	if (nArg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = '0' + (nArg / 10);
	*p = '0' + (nArg % 10);
}

MidiMonitor::MidiMonitor() :
		m_nMillisPrevious(Hardware::Get()->Millis()),
		m_pMidiMessage(const_cast<struct Message *>(Midi::Get()->GetMessage())) {
}

void MidiMonitor::Init() {
	//                                       1         2         3         4
	//                              1234567890123456789012345678901234567890
	constexpr char aHeaderLine[] = "TIMESTAMP ST D1 D2 CHL NOTE EVENT";

	console_set_fg_bg_color(CONSOLE_BLACK,CONSOLE_WHITE);
	console_set_cursor(0, 3);
	console_puts(aHeaderLine);

	for (uint32_t i = sizeof(aHeaderLine); i <= console_get_line_width(); i++) {
		console_putc(' ');
	}

	console_set_fg_bg_color(CONSOLE_WHITE, CONSOLE_BLACK);
	console_set_top_row(4);

	m_nInitTimestamp = h3_hs_timer_lo_us();
}

void MidiMonitor::Update(uint8_t nType) {
	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_write(s_aTimecode, TC_LENGTH);
	console_restore_cursor();

	if (nType != m_nTypePrevious) {
		m_nTypePrevious = nType;
		memcpy(&s_aTimecode[12], TC_TYPES[nType], 5);
	}
}

void MidiMonitor::HandleMtc() {
	const uint8_t type = m_pMidiMessage->aSystemExclusive[5] >> 5;

	itoa_base10((m_pMidiMessage->aSystemExclusive[5] & 0x1F), &s_aTimecode[0]);
	itoa_base10(m_pMidiMessage->aSystemExclusive[6], &s_aTimecode[3]);
	itoa_base10(m_pMidiMessage->aSystemExclusive[7], &s_aTimecode[6]);
	itoa_base10(m_pMidiMessage->aSystemExclusive[8], &s_aTimecode[9]);

	Update(type);
}

void MidiMonitor::HandleQf() {
	const uint8_t nPart = (m_pMidiMessage->nData1 & 0x70) >> 4;
	const uint8_t nValue = m_pMidiMessage->nData1 & 0x0F;

	s_Qf[nPart] = nValue;

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ((m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0))) {
		itoa_base10(s_Qf[6] | ((s_Qf[7] & 0x1) << 4) , &s_aTimecode[0]);
		itoa_base10(s_Qf[4] | (s_Qf[5] << 4) , &s_aTimecode[3]);
		itoa_base10(s_Qf[2] | (s_Qf[3] << 4) , &s_aTimecode[6]);
		itoa_base10(s_Qf[0] | (s_Qf[1] << 4) , &s_aTimecode[9]);

		const uint8_t nType = s_Qf[7] >> 1;

		Update(nType);
	}

	m_nPartPrevious = nPart;
}

void MidiMonitor::HandleMessage() {
	if (Midi::Get()->Read(static_cast<uint8_t>(Channel::OMNI))) {
		// Handle Active Sensing messages
		if (m_pMidiMessage->tType == Types::ACTIVE_SENSING) {
			// This is handled in ShowActiveSense
			return;
		}

		// Time stamp
		const uint32_t nDeltaUs = m_pMidiMessage->nTimestamp - m_nInitTimestamp;
		uint32_t nTime = nDeltaUs / 1000;
		const uint32_t nHours = nTime / 3600000;
		nTime -= nHours * 3600000;
		const uint32_t nMinutes = nTime /  60000;
		nTime -= nMinutes * 60000;
		const uint32_t nSeconds = nTime / 1000;
		const uint32_t nMillis = nTime - nSeconds * 1000;

		printf("%02d:%02d.%03d ", (nHours * 60) + nMinutes, nSeconds, nMillis);

		console_puthex(static_cast<uint8_t>(m_pMidiMessage->tType));
		console_putc(' ');

		switch (m_pMidiMessage->nBytesCount) {
		case 1:
			console_puts("-- -- ");
			break;
		case 2:
			console_puthex(m_pMidiMessage->nData1);
			console_puts(" -- ");
			break;
		case 3:
			console_puthex(m_pMidiMessage->nData1);
			console_putc(' ');
			console_puthex(m_pMidiMessage->nData2);
			console_putc(' ');
			break;
		default:
			console_puts("-- -- ");
			break;
		}

		if (m_pMidiMessage->nChannel != 0) {
			// Channel messages
			printf("%2d  ", m_pMidiMessage->nChannel);
			if (m_pMidiMessage->tType == Types::NOTE_OFF || m_pMidiMessage->tType == Types::NOTE_ON) {
				console_puts(MidiDescription::GetKeyName(m_pMidiMessage->nData1));
				auto i = strlen(MidiDescription::GetKeyName(m_pMidiMessage->nData1));
				while ((5 - i++) > 0) {
					console_putc(' ');
				}
			}
			else {
				console_puts("---- ");
			}
		} else {
			console_puts("--  ---- ");
		}

		console_puts(MidiDescription::GetType(m_pMidiMessage->tType));

		if (m_pMidiMessage->nChannel != 0) {
			// Channel messages
			switch (m_pMidiMessage->tType) {
			// Channel message
			case Types::NOTE_OFF:
			case Types::NOTE_ON:
				printf(" %d, Velocity %d\n", m_pMidiMessage->nData1, m_pMidiMessage->nData2);
				break;
			case Types::AFTER_TOUCH_POLY:
				printf(" %d, Pressure %d\n", m_pMidiMessage->nData1, m_pMidiMessage->nData2);
				break;
			case Types::CONTROL_CHANGE:
				// https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
				if (m_pMidiMessage->nData1 < 120) {
					// Control Change
					printf(", %s, Value %d\n", MidiDescription::GetControlFunction(static_cast<control::Function>(m_pMidiMessage->nData1)), m_pMidiMessage->nData2);
				} else {
					// Controller numbers 120-127 are reserved for Channel Mode Messages, which rather than controlling sound parameters, affect the channel's operating mode.
					// Channel Mode Messages
					printf(", %s", MidiDescription::GetControlChange(static_cast<control::Change>(m_pMidiMessage->nData1)));

					if (m_pMidiMessage->nData1 == static_cast<uint8_t>(control::Change::LOCAL_CONTROL)) {
						printf(" %s\n", m_pMidiMessage->nData2 == 0 ? "OFF" : "ON");
					} else {
						console_putc('\n');
					}
				}
				break;
			case Types::PROGRAM_CHANGE:
				if (m_pMidiMessage->nChannel == 10) {
					printf(", %s {%d}\n", MidiDescription::GetDrumKitName(m_pMidiMessage->nData1), m_pMidiMessage->nData1);
				} else {
					printf(", %s {%d}\n", MidiDescription::GetInstrumentName(m_pMidiMessage->nData1), m_pMidiMessage->nData1);
				}
				break;
			case Types::AFTER_TOUCH_CHANNEL:
				printf(", Pressure %d\n", m_pMidiMessage->nData1);
				break;
			case Types::PITCH_BEND:
				printf(", Bend %d\n", (m_pMidiMessage->nData1 | (m_pMidiMessage->nData2 << 7)));
				break;
			default:
				break;
			}
		} else {
			switch (m_pMidiMessage->tType) {
			// 1 byte message
			case Types::START:
			case Types::CONTINUE:
			case Types::STOP:
			case Types::CLOCK:
			case Types::ACTIVE_SENSING:
			case Types::SYSTEM_RESET:
			case Types::TUNE_REQUEST:
				console_putc('\n');
				break;
				// 2 bytes messages
			case Types::TIME_CODE_QUARTER_FRAME:
				printf(", Message number %d, Data %d\n", ((m_pMidiMessage->nData1 & 0x70) >> 4), (m_pMidiMessage->nData1 & 0x0F));
				HandleQf();
				break;
			case Types::SONG_SELECT:
				printf(", Song id number %d\n", m_pMidiMessage->nData1);
				break;
				// 3 bytes messages
			case Types::SONG_POSITION:
				printf(", Song position %d\n", (m_pMidiMessage->nData1 | (m_pMidiMessage->nData2 << 7)));
				break;
				// > 3 bytes messages
			case Types::SYSTEM_EXCLUSIVE:
				printf(", [%d] ", m_pMidiMessage->nBytesCount);
				{
					uint8_t c;
					for (c = 0; c < std::min(m_pMidiMessage->nBytesCount, static_cast<uint8_t>(16)); c++) {
						console_puthex(m_pMidiMessage->aSystemExclusive[c]);
						console_putc(' ');
					}
					if (c < m_pMidiMessage->nBytesCount) {
						console_puts("..");
					}
				}
				console_putc('\n');
				if ((m_pMidiMessage->aSystemExclusive[1] == 0x7F) && (m_pMidiMessage->aSystemExclusive[2] == 0x7F) && (m_pMidiMessage->aSystemExclusive[3] == 0x01)) {
					HandleMtc();
				}
				break;
			case Types::INVALIDE_TYPE:
			default:
				console_puts(", Invalid MIDI message\n");
				break;
			}
		}
	}
}

void MidiMonitor::ShowActiveSense() {
	const auto nNow = Hardware::Get()->Millis();

	if (__builtin_expect(((nNow - m_nMillisPrevious) < 1000), 0)) {
		return;
	}

	m_nMillisPrevious = nNow;

	const auto tState = Midi::Get()->GetActiveSenseState();

	if (tState == ActiveSenseState::ENABLED) {
		console_save_cursor();
		console_set_cursor(70, 3);
		console_set_fg_bg_color(CONSOLE_BLACK, CONSOLE_CYAN);
		console_puts("ACTIVE SENSING          ");
		console_restore_cursor();
	} else if (tState == ActiveSenseState::FAILED) {
		console_save_cursor();
		console_set_cursor(70, 3);
		console_set_fg_bg_color(CONSOLE_RED, CONSOLE_WHITE);
		console_puts("ACTIVE SENSING - Failed!");
		console_restore_cursor();
	}
}

void MidiMonitor::Run() {
	HandleMessage();
	ShowActiveSense();
}
