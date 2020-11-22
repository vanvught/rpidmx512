/**
 * @file systimereader.h
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
#include <string.h>
#include <cassert>

#include "h3/systimereader.h"

#include "ltc.h"
#include "timecodeconst.h"

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
#include "h3/ltcoutputs.h"

#include "debug.h"

namespace cmd {
static constexpr char aStart[] = "start";
static constexpr char aStop[] = "stop";
static constexpr char aRate[] = "rate#";
}

namespace length {
static constexpr auto START = sizeof(cmd::aStart) - 1;
static constexpr auto STOP = sizeof(cmd::aStop) - 1;
static constexpr auto RATE = sizeof(cmd::aRate) - 1;
}

namespace udp {
static constexpr auto PORT = 0x5443;
}

// IRQ Timer0
static volatile bool bTimeCodeAvailable;

static void irq_timer0_handler(__attribute__((unused)) uint32_t clo) {
	bTimeCodeAvailable = true;
}

SystimeReader *SystimeReader::s_pThis = nullptr;

SystimeReader::SystimeReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs, uint8_t nFps) : m_ptLtcDisabledOutputs(pLtcDisabledOutputs), m_nFps(nFps) {
	assert(m_ptLtcDisabledOutputs != nullptr);

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_nTimer0Interval = TimeCodeConst::TMR_INTV[Ltc::GetType(nFps)];
	m_tMidiTimeCode.nType = Ltc::GetType(nFps);
}

void SystimeReader::Start(bool bAutoStart) {
	// UDP Request
	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

	// System Time
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);

	LtcOutputs::Get()->Init();

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);

	if (bAutoStart) {
		ActionStart();
	}
}

void SystimeReader::Print() {
}

void SystimeReader::ActionStart() {
	DEBUG_ENTRY

	if(m_bIsStarted) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted = true;

	LtcOutputs::Get()->ResetTimeCodeTypePrevious();

	LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);

	DEBUG_EXIT
}

void SystimeReader::ActionStop() {
	DEBUG_ENTRY

	m_bIsStarted = false;

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);

	DEBUG_EXIT
}

void SystimeReader::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	ltc::type tType;

	if (Ltc::ParseTimeCodeRate(pTimeCodeRate, nFps, tType)) {
		if (nFps != m_nFps) {
			m_nFps = nFps;
			//
			if (m_tMidiTimeCode.nFrames >= m_nFps) {
				m_tMidiTimeCode.nFrames = m_nFps - 1;
			}
			m_tMidiTimeCode.nType = tType;
			//
			m_nTimer0Interval = TimeCodeConst::TMR_INTV[tType];
			H3_TIMER->TMR0_INTV = m_nTimer0Interval;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			//
			if (!m_ptLtcDisabledOutputs->bLtc) {
				LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode), false);
			}

			if (!m_ptLtcDisabledOutputs->bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&m_tMidiTimeCode));
			}

			if (!m_ptLtcDisabledOutputs->bRtpMidi) {
				RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(&m_tMidiTimeCode));
			}

			LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode));

			DEBUG_PRINTF("nFps=%d", nFps);
		}
	}

	DEBUG_EXIT
}

void SystimeReader::HandleRequest(void *pBuffer, uint32_t nBufferLength) {
	if ((pBuffer != nullptr) && (nBufferLength <= sizeof(m_Buffer))) {
		memcpy(m_Buffer, pBuffer, nBufferLength);
		m_nBytesReceived = nBufferLength;
	}

	if (__builtin_expect((memcmp("ltc!", m_Buffer, 4) != 0), 0)) {
		return;
	}

	if (m_Buffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	debug_dump(m_Buffer, m_nBytesReceived);

	if (m_nBytesReceived == (4 + length::START)) {
		if (memcmp(&m_Buffer[4], cmd::aStart, length::START) == 0) {
			ActionStart();
			return;
		}

		DEBUG_PUTS("Invalid !start command");
	}

	if (m_nBytesReceived == (4 + length::STOP)) {
		if (memcmp(&m_Buffer[4], cmd::aStop, length::STOP) == 0) {
			ActionStop();
			return;
		}

		DEBUG_PUTS("Invalid !stop command");
	}

	if (m_nBytesReceived == (4 + length::RATE  + TC_RATE_MAX_LENGTH)) {
		if (memcmp(&m_Buffer[4], cmd::aRate, length::RATE) == 0) {
			ActionSetRate(&m_Buffer[(4 + length::RATE)]);
			return;
		}
	}

	DEBUG_PUTS("Invalid command");
}

void SystimeReader::HandleUdpRequest() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Buffer, sizeof(m_Buffer), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 8), 1)) {
		return;
	}

	HandleRequest();
}

void SystimeReader::Run() {
	if (m_bIsStarted) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode));

		auto nTime = Hardware::Get()->GetTime();

		if (__builtin_expect((m_ntimePrevious != nTime), 0)) {
			m_ntimePrevious = nTime;

			m_tMidiTimeCode.nFrames = 0;
			m_tMidiTimeCode.nSeconds = nTime % 60;
			nTime /= 60;
			m_tMidiTimeCode.nMinutes = nTime % 60;
			nTime /= 60;
			m_tMidiTimeCode.nHours = nTime % 24;

			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
			bTimeCodeAvailable = true;
		}

		dmb();
		if (__builtin_expect((bTimeCodeAvailable), 0)) {
			bTimeCodeAvailable = false;

			if (!m_ptLtcDisabledOutputs->bLtc) {
				LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode), false);
			}

			if (!m_ptLtcDisabledOutputs->bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&m_tMidiTimeCode));
			}

			if (!m_ptLtcDisabledOutputs->bRtpMidi) {
				RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(&m_tMidiTimeCode));
			}

			LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode));

			m_tMidiTimeCode.nFrames++;
			if (m_nFps == m_tMidiTimeCode.nFrames) {
				m_tMidiTimeCode.nFrames = 0;
			}
		}
	}

	HandleUdpRequest();
}
