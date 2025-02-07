/**
 * @file systimereader.h
 *
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

#if defined (DEBUG_ARM_SYSTIMEREADER)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "arm/systimereader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "hardware.h"
#include "network.h"
// Output
#include "artnetnode.h"
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

	g_ltc_LtcTimeCode.nType = static_cast<uint8_t>(ltc::g_Type);

	DEBUG_EXIT
}

void SystimeReader::Start(bool bAutoStart) {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));

	H3_TIMER->TMR0_CUR = 0;
	H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[static_cast<uint8_t>(ltc::g_Type)];
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	__enable_irq();
	__DMB();
#elif defined (GD32)
	platform::ltc::timer11_config();
	platform::ltc::timer11_set_type(static_cast<uint8_t>(ltc::g_Type));
#endif

	m_nHandle = Network::Get()->Begin(UDP_PORT, StaticCallbackFunction);
	assert(m_nHandle != -1);

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	if (bAutoStart) {
		ActionStart();
	}

	DEBUG_EXIT
}

void SystimeReader::SetFps(uint8_t nFps) {
	if (nFps != m_nFps) {
		m_nFps = nFps;

		if (g_ltc_LtcTimeCode.nFrames >= m_nFps) {
			g_ltc_LtcTimeCode.nFrames = static_cast<uint8_t>(m_nFps - 1);
		}

		const auto nType = static_cast<uint8_t>(ltc::g_Type);
		g_ltc_LtcTimeCode.nType = nType;

#if defined (H3)
		H3_TIMER->TMR0_CUR = 0;
		H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[nType];
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
		platform::ltc::timer11_set_type(nType);
#endif

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
			LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode), false);
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(&g_ltc_LtcTimeCode));
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
		}

		LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
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
		SetFps(nFps);
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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void SystimeReader::Run() {
	// If not started, return early
	if (__builtin_expect((m_bIsStarted), 0)) {
		struct timeval tv;
		gettimeofday(&tv, 0);
		auto nTime = tv.tv_sec + m_nUtcOffset;

		// Calculate frames
		g_ltc_LtcTimeCode.nFrames = (tv.tv_usec * TimeCodeConst::FPS[g_ltc_LtcTimeCode.nType]) / 1000000U;

		// Drop-frame adjustments BEFORE time updates
		if (ltc::g_Type == ltc::Type::DF) {
			// Skip frames 00 and 01 in non-10th minutes
			if ((g_ltc_LtcTimeCode.nMinutes % 10 != 0) && (g_ltc_LtcTimeCode.nSeconds == 0) && (g_ltc_LtcTimeCode.nFrames < 2)) {
				g_ltc_LtcTimeCode.nFrames = 2;
			}
		}

		// Update timecode components if the time has changed
		if (__builtin_expect((m_nTimePrevious != nTime), 0)) {
			m_nTimePrevious = nTime;

			g_ltc_LtcTimeCode.nSeconds = static_cast<uint8_t>(nTime % 60U);
			nTime /= 60U;
			g_ltc_LtcTimeCode.nMinutes = static_cast<uint8_t>(nTime % 60U);
			nTime /= 60U;
			g_ltc_LtcTimeCode.nHours = static_cast<uint8_t>(nTime % 24U);

			// Trigger timecode availability at the start of a second
			if (tv.tv_usec == 0) {
#if defined (H3)
				H3_TIMER->TMR0_CUR = 0;
				H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
				TIMER_CNT(TIMER11) = 0;
				TIMER_CTL0(TIMER11) |= TIMER_CTL0_CEN;
#endif
				gv_ltc_bTimeCodeAvailable = true;
			}
		}
	}

	// Update timecode outputs if available
	__DMB();  // Data memory barrier to ensure memory consistency
	if (__builtin_expect((gv_ltc_bTimeCodeAvailable), 0)) {
		gv_ltc_bTimeCodeAvailable = false;

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
			LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode), false);
		}

		if (__builtin_expect((!m_bIsStarted), 0)) {
			return;
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(&g_ltc_LtcTimeCode));
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
		}

		if (__builtin_expect((m_bIsStarted), 0)) {
			LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
		}
	}
}
