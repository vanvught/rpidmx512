/**
 * @file systimereader.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_ARM_SYSTIMEREADER)
# undef NDEBUG
#endif

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cassert>

#include "arm/systimereader.h"
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
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

#include "debug.h"

static constexpr char CMD_START[] = "start";
static constexpr auto START_LENGTH = sizeof(CMD_START) - 1;

static constexpr char CMD_STOP[] = "stop";
static constexpr auto STOP_LENGTH = sizeof(CMD_STOP) - 1;

static constexpr char CMD_RATE[] = "rate#";
static constexpr auto RATE_LENGTH = sizeof(CMD_RATE) - 1;

static constexpr uint16_t UDP_PORT = 0x5443;

#if defined (H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo) {
	gv_ltc_bTimeCodeAvailable = true;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

SystimeReader::SystimeReader(uint8_t nFps, int32_t nUtcOffset) : m_nFps(nFps), m_nUtcOffset(nUtcOffset) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_MidiTimeCode.nType = static_cast<uint8_t>(ltc::g_Type);

	DEBUG_EXIT
}

void SystimeReader::Start(bool bAutoStart) {
	m_nHandle = Network::Get()->Begin(UDP_PORT, staticCallbackFunction);
	assert(m_nHandle != -1);

	const auto nTimerInterval = TimeCodeConst::TMR_INTV[static_cast<uint8_t>(ltc::g_Type)];

#if defined (H3)
	// System Time -> Frames
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_INTV = nTimerInterval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);

	__enable_irq();
	__DMB();
#elif defined (GD32)
	platform::ltc::timer11_config();
	TIMER_CAR(TIMER11) = nTimerInterval;
	TIMER_CNT(TIMER11) = 0;
	TIMER_CTL0(TIMER11) |= TIMER_CTL0_CEN;
#endif

	LtcOutputs::Get()->Init();

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	if (bAutoStart) {
		ActionStart();
	}
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

	if (ltc::parse_timecode_rate(pTimeCodeRate, nFps)) {
		if (nFps != m_nFps) {
			m_nFps = nFps;
			//
			if (m_MidiTimeCode.nFrames >= m_nFps) {
				m_MidiTimeCode.nFrames = static_cast<uint8_t>(m_nFps - 1);
			}

			const auto nType = static_cast<uint8_t>(ltc::g_Type);

			m_MidiTimeCode.nType = nType;
			const auto nTimerInterval = TimeCodeConst::TMR_INTV[nType];
			//
#if defined (H3)
			H3_TIMER->TMR0_INTV = nTimerInterval;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
			TIMER_CAR(TIMER11) = nTimerInterval;
			TIMER_CNT(TIMER11) = 0;
#endif

			if (!ltc::g_DisabledOutputs.bLtc) {
				LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&m_MidiTimeCode), false);
			}

			if (!ltc::g_DisabledOutputs.bArtNet) {
				ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode*>(&m_MidiTimeCode));
			}

			if (!ltc::g_DisabledOutputs.bEtc) {
				LtcEtc::Get()->Send(&m_MidiTimeCode);
			}

			LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&m_MidiTimeCode));

			DEBUG_PRINTF("nFps=%d", nFps);
		}
	}

	DEBUG_EXIT
}

void SystimeReader::HandleRequest(char *pBuffer, uint16_t nBufferLength) {
	if (pBuffer != nullptr) {
		assert(nBufferLength >= 8);
		m_pUdpBuffer = pBuffer;
		m_nBytesReceived = nBufferLength;
	}

	if (memcmp("ltc!", m_pUdpBuffer, 4) != 0) {
		return;
	}

	if (m_pUdpBuffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	debug_dump(m_pUdpBuffer, m_nBytesReceived);

	if (m_nBytesReceived == (4 + START_LENGTH)) {
		if (memcmp(&m_pUdpBuffer[4], CMD_START, START_LENGTH) == 0) {
			ActionStart();
			return;
		}

		DEBUG_PUTS("Invalid !start command");
	}

	if (m_nBytesReceived == (4 + STOP_LENGTH)) {
		if (memcmp(&m_pUdpBuffer[4], CMD_STOP, STOP_LENGTH) == 0) {
			ActionStop();
			return;
		}

		DEBUG_PUTS("Invalid !stop command");
	}

	if (m_nBytesReceived == (4 + RATE_LENGTH  + ltc::timecode::RATE_MAX_LENGTH)) {
		if (memcmp(&m_pUdpBuffer[4], CMD_RATE, RATE_LENGTH) == 0) {
			ActionSetRate(&m_pUdpBuffer[(4 + RATE_LENGTH)]);
			return;
		}
	}

	DEBUG_PUTS("Invalid command");
}

void SystimeReader::Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	m_pUdpBuffer = reinterpret_cast<char *>(const_cast<uint8_t *>(pBuffer));
	m_nBytesReceived = nSize;

	HandleRequest();
}

void SystimeReader::Run() {
	if (m_bIsStarted) {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct ltc::TimeCode*>(&m_MidiTimeCode));

		struct timeval tv;
		gettimeofday(&tv, 0);
		auto nTime = tv.tv_sec + m_nUtcOffset;

		if (__builtin_expect((m_nTimePrevious != nTime), 0)) {
			m_nTimePrevious = nTime;

			m_MidiTimeCode.nFrames = 0;
			m_MidiTimeCode.nSeconds = static_cast<uint8_t>(nTime % 60U);
			nTime /= 60U;
			m_MidiTimeCode.nMinutes = static_cast<uint8_t>(nTime % 60U);
			nTime /= 60U;
			m_MidiTimeCode.nHours = static_cast<uint8_t>(nTime % 24U);

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

			if (__builtin_expect((m_nFps != m_MidiTimeCode.nFrames), 0)) {

				if (!ltc::g_DisabledOutputs.bLtc) {
					LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(&m_MidiTimeCode), false);
				}

				if (!ltc::g_DisabledOutputs.bArtNet) {
					ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(&m_MidiTimeCode));
				}

				if (!ltc::g_DisabledOutputs.bEtc) {
					LtcEtc::Get()->Send(&m_MidiTimeCode);
				}

				LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(&m_MidiTimeCode));

				m_MidiTimeCode.nFrames++;
			}
		}
	}
}
