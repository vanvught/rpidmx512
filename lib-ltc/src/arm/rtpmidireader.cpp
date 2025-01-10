/**
 * @file rtpmidireader.cpp
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

#if defined (DEBUG_ARM_RTPMIDIREADER)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "arm/rtpmidireader.h"

#include "timecodeconst.h"
#include "hardware.h"
// Output
#include "artnetnode.h"
#include "midi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

static uint8_t s_qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}

static void irq_timer0_handler([[maybe_unused]] uint32_t clo) {
	gv_ltc_bTimeCodeAvailable = true;
	gv_ltc_nTimeCodeCounter = gv_ltc_nTimeCodeCounter + 1;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

void RtpMidiReader::Start() {
#if defined (H3)
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_handler_init();
#elif defined (GD32)
	platform::ltc::timer11_config();
#endif

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
}

void RtpMidiReader::Stop() {
#if defined (H3)
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(nullptr));
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif
}

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void RtpMidiReader::MidiMessage(const struct midi::Message *ptMidiMessage) {
	switch (static_cast<midi::Types>(ptMidiMessage->tType)) {
	case midi::Types::TIME_CODE_QUARTER_FRAME:
		HandleMtcQf(ptMidiMessage);
		break;
	case midi::Types::SYSTEM_EXCLUSIVE: {
			const auto *pSystemExclusive = ptMidiMessage->aSystemExclusive;
			if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
				HandleMtc(ptMidiMessage);
			}
		}
		break;
	case midi::Types::CLOCK: {
			uint32_t nBPM;
			if (m_MidiBPM.Get(ptMidiMessage->nTimestamp, nBPM)) {
				LtcOutputs::Get()->ShowBPM(nBPM);
			}
		}
		break;
	default:
		break;
	}
}

void RtpMidiReader::HandleMtc(const struct midi::Message *ptMidiMessage) {
	const auto *pSystemExclusive = ptMidiMessage->aSystemExclusive;

	m_LtcTimeCode.nFrames = pSystemExclusive[8];
	m_LtcTimeCode.nSeconds = pSystemExclusive[7];
	m_LtcTimeCode.nMinutes = pSystemExclusive[6];
	m_LtcTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	m_LtcTimeCode.nType = static_cast<uint8_t>(pSystemExclusive[5] >> 5);

	Update();

	gv_ltc_bTimeCodeAvailable = false;
	gv_ltc_nTimeCodeCounter = 0;
}

void RtpMidiReader::HandleMtcQf(const struct midi::Message *ptMidiMessage) {
	const auto nData1 = ptMidiMessage->nData1;
	const auto nPart = static_cast<uint8_t>((nData1 & 0x70) >> 4);

	s_qf[nPart] = nData1 & 0x0F;

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ( (m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0)) ) {
		m_LtcTimeCode.nFrames = static_cast<uint8_t>(s_qf[0] | (s_qf[1] << 4));
		m_LtcTimeCode.nSeconds = static_cast<uint8_t>(s_qf[2] | (s_qf[3] << 4));
		m_LtcTimeCode.nMinutes = static_cast<uint8_t>(s_qf[4] | (s_qf[5] << 4));
		m_LtcTimeCode.nHours = static_cast<uint8_t>(s_qf[6] | ((s_qf[7] & 0x1) << 4));
		m_LtcTimeCode.nType = static_cast<uint8_t>((s_qf[7] >> 1));

		if (m_LtcTimeCode.nFrames < m_nMtcQfFramePrevious) {
			m_nMtcQfFramesDelta = m_nMtcQfFramePrevious - m_LtcTimeCode.nFrames;
		} else {
			m_nMtcQfFramesDelta = m_LtcTimeCode.nFrames - m_nMtcQfFramePrevious;
		}

		m_nMtcQfFramePrevious = m_LtcTimeCode.nFrames;

		if (m_nMtcQfFramesDelta >= static_cast<uint32_t>(TimeCodeConst::FPS[m_LtcTimeCode.nType] - 2)) {
			m_nMtcQfFramesDelta = 2;
		}

		__DMB();

		if (gv_ltc_nTimeCodeCounter < m_nMtcQfFramesDelta) {
			Update();
		}

#if defined (H3)
		H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
		H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[m_LtcTimeCode.nType];
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
		platform::ltc::timer11_set_type(m_LtcTimeCode.nType);
#endif
		gv_ltc_bTimeCodeAvailable = false;
		gv_ltc_nTimeCodeCounter = 0;
	}

	m_nPartPrevious = nPart;
}

void RtpMidiReader::Update() {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(&m_LtcTimeCode));
	}

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<struct artnet::TimeCode*>(&m_LtcTimeCode));
	}

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
		LtcEtc::Get()->Send(reinterpret_cast<const midi::Timecode *>(&m_LtcTimeCode));
	}

	memcpy(&g_ltc_LtcTimeCode, &m_LtcTimeCode, sizeof(struct midi::Timecode));

	LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(&g_ltc_LtcTimeCode));

	gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}

void RtpMidiReader::Run() {
	__DMB();

	if (gv_ltc_bTimeCodeAvailable) {
		gv_ltc_bTimeCodeAvailable = false;

		const auto nFps = TimeCodeConst::FPS[m_LtcTimeCode.nType];

		if (m_bDirection) {
			m_LtcTimeCode.nFrames++;
			if (nFps == m_LtcTimeCode.nFrames) {
				m_LtcTimeCode.nFrames = 0;

				m_LtcTimeCode.nSeconds++;
				if (m_LtcTimeCode.nSeconds == 60) {
					m_LtcTimeCode.nSeconds = 0;

					m_LtcTimeCode.nMinutes++;
					if (m_LtcTimeCode.nMinutes == 60) {
						m_LtcTimeCode.nMinutes = 0;

						m_LtcTimeCode.nHours++;
						if (m_LtcTimeCode.nHours == 24) {
							m_LtcTimeCode.nHours = 0;
						}
					}
				}
			}
		} else {
			if (m_LtcTimeCode.nFrames == nFps - 1) {
				if (m_LtcTimeCode.nSeconds > 0) {
					m_LtcTimeCode.nSeconds--;
				} else {
					m_LtcTimeCode.nSeconds = 59;
				}

				if (m_LtcTimeCode.nSeconds == 59) {
					if (m_LtcTimeCode.nMinutes > 0) {
						m_LtcTimeCode.nMinutes--;
					} else {
						m_LtcTimeCode.nMinutes = 59;
					}

					if (m_LtcTimeCode.nMinutes == 59) {
						if (m_LtcTimeCode.nHours > 0) {
							m_LtcTimeCode.nHours--;
						} else {
							m_LtcTimeCode.nHours = 23;
						}
					}
				}
			}
		}

		Update();

 		if (m_nMtcQfFramesDelta == 2) {
 			m_nMtcQfFramesDelta = 0;
#if defined (H3)
 			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
 			H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[m_LtcTimeCode.nType];
 			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
 			platform::ltc::timer11_set_type(m_LtcTimeCode.nType);
#endif
 		}
	}

	__DMB();
	if (gv_ltc_nUpdatesPerSecond != 0) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	}
}
