/**
 * @file systimereader.h
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

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "h3/systimereader.h"

#include "ltc.h"
#include "timecodeconst.h"

#include "c/led.h"

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

#include "hardware.h"
#include "network.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "h3/ltcsender.h"
#include "display.h"
//
#include "h3/ltcoutputs.h"

#include "debug.h"

static const char sStart[] ALIGNED = "start";
#define START_LENGTH (sizeof(sStart)/sizeof(sStart[0]) - 1)

static const char sStop[] ALIGNED = "stop";
#define STOP_LENGTH (sizeof(sStop)/sizeof(sStop[0]) - 1)

static const char sRate[] ALIGNED = "rate";
#define RATE_LENGTH (sizeof(sRate)/sizeof(sRate[0]) - 1)

enum tUdpPort {
	UDP_PORT = 0x5443
};

// IRQ Timer0
static volatile bool bTimeCodeAvailable;

static void irq_timer0_handler(uint32_t clo) {
	bTimeCodeAvailable = true;
}

SystimeReader *SystimeReader::s_pThis = 0;

SystimeReader::SystimeReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs, uint8_t nFps) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_nFps(nFps),
	m_ntimePrevious(0),
	m_nHandle(-1),
	m_nBytesReceived(0),
	m_bIsStarted(false)
{
	assert(m_ptLtcDisabledOutputs != 0);

	s_pThis = this;

	m_nTimer0Interval = TimeCodeConst::TMR_INTV[(int) Ltc::GetType(nFps)];
	m_tMidiTimeCode.nType = (uint8_t) Ltc::GetType(nFps);
}

SystimeReader::~SystimeReader(void) {
}

void SystimeReader::Start(bool bAutoStart) {
	// UDP Request
	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);

	// System Time
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, (thunk_irq_timer_t) irq_timer0_handler);

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);

	LtcOutputs::Get()->Init();

	led_set_ticks_per_second(LED_TICKS_NO_DATA);

	if (bAutoStart) {
		ActionStart();
	}
}

void SystimeReader::Print(void) {
}

void SystimeReader::ActionStart(void) {
	DEBUG_ENTRY

	if(m_bIsStarted) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted = true;

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	led_set_ticks_per_second(LED_TICKS_DATA);

	DEBUG_EXIT
}

void SystimeReader::ActionStop(void) {
	DEBUG_ENTRY

	m_bIsStarted = false;

	led_set_ticks_per_second(LED_TICKS_NO_DATA);

	DEBUG_EXIT
}

void SystimeReader::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	TTimecodeTypes tType;

	if (Ltc::ParseTimeCodeRate(pTimeCodeRate, nFps, tType)) {
		if (nFps != m_nFps) {
			m_nFps = nFps;
			//
			if (m_tMidiTimeCode.nFrames >= m_nFps) {
				m_tMidiTimeCode.nFrames = m_nFps - 1;
			}
			m_tMidiTimeCode.nType = (uint8_t) tType;
			//
			m_nTimer0Interval = TimeCodeConst::TMR_INTV[tType];
			H3_TIMER->TMR0_INTV = m_nTimer0Interval;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			//
			if (!m_ptLtcDisabledOutputs->bLtc) {
				LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *) &m_tMidiTimeCode, false);
			}

			if (!m_ptLtcDisabledOutputs->bArtNet) {
				ArtNetNode::Get()->SendTimeCode((const struct TArtNetTimeCode *) &m_tMidiTimeCode);
			}

			if (!m_ptLtcDisabledOutputs->bRtpMidi) {
				RtpMidi::Get()->SendTimeCode((const struct _midi_send_tc *)&m_tMidiTimeCode);
			}

			LtcOutputs::Get()->Update((const struct TLtcTimeCode *)&m_tMidiTimeCode);

			DEBUG_PRINTF("nFps=%d", nFps);
		}
	}

	DEBUG_EXIT
}

void SystimeReader::HandleUdpRequest(void) {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) &m_Buffer, (uint16_t) sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < (int) 8), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("ltc", m_Buffer, 3) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (m_Buffer[3] != '!') {
		DEBUG_PUTS("Invalid command");
		return;
	}

	if (memcmp(&m_Buffer[4], sStart, START_LENGTH) == 0) {
		if (m_nBytesReceived == (4 + START_LENGTH)) {
			ActionStart();
		} else {
			DEBUG_PUTS("Invalid !start command");
		}
	} else if (memcmp(&m_Buffer[4], sStop, STOP_LENGTH) == 0) {
		if (m_nBytesReceived == (4 + STOP_LENGTH)) {
			ActionStop();
		} else {
			DEBUG_PUTS("Invalid !stop command");
		}
	} else if (memcmp(&m_Buffer[4], sRate, RATE_LENGTH) == 0) {
		if ((m_nBytesReceived == (4 + RATE_LENGTH + 1 + TC_RATE_MAX_LENGTH)) && (m_Buffer[4 + RATE_LENGTH] == '#')) {
			ActionSetRate((const char *)&m_Buffer[(4 + RATE_LENGTH + 1)]);
		}
	} else {
		DEBUG_PUTS("Invalid command");
	}
}

void SystimeReader::Run(void) {
	if (m_bIsStarted) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage((const struct TLtcTimeCode *)&m_tMidiTimeCode);

		time_t ntime = Hardware::Get()->GetTime();

		if (__builtin_expect((m_ntimePrevious != ntime), 0)) {
			m_ntimePrevious = ntime;

			m_tMidiTimeCode.nFrames = 0;
			m_tMidiTimeCode.nSeconds = ntime % 60;
			ntime /= 60;
			m_tMidiTimeCode.nMinutes = ntime % 60;
			ntime /= 60;
			m_tMidiTimeCode.nHours = ntime % 24;

			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			bTimeCodeAvailable = true;
		}

		dmb();
		if (__builtin_expect((bTimeCodeAvailable), 0)) {
			bTimeCodeAvailable = false;

			if (!m_ptLtcDisabledOutputs->bLtc) {
				LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *) &m_tMidiTimeCode, false);
			}

			if (!m_ptLtcDisabledOutputs->bArtNet) {
				ArtNetNode::Get()->SendTimeCode((const struct TArtNetTimeCode *) &m_tMidiTimeCode);
			}

			if (!m_ptLtcDisabledOutputs->bRtpMidi) {
				RtpMidi::Get()->SendTimeCode((const struct _midi_send_tc *)&m_tMidiTimeCode);
			}

			LtcOutputs::Get()->Update((const struct TLtcTimeCode *)&m_tMidiTimeCode);

			m_tMidiTimeCode.nFrames++;
			if (m_nFps == m_tMidiTimeCode.nFrames) {
				m_tMidiTimeCode.nFrames = 0;
			}
		}
	}

	HandleUdpRequest();
}
