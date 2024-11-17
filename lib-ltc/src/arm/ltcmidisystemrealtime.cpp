/**
 * @file ltcmidisystemrealtime.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cctype>
#include <cassert>

#include "arm/ltcmidisystemrealtime.h"

#include "ltc.h"

#include "midi.h"

#include "network.h"

#include "debug.h"

namespace cmd {
static constexpr char START[] = "start";
static constexpr char STOP[] = "stop";
static constexpr char CONTINUE[] = "continue";
static constexpr char BPM[] = "bpm#";
}

namespace length {
static constexpr auto START = sizeof(cmd::START) - 1;
static constexpr auto STOP = sizeof(cmd::STOP) - 1;
static constexpr auto CONTINUE = sizeof(cmd::CONTINUE) - 1;
static constexpr auto BPM = sizeof(cmd::BPM) - 1;
}

namespace udp {
static constexpr auto PORT = 0x4444;
}

#if defined (H3)
static void timer_handler() {
	if (!ltc::g_DisabledOutputs.bRtpMidi) {
		RtpMidi::Get()->SendRaw(midi::Types::CLOCK);
	}

	if (!ltc::g_DisabledOutputs.bMidi) {
		Midi::Get()->SendRaw(midi::Types::CLOCK);
	}
}
#elif defined (GD32)
#endif

void LtcMidiSystemRealtime::Start() {
	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);
}

void LtcMidiSystemRealtime::Stop() {
	m_nHandle = Network::Get()->End(udp::PORT);
	assert(m_nHandle == -1);
}

void LtcMidiSystemRealtime::SetBPM(uint32_t nBPM) {
	if ((!ltc::g_DisabledOutputs.bRtpMidi) || (!ltc::g_DisabledOutputs.bMidi)) {
		if (nBPM != m_nBPMPrevious) {
			m_nBPMPrevious = nBPM;

			if (nBPM == 0) {
#if defined (H3)
				irq_timer_arm_virtual_set(nullptr, 0);
#elif defined (GD32)
#endif
			} else if ((nBPM >= midi::bpm::MIN) && (nBPM <= midi::bpm::MAX)) {
#if defined (H3)
				const uint32_t nValue =  60000000U / nBPM;
				irq_timer_arm_virtual_set(reinterpret_cast<thunk_irq_timer_arm_t>(timer_handler), nValue);
#elif defined (GD32)
#endif
			}
		}
	}
}

void LtcMidiSystemRealtime::Run() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < 9), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("midi!", s_pUdpBuffer, 5) != 0), 0)) {
		return;
	}

	if (s_pUdpBuffer[nBytesReceived - 1] == '\n') {
		nBytesReceived--;
	}

	debug_dump(s_pUdpBuffer, nBytesReceived);

	if (nBytesReceived == (5 + length::START)) {
		if (memcmp(&s_pUdpBuffer[5], cmd::START, length::START) == 0) {
			SendStart();
			DEBUG_PUTS("Start");
			return;
		}
	}

	if (nBytesReceived == (5 + length::STOP)) {
		if (memcmp(&s_pUdpBuffer[5], cmd::STOP, length::STOP) == 0) {
			SendStop();
			DEBUG_PUTS("Stop");
			return;
		}
	}

	if (nBytesReceived == (5 + length::CONTINUE)) {
		if (memcmp(&s_pUdpBuffer[5], cmd::CONTINUE, length::CONTINUE) == 0) {
			SendContinue();
			DEBUG_PUTS("Continue");
			return;
		}
	}

	if (nBytesReceived == (5 + length::BPM + 3)) {
		if (memcmp(&s_pUdpBuffer[5], cmd::BPM, length::BPM) == 0) {
			uint32_t nOfffset = 5 + length::BPM;
			uint32_t nBPM;

			if (isdigit(s_pUdpBuffer[nOfffset])) {
				nBPM = 100U * static_cast<uint32_t>(s_pUdpBuffer[nOfffset++] - '0');
				if (isdigit(s_pUdpBuffer[nOfffset])) {
					nBPM += 10U * static_cast<uint32_t>(s_pUdpBuffer[nOfffset++] - '0');
					if (isdigit(s_pUdpBuffer[nOfffset])) {
						nBPM += static_cast<uint32_t>(s_pUdpBuffer[nOfffset++] - '0');
						SetBPM(nBPM);
						ShowBPM(nBPM);
						DEBUG_PRINTF("BPM: %u", nBPM);
					}
				}
			}
			return;
		}
	}
}
