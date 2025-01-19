/**
 * @file tcnetreader.cpp
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

#if defined (DEBUG_ARM_TCNETREADER)
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

#include "arm/tcnetreader.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "network.h"
#include "hardware.h"
// Input
#include "tcnet.h"
// Output
#include "artnetnode.h"
#include "ltcetc.h"
#include "ltcsender.h"
#include "tcnetdisplay.h"
#include "arm/ltcoutputs.h"

#include "arm/platform_ltc.h"

#include "debug.h"

static constexpr char CMD_LAYER[] = "layer#";
static constexpr auto LAYER_LENGTH = sizeof(CMD_LAYER) - 1U;

static constexpr char CMD_TYPE[] = "type#";
static constexpr auto TYPE_LENGTH = sizeof(CMD_TYPE) - 1U;

static constexpr char CMD_TIMECODE[] = "timecode#";
static constexpr auto TIMECODE_LENGTH = sizeof(CMD_TIMECODE) - 1U;

static constexpr auto UDP_PORT = 0x0ACA;

#if defined (H3)
static void irq_timer0_handler([[maybe_unused]] uint32_t clo) {
	gv_ltc_bTimeCodeAvailable = true;
}
#elif defined (GD32)
// Defined in platform_ltc.cpp
#endif

#if defined (H3)
static void arm_timer_handler() {
	gv_ltc_nUpdatesPerSecond = gv_ltc_nUpdates - gv_ltc_nUpdatesPrevious;
	gv_ltc_nUpdatesPrevious = gv_ltc_nUpdates;
}
#elif defined (GD32)
	// Defined in platform_ltc.cpp
#endif

void TCNetReader::Start() {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_handler));
	H3_TIMER->TMR0_CUR = 0;
#elif defined (GD32)
	platform::ltc::timer11_config();
#endif

#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));
	irq_handler_init();
#elif defined (GD32)
#endif

	m_nHandle = Network::Get()->Begin(UDP_PORT, StaticCallbackFunctionInput);
	assert(m_nHandle != -1);

	LtcOutputs::Get()->Init();
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void TCNetReader::Stop() {
	DEBUG_ENTRY

#if defined (H3)
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(nullptr));
#elif defined (GD32)
#endif

	DEBUG_EXIT
}

void TCNetReader::Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	if (__builtin_expect((memcmp("tcnet!", pBuffer, 6) != 0), 0)) {
		return;
	}

	if (pBuffer[nSize - 1] == '\n') {
		nSize--;
	}

	debug_dump(pBuffer, nSize);

	if ((nSize == (6 + LAYER_LENGTH + 1)) && (memcmp(&pBuffer[6], CMD_LAYER, LAYER_LENGTH) == 0)) {
		const auto tLayer = TCNet::GetLayer(pBuffer[6 + LAYER_LENGTH]);

		TCNet::Get()->SetLayer(tLayer);
		tcnet::display::show();

		DEBUG_PRINTF("tcnet!layer#%c -> %d", pBuffer[6 + LAYER_LENGTH + 1], tLayer);
		return;
	}

	if ((nSize == (6 + TYPE_LENGTH + 2)) && (memcmp(&pBuffer[6], CMD_TYPE, TYPE_LENGTH) == 0)) {
		if (pBuffer[6 + TYPE_LENGTH] == '2') {

			const auto nValue = 20U + pBuffer[6 + TYPE_LENGTH + 1] - '0';

			switch (nValue) {
			case 24:
				TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_FILM);
				break;
			case 25:
				TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_EBU_25FPS);
				break;
			case 29:
				TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_DF);
				break;;
			default:
				break;
			}

			tcnet::display::show();

			DEBUG_PRINTF("tcnet!type#%d", nValue);
			return;
		}

		if ((pBuffer[6 + TYPE_LENGTH] == '3') && (pBuffer[6 + TYPE_LENGTH + 1] == '0')) {
			TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);
			tcnet::display::show();

			DEBUG_PUTS("tcnet!type#30");
			return;
		}

		DEBUG_PUTS("Invalid tcnet!type command");
		return;
	}

	if ((nSize == (6 + TIMECODE_LENGTH + 1)) && (memcmp(&pBuffer[6], CMD_TIMECODE, TIMECODE_LENGTH) == 0)) {
		const auto nChar = pBuffer[6 + TIMECODE_LENGTH];
		const auto bUseTimeCode = ((nChar == 'y') || (nChar == 'Y'));

		TCNet::Get()->SetUseTimeCode(bUseTimeCode);
		tcnet::display::show();

		DEBUG_PRINTF("tcnet!timecode#%c -> %d", nChar, static_cast<int>(bUseTimeCode));
		return;
	}

	DEBUG_PUTS("Invalid command");
}

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void TCNetReader::ResetTimer(const bool doReset, const struct tcnet::TimeCode *pTimeCode) {
	if (m_doResetTimer != doReset) {
		m_doResetTimer = doReset;

		if (m_doResetTimer) {
			memcpy(&m_timeCode, pTimeCode, sizeof(struct tcnet::TimeCode));

#if defined (H3)
			H3_TIMER->TMR0_CUR = 0;
			H3_TIMER->TMR0_INTV = TimeCodeConst::TMR_INTV[pTimeCode->nType];
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
			platform::ltc::timer11_set_type(pTimeCode->nType);
#endif
			gv_ltc_bTimeCodeAvailable = true;
		}
	}
}

void TCNetReader::Handler(const struct tcnet::TimeCode *pTimeCode) {
	if (__builtin_expect((pTimeCode->nType != m_nTypePrevious), 0)) {
		m_nTypePrevious = pTimeCode->nType;
		m_doResetTimer = false;
	}

	ResetTimer(!(pTimeCode->nFrames != 0), pTimeCode);

	gv_ltc_nUpdates = gv_ltc_nUpdates + 1;
}

void TCNetReader::Run() {
	// Update timecode outputs if available
	__DMB();  // Data memory barrier to ensure memory consistency
	if (__builtin_expect((gv_ltc_bTimeCodeAvailable), 0)) {
		if (ltc::Destination::IsEnabled(ltc::Destination::Output::LTC)) {
			LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct ltc::TimeCode *>(&m_timeCode));
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET)) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct artnet::TimeCode *>(&m_timeCode));
		}

		if (ltc::Destination::IsEnabled(ltc::Destination::Output::ETC)) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(&m_timeCode));
		}

		memcpy(&g_ltc_LtcTimeCode, &m_timeCode, sizeof(struct midi::Timecode));

		LtcOutputs::Get()->Update(const_cast<const struct ltc::TimeCode *>(&g_ltc_LtcTimeCode));

		m_timeCode.nFrames++;

		if (m_timeCode.nFrames >= TimeCodeConst::FPS[m_timeCode.nType]) {
#if defined (H3)
			H3_TIMER->TMR0_CTRL &= ~TIMER_CTRL_EN_START;
#elif defined (GD32)
			TIMER_CTL0(TIMER11) &= ~TIMER_CTL0_CEN;
#endif
			m_timeCode.nFrames = 0;
		}

		gv_ltc_bTimeCodeAvailable = false;
	}

	__DMB();
	if (gv_ltc_nUpdatesPerSecond != 0) {
		Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		Reset(false);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		Reset(true);
	}
}
