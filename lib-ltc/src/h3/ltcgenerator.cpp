/**
 * @file ltcgenerator.cpp
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
#include <stdio.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "h3/ltcgenerator.h"
#include "ltc.h"

#include "network.h"

#include "hardwarebaremetal.h"

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

#ifndef NDEBUG
 #include "console.h"
#endif

// Output
#include "ltcleds.h"
#include "display.h"
#include "displaymax7219.h"
#include "artnetnode.h"
#include "midi.h"
#include "h3/ltcsender.h"
#include "ntpserver.h"

#include "debug.h"

static const char sStart[] ALIGNED = "start";
#define START_LENGTH (sizeof(sStart)/sizeof(sStart[0]) - 1)

static const char sStop[] ALIGNED = "stop";
#define STOP_LENGTH (sizeof(sStop)/sizeof(sStop[0]) - 1)

static const char sResume[] ALIGNED = "resume";
#define RESUME_LENGTH (sizeof(sResume)/sizeof(sResume[0]) - 1)

enum tUdpPort {
	UDP_PORT = 0x5443
};

static ArtNetNode* s_pNode;
static struct TLtcDisabledOutputs* s_ptLtcDisabledOutputs;

static 	struct TLtcTimeCode s_tLtcTimeCode;
static volatile bool bTimeCodeAvailable;

static volatile uint32_t nMidiQuarterFramePiece;
static volatile uint32_t nMidiQuarterFrameUs12;
static volatile bool IsMidiQuarterFrameMessage;

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

static void irq_timer0_handler(uint32_t clo) {
	if (!s_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *) &s_tLtcTimeCode, false);
	}

	if (!s_ptLtcDisabledOutputs->bArtNet) {
		s_pNode->SendTimeCode((const struct TArtNetTimeCode *) &s_tLtcTimeCode);
	}

	bTimeCodeAvailable = true;
}

static void irq_timer1_midi_handler(uint32_t clo) {
	H3_TIMER->TMR1_INTV = nMidiQuarterFrameUs12;
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	IsMidiQuarterFrameMessage = true;
}

LtcGenerator::LtcGenerator(ArtNetNode* pNode, const struct TLtcTimeCode* pStartLtcTimeCode, const struct TLtcTimeCode* pStopLtcTimeCode, struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_pStartLtcTimeCode((struct TLtcTimeCode *)pStartLtcTimeCode),
	m_pStopLtcTimeCode((struct TLtcTimeCode *)pStopLtcTimeCode),
	m_nFps(0),
	m_nTimer0Interval(0),
	m_nHandle(-1),
	m_nBytesReceived(0),
	m_bIncrement(false)
{
	assert(pNode != 0);
	assert(pStartLtcTimeCode != 0);
	assert(pStopLtcTimeCode != 0);
	assert(pLtcDisabledOutputs != 0);

	s_pNode = pNode;
	s_ptLtcDisabledOutputs = pLtcDisabledOutputs;

	bTimeCodeAvailable = false;
	IsMidiQuarterFrameMessage = false;

	for (uint32_t i = 0; i < sizeof(m_aTimeCode) / sizeof(m_aTimeCode[0]) ; i++) {
		m_aTimeCode[i] = ' ';
	}

	m_aTimeCode[2] = ':';
	m_aTimeCode[5] = ':';
	m_aTimeCode[8] = '.';

	switch (pStartLtcTimeCode->nType) {
	case TC_TYPE_FILM:
		m_nFps = 24;
		m_nTimer0Interval = 12000000 / 24;
		break;
	case TC_TYPE_EBU:
		m_nFps = 25;
		m_nTimer0Interval = 12000000 / 25;
		break;
	case TC_TYPE_DF:
	case TC_TYPE_SMPTE:
		m_nFps = 30;
		m_nTimer0Interval = 12000000 / 30;
		break;
	default:
		assert(0);
		break;
	}

	nMidiQuarterFrameUs12 = m_nTimer0Interval / 4;
}

LtcGenerator::~LtcGenerator(void) {
	Stop();
}

void LtcGenerator::Start(void) {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);

	irq_timer_init();
	__disable_irq();

	irq_timer_set(IRQ_TIMER_0, irq_timer0_handler);

	if (!s_ptLtcDisabledOutputs->bMidi) {
		irq_timer_set(IRQ_TIMER_1, irq_timer1_midi_handler);
		H3_TIMER->TMR1_CTRL |= TIMER_CTRL_SINGLE_MODE;
	}

	LtcLeds::Get()->Show((TTimecodeTypes) m_pStartLtcTimeCode->nType);

	DEBUG_EXIT
}

void LtcGenerator::Stop(void) {
	DEBUG_ENTRY

	__disable_irq();
	irq_timer_set(IRQ_TIMER_0, 0);

	if (!s_ptLtcDisabledOutputs->bMidi) {
		irq_timer_set(IRQ_TIMER_1, 0);
	}

	m_nHandle = Network::Get()->End(UDP_PORT);

	DEBUG_EXIT
}

void LtcGenerator::ActionStart(void) {
	DEBUG_ENTRY

	memcpy((void *)&s_tLtcTimeCode, m_pStartLtcTimeCode, sizeof(struct TLtcTimeCode));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	if (!s_ptLtcDisabledOutputs->bMidi) {
		Midi::Get()->SendTimeCode((struct _midi_send_tc *) &s_tLtcTimeCode);

		H3_TIMER->TMR1_INTV = nMidiQuarterFrameUs12;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		nMidiQuarterFramePiece = 0;
	}

	m_bIncrement = true;

	__enable_irq();

	if (!s_ptLtcDisabledOutputs->bDisplay) {
		Display::Get()->TextLine(2, (const char *) Ltc::GetType((TTimecodeTypes) m_pStartLtcTimeCode->nType), TC_TYPE_MAX_LENGTH);
	}

	DEBUG_EXIT
}

void LtcGenerator::ActionStop(void) {
	DEBUG_ENTRY

	m_bIncrement = false;

	DEBUG_EXIT
}

void LtcGenerator::ActionResume(void) {
	DEBUG_ENTRY

	m_bIncrement = true;

	DEBUG_EXIT
}

void LtcGenerator::HandleAction(void) {
	DEBUG_ENTRY

	if ((m_nBytesReceived == (4 + START_LENGTH)) && (memcmp(&m_Buffer[4], sStart, START_LENGTH) == 0)) {
		ActionStart();
	} else if ((m_nBytesReceived == (4 + STOP_LENGTH)) && (memcmp(&m_Buffer[4], sStop, STOP_LENGTH) == 0)) {
		ActionStop();
	} else if ((m_nBytesReceived == (4 + RESUME_LENGTH)) && (memcmp(&m_Buffer[4], sResume, RESUME_LENGTH) == 0)) {
		ActionResume();
	} else {
		DEBUG_PUTS("Invalid command");
	}

	DEBUG_EXIT
}

void LtcGenerator::Increment(void) {

	if (__builtin_expect((memcmp(&s_tLtcTimeCode, m_pStopLtcTimeCode, sizeof(struct TLtcTimeCode)) == 0), 0)) {
		return;
	}

	if (__builtin_expect((!m_bIncrement), 0)) {
		return;
	}

	s_tLtcTimeCode.nFrames++;
	if (m_nFps == s_tLtcTimeCode.nFrames) {
		s_tLtcTimeCode.nFrames = 0;

		s_tLtcTimeCode.nSeconds++;
		if (s_tLtcTimeCode.nSeconds == 60) {
			s_tLtcTimeCode.nSeconds = 0;

			s_tLtcTimeCode.nMinutes++;
			if (s_tLtcTimeCode.nMinutes == 60) {
				s_tLtcTimeCode.nMinutes = 0;

				s_tLtcTimeCode.nHours++;
				if (s_tLtcTimeCode.nHours == 24) {
					s_tLtcTimeCode.nHours = 0;
				}
			}
		}
	}
}

void LtcGenerator::Update(void) {
	if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
		dmb();
		IsMidiQuarterFrameMessage = false;

		uint8_t bytes[2] = { 0xF1, 0x00 };
		const uint8_t data = nMidiQuarterFramePiece << 4;

		switch (nMidiQuarterFramePiece) {
		case 0:
			bytes[1] = data | (s_tLtcTimeCode.nFrames & 0x0F);
			break;
		case 1:
			bytes[1] = data | ((s_tLtcTimeCode.nFrames & 0x10) >> 4);
			break;
		case 2:
			bytes[1] = data | (s_tLtcTimeCode.nSeconds & 0x0F);
			break;
		case 3:
			bytes[1] = data | ((s_tLtcTimeCode.nSeconds & 0x30) >> 4);
			break;
		case 4:
			bytes[1] = data | (s_tLtcTimeCode.nMinutes & 0x0F);
			break;
		case 5:
			bytes[1] = data | ((s_tLtcTimeCode.nMinutes & 0x30) >> 4);
			break;
		case 6:
			bytes[1] = data | (s_tLtcTimeCode.nHours & 0x0F);
			break;
		case 7:
			bytes[1] = data | (s_tLtcTimeCode.nType << 1) | ((s_tLtcTimeCode.nHours & 0x10) >> 4);
			break;
		default:
			break;
		}

		Midi::Get()->SendRaw(bytes, 2);
		nMidiQuarterFramePiece = (nMidiQuarterFramePiece + 1) & 0x07;
	}

	dmb();
	if (bTimeCodeAvailable) {
		bTimeCodeAvailable = false;

		if (!s_ptLtcDisabledOutputs->bNtp) {
			NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *) &s_tLtcTimeCode);
		}

		itoa_base10(s_tLtcTimeCode.nHours, (char *) &m_aTimeCode[0]);
		itoa_base10(s_tLtcTimeCode.nMinutes, (char *) &m_aTimeCode[3]);
		itoa_base10(s_tLtcTimeCode.nSeconds, (char *) &m_aTimeCode[6]);
		itoa_base10(s_tLtcTimeCode.nFrames, (char *) &m_aTimeCode[9]);

		if (!s_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(1, (const char *) m_aTimeCode, TC_CODE_MAX_LENGTH);
		}

		if (!s_ptLtcDisabledOutputs->bMax7219) {
			DisplayMax7219::Get()->Show((const char *) m_aTimeCode);
		}

		Increment();
	}
}

void LtcGenerator::Run(void) {
	Update();

	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) &m_Buffer, (uint16_t) sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < (int) 4), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("ltc", m_Buffer, 3) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (m_Buffer[3] == '!') {
		HandleAction();
	} else {
		DEBUG_PUTS("Invalid command");
	}
}
