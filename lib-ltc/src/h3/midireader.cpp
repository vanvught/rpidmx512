/**
 * @file midireader.h
 *
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "h3/midireader.h"
#include "ltc.h"

#include "c/led.h"

#include "arm/synchronize.h"
#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Input
#include "midi.h"

// Output
#include "h3/ltcsender.h"
#include "artnetnode.h"
#include "tcnet.h"
#include "rtpmidi.h"
//
#include "h3/ltcoutputs.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static uint8_t qf[8] ALIGNED = { 0, 0, 0, 0, 0, 0, 0, 0 };	///<

inline static void itoa_base10(uint32_t arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = '0' + (arg / 10);
	*n = '0' + (arg % 10);
}

MidiReader::MidiReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_nTimeCodeType(MIDI_TC_TYPE_UNKNOWN),
	m_nPartPrevious(0),
	m_bDirection(true)
{
	assert(m_ptLtcDisabledOutputs != 0);

	Ltc::InitTimeCode(m_aTimeCode);
}

MidiReader::~MidiReader(void) {
}

void MidiReader::Start(void) {
	midi_active_set_sense(false); //TODO We do nothing with sense data, yet
	midi_init(MIDI_DIRECTION_INPUT);
}

void MidiReader::HandleMtc(void) {
	uint8_t nSystemExclusiveLength;
	const uint8_t *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

	m_nTimeCodeType = static_cast<_midi_timecode_type>((pSystemExclusive[5] >> 5));

	itoa_base10((pSystemExclusive[5] & 0x1F), &m_aTimeCode[0]);
	itoa_base10(pSystemExclusive[6], &m_aTimeCode[3]);
	itoa_base10(pSystemExclusive[7], &m_aTimeCode[6]);
	itoa_base10(pSystemExclusive[8], &m_aTimeCode[9]);

	m_MidiTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	m_MidiTimeCode.nMinutes = pSystemExclusive[6];
	m_MidiTimeCode.nSeconds = pSystemExclusive[7];
	m_MidiTimeCode.nFrames = pSystemExclusive[8];
	m_MidiTimeCode.nType = m_nTimeCodeType;

	Update();
}

void MidiReader::HandleMtcQf(void) {
	uint8_t nData1, nData2;

	Midi::Get()->GetMessageData(nData1, nData2);

	const uint8_t nPart = (nData1 & 0x70) >> 4;

	qf[nPart] = nData1 & 0x0F;

	m_nTimeCodeType = static_cast<_midi_timecode_type>((qf[7] >> 1));

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ((m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0))) {
		itoa_base10(qf[6] | ((qf[7] & 0x1) << 4), &m_aTimeCode[0]);
		itoa_base10(qf[4] | (qf[5] << 4), &m_aTimeCode[3]);
		itoa_base10(qf[2] | (qf[3] << 4), &m_aTimeCode[6]);
		itoa_base10(qf[0] | (qf[1] << 4), &m_aTimeCode[9]);

		m_MidiTimeCode.nHours = qf[6] | ((qf[7] & 0x1) << 4);
		m_MidiTimeCode.nMinutes = qf[4] | (qf[5] << 4);
		m_MidiTimeCode.nSeconds = qf[2] | (qf[3] << 4);
		m_MidiTimeCode.nFrames = qf[0] | (qf[1] << 4);
		m_MidiTimeCode.nType = m_nTimeCodeType;

		Update();
	}

	m_nPartPrevious = nPart;
}

void MidiReader::Update(void) {
	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(&m_MidiTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&m_MidiTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(&m_MidiTimeCode);
	}

	LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&m_MidiTimeCode));
}

void MidiReader::Run(void) {
	uint8_t nSystemExclusiveLength;
	const uint8_t *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

	if (Midi::Get()->Read(MIDI_CHANNEL_OMNI)) {
		if (Midi::Get()->GetChannel() == 0) {
			switch (Midi::Get()->GetMessageType()) {
			case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
				HandleMtcQf(); // type = midi_reader_mtc_qf(midi_message);
				break;
			case MIDI_TYPES_SYSTEM_EXCLUSIVE:
				if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
					HandleMtc(); // type = midi_reader_mtc(midi_message);
				}
				break;
			default:
				break;
			}
		}
	}

	if (Midi::Get()->GetUpdatesPerSeconde() != 0)  {
		led_set_ticks_per_second(LED_TICKS_DATA);
	} else {
		led_set_ticks_per_second(LED_TICKS_NO_DATA);
	}
}
