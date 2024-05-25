/**
 * @file systimereader.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "systimereader.h"

#include "ltc.h"
#include "timecodeconst.h"

#include "hardware.h"
#include "network.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "display.h"
#include "ltcoutputs.h"

#include "platform_ltc.h"

#include "debug.h"

namespace cmd {
static constexpr char START[] = "start";
static constexpr char STOP[] = "stop";
static constexpr char RATE[] = "rate#";
}

namespace length {
static constexpr auto START = sizeof(cmd::START) - 1;
static constexpr auto STOP = sizeof(cmd::STOP) - 1;
static constexpr auto RATE = sizeof(cmd::RATE) - 1;
}

namespace udp {
static constexpr auto PORT = 0x5443;
}

#if defined (H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo) {
	gv_ltc_bTimeCodeAvailable = true;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

char *SystimeReader::s_pUdpBuffer;
SystimeReader *SystimeReader::s_pThis;

SystimeReader::SystimeReader(uint8_t nFps) : m_nFps(nFps) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	const auto nType = static_cast<uint8_t>(ltc::get_type(nFps));

	m_nTimer0Interval = TimeCodeConst::TMR_INTV[nType];
	m_tMidiTimeCode.nType = nType;
}

void SystimeReader::Start(bool bAutoStart) {
	// UDP Request
	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

#if defined (H3)
	// System Time -> Frames
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
#elif defined (GD32)
	platform::ltc::timer11_config();
	timer_single_pulse_mode_config(TIMER11, TIMER_SP_MODE_REPETITIVE);
	TIMER_CH0CV(TIMER11) = m_nTimer0Interval;
#endif

	LtcOutputs::Get()->Init();

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

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

	Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);

	DEBUG_EXIT
}

void SystimeReader::ActionStop() {
	DEBUG_ENTRY

	m_bIsStarted = false;

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void SystimeReader::ActionSetRate(const char *pTimeCodeRate) {
	DEBUG_ENTRY

	uint8_t nFps;
	ltc::Type type;

	if (ltc::parse_timecode_rate(pTimeCodeRate, nFps, type)) {
		if (nFps != m_nFps) {
			const auto nType = static_cast<uint8_t>(type);
			m_nFps = nFps;
			//
			if (m_tMidiTimeCode.nFrames >= m_nFps) {
				m_tMidiTimeCode.nFrames = static_cast<uint8_t>(m_nFps - 1);
			}
			m_tMidiTimeCode.nType = nType;
			//
#if defined (H3)
			m_nTimer0Interval = TimeCodeConst::TMR_INTV[nType];
			H3_TIMER->TMR0_INTV = m_nTimer0Interval;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
			TIMER_CNT(TIMER11) = 0;
			TIMER_CH0CV(TIMER11) = m_nTimer0Interval;
#endif

			if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
				LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode), false);
			}

			if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&m_tMidiTimeCode));
			}

			if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
				RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&m_tMidiTimeCode));
			}

			if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
				LtcEtc::Get()->Send(&m_tMidiTimeCode);
			}

			LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

			DEBUG_PRINTF("nFps=%d", nFps);
		}
	}

	DEBUG_EXIT
}

void SystimeReader::HandleRequest(char *pBuffer, uint16_t nBufferLength) {
	if (pBuffer != nullptr) {
		assert(nBufferLength >= 8);
		s_pUdpBuffer = pBuffer;
		m_nBytesReceived = nBufferLength;
	}

	if (memcmp("ltc!", s_pUdpBuffer, 4) != 0) {
		return;
	}

	if (s_pUdpBuffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	debug_dump(s_pUdpBuffer, m_nBytesReceived);

	if (m_nBytesReceived == (4 + length::START)) {
		if (memcmp(&s_pUdpBuffer[4], cmd::START, length::START) == 0) {
			ActionStart();
			return;
		}

		DEBUG_PUTS("Invalid !start command");
	}

	if (m_nBytesReceived == (4 + length::STOP)) {
		if (memcmp(&s_pUdpBuffer[4], cmd::STOP, length::STOP) == 0) {
			ActionStop();
			return;
		}

		DEBUG_PUTS("Invalid !stop command");
	}

	if (m_nBytesReceived == (4 + length::RATE  + ltc::timecode::RATE_MAX_LENGTH)) {
		if (memcmp(&s_pUdpBuffer[4], cmd::RATE, length::RATE) == 0) {
			ActionSetRate(&s_pUdpBuffer[(4 + length::RATE)]);
			return;
		}
	}

	DEBUG_PUTS("Invalid command");
}

void SystimeReader::HandleUdpRequest() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 8), 1)) {
		return;
	}

	HandleRequest();
}

void SystimeReader::Run() {
	if (m_bIsStarted) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

		auto nTime = time(nullptr);

		if (__builtin_expect((m_nTimePrevious != nTime), 0)) {
			m_nTimePrevious = nTime;

			m_tMidiTimeCode.nFrames = 0;
			m_tMidiTimeCode.nSeconds = static_cast<uint8_t>(nTime % 60);
			nTime /= 60;
			m_tMidiTimeCode.nMinutes = static_cast<uint8_t>(nTime % 60);
			nTime /= 60;
			m_tMidiTimeCode.nHours = static_cast<uint8_t>(nTime % 24);

#if defined (H3)
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
			TIMER_CNT(TIMER11) = 0;
#endif
			gv_ltc_bTimeCodeAvailable = true;
		}

		__DMB();
		if (__builtin_expect((gv_ltc_bTimeCodeAvailable), 0)) {
			gv_ltc_bTimeCodeAvailable = false;

			if (__builtin_expect((m_nFps != m_tMidiTimeCode.nFrames), 0)) {

				if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
					LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode), false);
				}

				if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
					ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&m_tMidiTimeCode));
				}

				if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
					RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&m_tMidiTimeCode));
				}

				if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
					LtcEtc::Get()->Send(&m_tMidiTimeCode);
				}

				LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

				m_tMidiTimeCode.nFrames++;
			}
		}
	}

	HandleUdpRequest();
}
