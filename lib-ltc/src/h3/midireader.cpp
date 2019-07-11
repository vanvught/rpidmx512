/**
 * @file midireader.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "h3/midireader.h"
#include "ltc.h"

#include "hardwarebaremetal.h"

#include "arm/synchronize.h"
#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

#ifndef NDEBUG
 #include "console.h"
#endif

// Input
#include "midi.h"

// Output
#include "ltcleds.h"
#include "artnetnode.h"
#include "display.h"
#include "displaymax7219.h"
#include "ntpserver.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static volatile uint32_t nLedToggle = 0;

static uint8_t qf[8] ALIGNED = { 0, 0, 0, 0, 0, 0, 0, 0 };	///<

static void itoa_base10(uint32_t arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = (char) ('0' + (arg / 10));
	*n = (char) ('0' + (arg % 10));
}

static void irq_timer0_update_handler(uint32_t clo) {
	dmb();
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;

	if ((nUpdatesPerSecond >= 24) && (nUpdatesPerSecond <= 30)) {
		if (nLedToggle++ & 0x01) {
			Hardware::Get()->SetLed(HARDWARE_LED_ON);
		} else {
			Hardware::Get()->SetLed(HARDWARE_LED_OFF);
		}
	} else {
		Hardware::Get()->SetLed(HARDWARE_LED_ON);
	}
}

MidiReader::MidiReader(ArtNetNode* pNode, struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_pNode(pNode),
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_tTimeCodeType(MIDI_TC_TYPE_UNKNOWN),
	m_tTimeCodeTypePrevious(MIDI_TC_TYPE_UNKNOWN),
	m_nPartPrevious(0)
{
	for (unsigned i = 0; i < sizeof(m_aTimeCode) / sizeof(m_aTimeCode[0]); i++) {
		m_aTimeCode[i] = ' ';
	}

	m_aTimeCode[2] = ':';
	m_aTimeCode[5] = ':';
	m_aTimeCode[8] = '.';
}

MidiReader::~MidiReader(void) {
}

void MidiReader::Start(void) {
	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, irq_timer0_update_handler);
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	midi_active_set_sense(false); //TODO We do nothing with sense data, yet
	midi_init(MIDI_DIRECTION_INPUT);
}

void MidiReader::Run(void) {
#ifndef NDEBUG
	uint32_t nLimitUs = 0;
	char aLimitWarning[16];
	const uint32_t nNowUs = h3_hs_timer_lo_us();
#endif

	bool isMtc = false;
	uint8_t nSystemExclusiveLength;
	const uint8_t *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

	if (midi_read_channel(MIDI_CHANNEL_OMNI)) {
		if (Midi::Get()->GetChannel() == 0) {
			switch (Midi::Get()->GetMessageType()) {
			case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
				HandleMtcQf(); // type = midi_reader_mtc_qf(midi_message);
				isMtc = true;
				break;
			case MIDI_TYPES_SYSTEM_EXCLUSIVE:
				if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
					HandleMtc(); // type = midi_reader_mtc(midi_message);
					isMtc = true;
				}
				break;
			default:
				break;
			}
		}
	}

	if (isMtc) {
		nUpdates++;

#ifndef NDEBUG
		switch (m_tTimeCodeType) {
		case TC_TYPE_FILM:
			nLimitUs = (uint32_t) ((double) 1000000 / (double) 24);
			break;
		case TC_TYPE_EBU:
			nLimitUs = (uint32_t) ((double) 1000000 / (double) 25);
			break;
		case TC_TYPE_DF:
		case TC_TYPE_SMPTE:
			nLimitUs = (uint32_t) ((double) 1000000 / (double) 30);
			break;
		default:
			nLimitUs = 0;
			break;
		}

		const uint32_t nDeltaUs = h3_hs_timer_lo_us() - nNowUs;

		if (nLimitUs == 0) {
			sprintf(aLimitWarning, "%.2d:-----:%.5d", (int) nUpdatesPerSecond, (int) nDeltaUs);
			console_status(CONSOLE_CYAN, aLimitWarning);
		} else {
			sprintf(aLimitWarning, "%.2d:%.5d:%.5d", (int) nUpdatesPerSecond, (int) nLimitUs, (int) nDeltaUs);
			console_status(nDeltaUs < nLimitUs ? CONSOLE_YELLOW : CONSOLE_RED, aLimitWarning);
		}
#endif
	}
}

void MidiReader::HandleMtc(void) {
	uint8_t nSystemExclusiveLength;
	const uint8_t *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

	m_tTimeCodeType = (_midi_timecode_type) (pSystemExclusive[5] >> 5);

	itoa_base10((pSystemExclusive[5] & 0x1F), (char *) &m_aTimeCode[0]);
	itoa_base10(pSystemExclusive[6], (char *) &m_aTimeCode[3]);
	itoa_base10(pSystemExclusive[7], (char *) &m_aTimeCode[6]);
	itoa_base10(pSystemExclusive[8], (char *) &m_aTimeCode[9]);

	m_MidiTimeCode.hour = pSystemExclusive[5] & 0x1F;
	m_MidiTimeCode.minute = pSystemExclusive[6];
	m_MidiTimeCode.second = pSystemExclusive[7];
	m_MidiTimeCode.frame = pSystemExclusive[8];
	m_MidiTimeCode.rate = m_tTimeCodeType;

	Update();
}

void MidiReader::HandleMtcQf(void) {
	uint8_t nData1, nData2;

	Midi::Get()->GetMessageData(nData1, nData2);

	const uint8_t nPart = (nData1 & 0x70) >> 4;

	qf[nPart] = nData1 & 0x0F;

	m_tTimeCodeType = (_midi_timecode_type) (qf[7] >> 1);

	if (((nPart == 7) && (m_nPartPrevious == 6)) || ((nPart == 0) && (m_nPartPrevious == 7))) {
		itoa_base10(qf[6] | ((qf[7] & 0x1) << 4), (char *) &m_aTimeCode[0]);
		itoa_base10(qf[4] | (qf[5] << 4), (char *) &m_aTimeCode[3]);
		itoa_base10(qf[2] | (qf[3] << 4), (char *) &m_aTimeCode[6]);
		itoa_base10(qf[0] | (qf[1] << 4), (char *) &m_aTimeCode[9]);

		Update();
	}
}

void MidiReader::Update(void) {
	if (!m_ptLtcDisabledOutputs->bArtNet) {
		m_pNode->SendTimeCode((struct TArtNetTimeCode *) &m_MidiTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bNtp) {
		NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *) &m_MidiTimeCode);
	}

	if (m_tTimeCodeType != m_tTimeCodeTypePrevious) {
		m_tTimeCodeTypePrevious = m_tTimeCodeType;

		if (!m_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(2, (char *) Ltc::GetType((TTimecodeTypes) m_tTimeCodeType), TC_TYPE_MAX_LENGTH);
		}
		LtcLeds::Get()->Show((TTimecodeTypes) m_tTimeCodeType);
	}

	if (!m_ptLtcDisabledOutputs->bDisplay) {
		Display::Get()->TextLine(1, (const char *) m_aTimeCode, TC_CODE_MAX_LENGTH);
	}
	if (!m_ptLtcDisabledOutputs->bMax7219) {
		DisplayMax7219::Get()->Show((const char *) m_aTimeCode);
	}
}
