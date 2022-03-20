/**
 * @file rtpmidireader.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3/rtpmidireader.h"

#include "timecodeconst.h"

#include "arm/synchronize.h"
#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

#include "ledblink.h"

// Output
#include "artnetnode.h"
#include "midi.h"
#include "ltcetc.h"
#include "h3/ltcsender.h"
#include "h3/ltcoutputs.h"

static uint8_t s_qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

/**
 * ARM Generic Timer
 */
static volatile uint32_t sv_nUpdatesPerSecond;
static volatile uint32_t sv_nUpdatesPrevious;
static volatile uint32_t sv_nUpdates;
/**
 *  Timer0
 */
static volatile bool sv_bTimeCodeAvailable;
static volatile uint32_t sv_bTimeCodeCounter;

static void irq_timer0_handler(__attribute__((unused)) uint32_t clo) {
	sv_bTimeCodeAvailable = true;
	sv_bTimeCodeCounter++;
}

static void irq_arm_handler() {
	sv_nUpdatesPerSecond = sv_nUpdates - sv_nUpdatesPrevious;
	sv_nUpdatesPrevious = sv_nUpdates;
}

RtpMidiReader::RtpMidiReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) : m_ptLtcDisabledOutputs(pLtcDisabledOutputs) {
	assert(m_ptLtcDisabledOutputs != nullptr);
}

void RtpMidiReader::Start() {
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(irq_arm_handler));
	irq_timer_init();

	LtcOutputs::Get()->Init();
	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
}

void RtpMidiReader::Stop() {
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(nullptr));
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
}

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

	m_tLtcTimeCode.nFrames = pSystemExclusive[8];
	m_tLtcTimeCode.nSeconds = pSystemExclusive[7];
	m_tLtcTimeCode.nMinutes = pSystemExclusive[6];
	m_tLtcTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	m_tLtcTimeCode.nType = static_cast<uint8_t>(pSystemExclusive[5] >> 5);

	Update();

	sv_bTimeCodeAvailable = false;
	sv_bTimeCodeCounter = 0;
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
		m_tLtcTimeCode.nFrames = static_cast<uint8_t>(s_qf[0] | (s_qf[1] << 4));
		m_tLtcTimeCode.nSeconds = static_cast<uint8_t>(s_qf[2] | (s_qf[3] << 4));
		m_tLtcTimeCode.nMinutes = static_cast<uint8_t>(s_qf[4] | (s_qf[5] << 4));
		m_tLtcTimeCode.nHours = static_cast<uint8_t>(s_qf[6] | ((s_qf[7] & 0x1) << 4));
		m_tLtcTimeCode.nType = static_cast<uint8_t>((s_qf[7] >> 1));

		if (m_tLtcTimeCode.nFrames < m_nMtcQfFramePrevious) {
			m_nMtcQfFramesDelta = m_nMtcQfFramePrevious - m_tLtcTimeCode.nFrames;
		} else {
			m_nMtcQfFramesDelta = m_tLtcTimeCode.nFrames - m_nMtcQfFramePrevious;
		}

		m_nMtcQfFramePrevious = m_tLtcTimeCode.nFrames;

		if (m_nMtcQfFramesDelta >= static_cast<uint32_t>(TimeCodeConst::FPS[m_tLtcTimeCode.nType] - 2)) {
			m_nMtcQfFramesDelta = 2;
		}

		dmb();
		if (sv_bTimeCodeCounter < m_nMtcQfFramesDelta) {
			Update();
		}

		H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
		H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[m_tLtcTimeCode.nType];
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		sv_bTimeCodeAvailable = false;
		sv_bTimeCodeCounter = 0;
	}

	m_nPartPrevious = nPart;
}

void RtpMidiReader::Update() {
	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bArtNet) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<struct TArtNetTimeCode*>(&m_tLtcTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bEtc) {
		LtcEtc::Get()->Send(reinterpret_cast<const midi::Timecode *>(&m_tLtcTimeCode));
	}

	LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));

	sv_nUpdates++;
}

void RtpMidiReader::Run() {
	dmb();
	if (sv_bTimeCodeAvailable) {
		sv_bTimeCodeAvailable = false;

		const auto nFps = TimeCodeConst::FPS[m_tLtcTimeCode.nType];

		if (m_bDirection) {
			m_tLtcTimeCode.nFrames++;
			if (nFps == m_tLtcTimeCode.nFrames) {
				m_tLtcTimeCode.nFrames = 0;

				m_tLtcTimeCode.nSeconds++;
				if (m_tLtcTimeCode.nSeconds == 60) {
					m_tLtcTimeCode.nSeconds = 0;

					m_tLtcTimeCode.nMinutes++;
					if (m_tLtcTimeCode.nMinutes == 60) {
						m_tLtcTimeCode.nMinutes = 0;

						m_tLtcTimeCode.nHours++;
						if (m_tLtcTimeCode.nHours == 24) {
							m_tLtcTimeCode.nHours = 0;
						}
					}
				}
			}
		} else {
			if (m_tLtcTimeCode.nFrames == nFps - 1) {
				if (m_tLtcTimeCode.nSeconds > 0) {
					m_tLtcTimeCode.nSeconds--;
				} else {
					m_tLtcTimeCode.nSeconds = 59;
				}

				if (m_tLtcTimeCode.nSeconds == 59) {
					if (m_tLtcTimeCode.nMinutes > 0) {
						m_tLtcTimeCode.nMinutes--;
					} else {
						m_tLtcTimeCode.nMinutes = 59;
					}

					if (m_tLtcTimeCode.nMinutes == 59) {
						if (m_tLtcTimeCode.nHours > 0) {
							m_tLtcTimeCode.nHours--;
						} else {
							m_tLtcTimeCode.nHours = 23;
						}
					}
				}
			}
		}

		Update();

 		if (m_nMtcQfFramesDelta == 2) {
 			m_nMtcQfFramesDelta = 0;
 			H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
 			H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[m_tLtcTimeCode.nType];
 			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
 		}
	}

	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct TLtcTimeCode*>(&m_tLtcTimeCode));

	dmb();
	if (sv_nUpdatesPerSecond != 0) {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
