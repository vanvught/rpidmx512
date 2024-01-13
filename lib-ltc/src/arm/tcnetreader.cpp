/**
 * @file tcnetreader.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "tcnetreader.h"

#include "timecodeconst.h"
#include "network.h"
#include "hardware.h"

// Input
#include "tcnet.h"
// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "tcnetdisplay.h"
#include "ltcoutputs.h"

#include "platform_ltc.h"

#include "debug.h"

namespace cmd {
static constexpr char LAYER[] = "layer#";
static constexpr char TYPE[] = "type#";
static constexpr char TIMECODE[] = "timecode#";
}

namespace length {
static constexpr auto LAYER = sizeof(cmd::LAYER) - 1;
static constexpr auto TYPE = sizeof(cmd::TYPE) - 1;
static constexpr auto TIMECODE = sizeof(cmd::TIMECODE) - 1;
}

namespace udp {
static constexpr auto PORT = 0x0ACA;
}

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

char *TCNetReader::s_pUdpBuffer;

void TCNetReader::Start() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_timer_init();
#elif defined (GD32)
	platform::ltc::timer6_config();
#endif

	LtcOutputs::Get()->Init();

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);
}

void TCNetReader::Stop() {
#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif
}

void TCNetReader::Handler(const struct TTCNetTimeCode *pTimeCode) {
	gv_ltc_nUpdates++;

	assert((reinterpret_cast<uint32_t>(pTimeCode) & 0x3) == 0); // Check if we can do 4-byte compare
#if __GNUC__ > 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"	// FIXME ignored "-Waddress-of-packed-member"
#endif
	const auto *p = reinterpret_cast<const uint32_t*>(pTimeCode);
#if __GNUC__ > 8
#pragma GCC diagnostic pop
#endif

	if (m_nTimeCodePrevious != *p) {
		m_nTimeCodePrevious = *p;

		if (!g_ltc_ptLtcDisabledOutputs.bLtc) {
			LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode*>(pTimeCode));
		}

		if (!g_ltc_ptLtcDisabledOutputs.bArtNet) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(pTimeCode));
		}

		if (!g_ltc_ptLtcDisabledOutputs.bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(pTimeCode));
		}

		if (!g_ltc_ptLtcDisabledOutputs.bEtc) {
			LtcEtc::Get()->Send(&m_tMidiTimeCode);
		}

		memcpy(&m_tMidiTimeCode, pTimeCode, sizeof(struct midi::Timecode));

		LtcOutputs::Get()->Update(reinterpret_cast<const struct ltc::TimeCode*>(pTimeCode));
	}
}

void TCNetReader::HandleUdpRequest() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < 13), 1)) {
		return;
	}

	if (__builtin_expect((memcmp("tcnet!", s_pUdpBuffer, 6) != 0), 0)) {
		return;
	}

	if (s_pUdpBuffer[nBytesReceived - 1] == '\n') {
		nBytesReceived--;
	}

	debug_dump(s_pUdpBuffer, nBytesReceived);

	if ((nBytesReceived == (6 + length::LAYER + 1)) && (memcmp(&s_pUdpBuffer[6], cmd::LAYER, length::LAYER) == 0)) {
		const auto tLayer = TCNet::GetLayer(s_pUdpBuffer[6 + length::LAYER]);

		TCNet::Get()->SetLayer(tLayer);
		tcnet::display::show();

		DEBUG_PRINTF("tcnet!layer#%c -> %d", s_pUdpBuffer[6 + length::LAYER + 1], tLayer);
		return;
	}

	if ((nBytesReceived == (6 + length::TYPE + 2)) && (memcmp(&s_pUdpBuffer[6], cmd::TYPE, length::TYPE) == 0)) {
		if (s_pUdpBuffer[6 + length::TYPE] == '2') {

			const auto nValue = 20U + s_pUdpBuffer[6 + length::TYPE + 1] - '0';

			switch (nValue) {
			case 24:
				TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_FILM);
				break;
			case 25:
				TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_EBU_25FPS);
				break;
			case 29:
				TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_DF);
				break;;
			default:
				break;
			}

			tcnet::display::show();

			DEBUG_PRINTF("tcnet!type#%d", nValue);
			return;
		}

		if ((s_pUdpBuffer[6 + length::TYPE] == '3') && (s_pUdpBuffer[6 + length::TYPE + 1] == '0')) {
			TCNet::Get()->SetTimeCodeType(TCNET_TIMECODE_TYPE_SMPTE_30FPS);
			tcnet::display::show();

			DEBUG_PUTS("tcnet!type#30");
			return;
		}

		DEBUG_PUTS("Invalid tcnet!type command");
		return;
	}

	if ((nBytesReceived == (6 + length::TIMECODE + 1)) && (memcmp(&s_pUdpBuffer[6], cmd::TIMECODE, length::TIMECODE) == 0)) {
		const auto nChar = s_pUdpBuffer[6 + length::TIMECODE];
		const auto bUseTimeCode = ((nChar == 'y') || (nChar == 'Y'));

		TCNet::Get()->SetUseTimeCode(bUseTimeCode);
		tcnet::display::show();

		DEBUG_PRINTF("tcnet!timecode#%c -> %d", nChar, static_cast<int>(bUseTimeCode));
		return;
	}

	DEBUG_PUTS("Invalid command");
}

void TCNetReader::Run() {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

	__DMB();
	if (gv_ltc_nUpdatesPerSecond != 0) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		m_nTimeCodePrevious = static_cast<uint32_t>(~0);
	}

	HandleUdpRequest();
}
