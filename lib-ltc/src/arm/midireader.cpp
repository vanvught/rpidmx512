/**
 * @file midireader.cpp
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

#if defined (DEBUG_ARM_MIDIREADER)
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

#include "arm/midireader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "hardware.h"
// Input
#include "midi.h"
// Output
#include "ltcsender.h"
#include "artnetnode.h"
#include "ltcetc.h"
#include "net/rtpmidi.h"
#include "arm/ltcmidisystemrealtime.h"
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

#include "debug.h"

static uint8_t s_qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

#if defined (H3)
static void irq_timer1_handler() {
	gv_ltc_bTimeCodeAvailable = true;
	gv_ltc_nTimeCodeCounter = gv_ltc_nTimeCodeCounter + 1;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

void MidiReader::Start() {
	DEBUG_ENTRY

#if defined (H3)
	Midi::Get()->SetIrqTimer1(irq_timer1_handler);
#elif defined (GD32)
	platform::ltc::timer11_config();
#endif
	Midi::Get()->Init(midi::Direction::INPUT);

	LtcOutputs::Get()->Init(true);
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void MidiReader::HandleMtc() {
	uint8_t nSystemExclusiveLength;
	const auto *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);

	g_ltc_LtcTimeCode.nHours = pSystemExclusive[5] & 0x1F;
	g_ltc_LtcTimeCode.nMinutes = pSystemExclusive[6];
	g_ltc_LtcTimeCode.nSeconds = pSystemExclusive[7];
	g_ltc_LtcTimeCode.nFrames = pSystemExclusive[8];
	g_ltc_LtcTimeCode.nType = static_cast<uint8_t>(pSystemExclusive[5] >> 5);

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI)) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	Update();

	gv_ltc_bTimeCodeAvailable = false;
}

void MidiReader::HandleMtcQf() {
	uint8_t nData1, nData2;
	Midi::Get()->GetMessageData(nData1, nData2);

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI)) {
		RtpMidi::Get()->SendQf(nData1);
	}

	const auto nPart = static_cast<uint8_t>((nData1 & 0x70) >> 4);

	s_qf[nPart] = nData1 & 0x0F;

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ((m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0))) {
		g_ltc_LtcTimeCode.nHours = static_cast<uint8_t>(s_qf[6] | ((s_qf[7] & 0x1) << 4));
		g_ltc_LtcTimeCode.nMinutes = static_cast<uint8_t>(s_qf[4] | (s_qf[5] << 4));
		g_ltc_LtcTimeCode.nSeconds = static_cast<uint8_t>(s_qf[2] | (s_qf[3] << 4));
		g_ltc_LtcTimeCode.nFrames = static_cast<uint8_t>(s_qf[0] | (s_qf[1] << 4));
		g_ltc_LtcTimeCode.nType = static_cast<uint8_t>(s_qf[7] >> 1);

		if (g_ltc_LtcTimeCode.nFrames < m_nMtcQfFramePrevious) {
			m_nMtcQfFramesDelta = m_nMtcQfFramePrevious - g_ltc_LtcTimeCode.nFrames;
		} else {
			m_nMtcQfFramesDelta = g_ltc_LtcTimeCode.nFrames - m_nMtcQfFramePrevious;
		}

		m_nMtcQfFramePrevious = g_ltc_LtcTimeCode.nFrames;

		if (m_nMtcQfFramesDelta >= static_cast<uint32_t>(TimeCodeConst::FPS[g_ltc_LtcTimeCode.nType] - 2)) {
			m_nMtcQfFramesDelta = 2;
		}

		__DMB();

		if (gv_ltc_nTimeCodeCounter < m_nMtcQfFramesDelta) {
			Update();
		}

#if defined (H3)
		H3_TIMER->TMR1_CTRL |= TIMER_CTRL_SINGLE_MODE;
		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[g_ltc_LtcTimeCode.nType];
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START);
#elif defined (GD32)
		platform::ltc::timer11_set_type(g_ltc_LtcTimeCode.nType);
#endif
		gv_ltc_bTimeCodeAvailable = false;
		gv_ltc_nTimeCodeCounter = 0;
	}

	m_nPartPrevious = nPart;
}

void MidiReader::Update() {
	if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
	}

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
		ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(&g_ltc_LtcTimeCode));
	}

	if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
		LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&g_ltc_LtcTimeCode));
	}

	LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));
}

void MidiReader::Run() {
	if (Midi::Get()->Read(static_cast<uint8_t>(midi::Channel::OMNI))) {
		if (Midi::Get()->GetChannel() == 0) {
			switch (Midi::Get()->GetMessageType()) {
			case midi::Types::TIME_CODE_QUARTER_FRAME:
				HandleMtcQf();
				break;
			case midi::Types::SYSTEM_EXCLUSIVE: {
				uint8_t nSystemExclusiveLength;
				const auto *pSystemExclusive = Midi::Get()->GetSystemExclusive(nSystemExclusiveLength);
				if ((pSystemExclusive[1] == 0x7F) && (pSystemExclusive[2] == 0x7F) && (pSystemExclusive[3] == 0x01)) {
					HandleMtc();
				}
			}
				break;
			case midi::Types::CLOCK:
				uint32_t nBPM;
				if (m_MidiBPM.Get(Midi::Get()->GetMessageTimeStamp() * 10, nBPM)) {
					LtcOutputs::Get()->ShowBPM(nBPM);
				}
				break;
			default:
				break;
			}
		}
	}

	__DMB();

	if (gv_ltc_bTimeCodeAvailable) {
		gv_ltc_bTimeCodeAvailable = false;
		const auto nFps = TimeCodeConst::FPS[g_ltc_LtcTimeCode.nType];

		if (m_bDirection) {
			g_ltc_LtcTimeCode.nFrames++;
			if (nFps == g_ltc_LtcTimeCode.nFrames) {
				g_ltc_LtcTimeCode.nFrames = 0;

				g_ltc_LtcTimeCode.nSeconds++;
				if (g_ltc_LtcTimeCode.nSeconds == 60) {
					g_ltc_LtcTimeCode.nSeconds = 0;

					g_ltc_LtcTimeCode.nMinutes++;
					if (g_ltc_LtcTimeCode.nMinutes == 60) {
						g_ltc_LtcTimeCode.nMinutes = 0;

						g_ltc_LtcTimeCode.nHours++;
						if (g_ltc_LtcTimeCode.nHours == 24) {
							g_ltc_LtcTimeCode.nHours = 0;
						}
					}
				}
			}
		} else {
			if (g_ltc_LtcTimeCode.nFrames == nFps - 1) {
				if (g_ltc_LtcTimeCode.nSeconds > 0) {
					g_ltc_LtcTimeCode.nSeconds--;
				} else {
					g_ltc_LtcTimeCode.nSeconds = 59;
				}

				if (g_ltc_LtcTimeCode.nSeconds == 59) {
					if (g_ltc_LtcTimeCode.nMinutes > 0) {
						g_ltc_LtcTimeCode.nMinutes--;
					} else {
						g_ltc_LtcTimeCode.nMinutes = 59;
					}

					if (g_ltc_LtcTimeCode.nMinutes == 59) {
						if (g_ltc_LtcTimeCode.nHours > 0) {
							g_ltc_LtcTimeCode.nHours--;
						} else {
							g_ltc_LtcTimeCode.nHours = 23;
						}
					}
				}
			}
		}

		Update();
	}

	if (Midi::Get()->GetUpdatesPerSecond() != 0)  {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	}
}
