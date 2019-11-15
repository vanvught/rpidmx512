/**
 * @file rtpmidireader.cpp
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
#include <string.h>
#include <assert.h>

#include "h3/rtpmidireader.h"

#include "c/led.h"

#include "arm/synchronize.h"
#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Output
#include "artnetnode.h"
#include "midi.h"
#include "h3/ltcsender.h"
#include "displaymax7219.h"
//
#include "h3/ltcoutputs.h"

// IRQ Timer0
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static uint8_t qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

static void irq_timer0_update_handler(uint32_t clo) {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

inline static void itoa_base10(uint32_t arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = (char) ('0' + (arg / 10));
	*n = (char) ('0' + (arg % 10));
}

RtpMidiHandler::~RtpMidiHandler(void) {

}

RtpMidiReader::RtpMidiReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_nTimeCodeType(MIDI_TC_TYPE_UNKNOWN),
	m_nPartPrevious(0),
	m_bDirection(true)
{
	assert(m_ptLtcDisabledOutputs != 0);

	Ltc::InitTimeCode(m_aTimeCode);
}

RtpMidiReader::~RtpMidiReader(void) {
	Stop();
}

void RtpMidiReader::Start(void) {
	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, (thunk_irq_timer_t) irq_timer0_update_handler);
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	LtcOutputs::Get()->Init();

	led_set_ticks_per_second(LED_TICKS_NO_DATA);
}

void RtpMidiReader::Stop(void) {
	irq_timer_set(IRQ_TIMER_0, 0);
}

void RtpMidiReader::MidiMessage(const struct _midi_message *ptMidiMessage) {
	const uint8_t *pSystemExclusive = ptMidiMessage->system_exclusive;

	bool isMtc = false;

	if (ptMidiMessage->type == MIDI_TYPES_TIME_CODE_QUARTER_FRAME) {
		HandleMtcQf(ptMidiMessage);
		isMtc = true;
	} else if (ptMidiMessage->type == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
		if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
			HandleMtc(ptMidiMessage);
			isMtc = true;
		}
	}

	if (isMtc) {
		nUpdates++;
	}
}

void RtpMidiReader::HandleMtc(const struct _midi_message *ptMidiMessage) {
	const uint8_t *pSystemExclusive =ptMidiMessage->system_exclusive;

	m_nTimeCodeType = (_midi_timecode_type) (pSystemExclusive[5] >> 5);

	itoa_base10((pSystemExclusive[5] & 0x1F), (char *) &m_aTimeCode[0]);
	itoa_base10(pSystemExclusive[6], (char *) &m_aTimeCode[3]);
	itoa_base10(pSystemExclusive[7], (char *) &m_aTimeCode[6]);
	itoa_base10(pSystemExclusive[8], (char *) &m_aTimeCode[9]);

	m_tLtcTimeCode.nFrames = pSystemExclusive[8];
	m_tLtcTimeCode.nSeconds = pSystemExclusive[7];
	m_tLtcTimeCode.nMinutes = pSystemExclusive[6];
	m_tLtcTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	m_tLtcTimeCode.nType = m_nTimeCodeType;

	Update(ptMidiMessage);
}

void RtpMidiReader::HandleMtcQf(const struct _midi_message *ptMidiMessage) {
	const uint8_t nData1 = ptMidiMessage->data1;
	const uint8_t nPart = (nData1 & 0x70) >> 4;

	qf[nPart] = nData1 & 0x0F;

	m_nTimeCodeType = (_midi_timecode_type) (qf[7] >> 1);

	if (!m_ptLtcDisabledOutputs->bMidi) {
		midi_send_qf(ptMidiMessage->data1);
	}

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ( (m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0)) ) {
		itoa_base10(qf[6] | ((qf[7] & 0x1) << 4), (char *) &m_aTimeCode[0]);
		itoa_base10(qf[4] | (qf[5] << 4), (char *) &m_aTimeCode[3]);
		itoa_base10(qf[2] | (qf[3] << 4), (char *) &m_aTimeCode[6]);
		itoa_base10(qf[0] | (qf[1] << 4), (char *) &m_aTimeCode[9]);

		m_tLtcTimeCode.nFrames = qf[0] | (qf[1] << 4);
		m_tLtcTimeCode.nSeconds = qf[2] | (qf[3] << 4);
		m_tLtcTimeCode.nMinutes = qf[4] | (qf[5] << 4);
		m_tLtcTimeCode.nHours = qf[6] | ((qf[7] & 0x1) << 4);
		m_tLtcTimeCode.nType = m_nTimeCodeType;

		Update(ptMidiMessage);
	}

	m_nPartPrevious = nPart;
}

void RtpMidiReader::Update(const struct _midi_message *ptMidiMessage) {
	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *)&m_tLtcTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bArtNet) {
		ArtNetNode::Get()->SendTimeCode((struct TArtNetTimeCode *) &m_tLtcTimeCode);
	}

	LtcOutputs::Get()->Update((const struct TLtcTimeCode *)&m_tLtcTimeCode);
}

void RtpMidiReader::Run(void) {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage((const struct TLtcTimeCode *)&m_tLtcTimeCode);

	dmb();
	if (nUpdatesPerSecond >= 24) {
		led_set_ticks_per_second(LED_TICKS_DATA);
	} else {
		DisplayMax7219::Get()->ShowSysTime();
		LtcOutputs::Get()->ResetTimeCodeTypePrevious();
		led_set_ticks_per_second(LED_TICKS_NO_DATA);
	}
}
