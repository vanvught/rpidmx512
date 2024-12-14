/**
 * @file ltcoutputs.cpp
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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <cassert>

#include "arm/ltcoutputs.h"

#include "ltc.h"
#include "timecodeconst.h"

// Outputs
#include "ltcsender.h"
#include "rtpmidi.h"
#include "midi.h"
#include "ntpserver.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#if !(defined(CONFIG_LTC_DISABLE_RGB_PANEL) && defined (CONFIG_LTC_DISABLE_WS28XX))
# include "ltcdisplayrgb.h"
#else
# define LTC_NO_DISPLAY_RGB
#endif

#include "arm/platform_ltc.h"

static volatile bool sv_isMidiQuarterFrameMessage;

#if defined (H3)
static void irq_timer1_midi_handler([[maybe_unused]] uint32_t clo) {
	sv_isMidiQuarterFrameMessage = true;
}
#elif defined (GD32)
extern "C" {
void TIMER0_TRG_CMT_TIMER10_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER10);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		sv_isMidiQuarterFrameMessage = true;
#if defined (DEBUG_LTC_TIMER10)
		GPIO_TG(DEBUG_TIMER10_GPIOx) = DEBUG_TIMER10_GPIO_PINx;
#endif
	}

	timer_interrupt_flag_clear(TIMER10, nIntFlag);
}
}
#endif

LtcOutputs *LtcOutputs::s_pThis;

LtcOutputs::LtcOutputs(const ltc::Source source, const bool bShowSysTime): m_bShowSysTime(bShowSysTime) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(m_cBPM, 0, sizeof(m_cBPM));
	memcpy(m_cBPM, "BPM: ", 5);

	ltc::g_DisabledOutputs.bMidi |= (source == ltc::Source::MIDI);
	ltc::g_DisabledOutputs.bArtNet |= (source == ltc::Source::ARTNET);
	ltc::g_DisabledOutputs.bLtc |= (source == ltc::Source::LTC);
	ltc::g_DisabledOutputs.bRtpMidi |= (source == ltc::Source::APPLEMIDI);
	ltc::g_DisabledOutputs.bEtc |= (source == ltc::Source::ETC);
	// Display's
	ltc::g_DisabledOutputs.bRgbPanel |= ((source == ltc::Source::LTC) || (source == ltc::Source::MIDI));
	ltc::g_DisabledOutputs.bMax7219 |= (!ltc::g_DisabledOutputs.bWS28xx || !ltc::g_DisabledOutputs.bRgbPanel);
	ltc::g_DisabledOutputs.bOled |= (!ltc::g_DisabledOutputs.bRgbPanel);
	// Do not change the order
	ltc::g_DisabledOutputs.bMidi |= (!ltc::g_DisabledOutputs.bRgbPanel);
	ltc::g_DisabledOutputs.bLtc |= (!ltc::g_DisabledOutputs.bRgbPanel);

	ltc::init_timecode(m_aTimeCode);
	ltc::init_systemtime(m_aSystemTime);
}

void LtcOutputs::Init() {
	if ((!ltc::g_DisabledOutputs.bMidi) || (!ltc::g_DisabledOutputs.bRtpMidi)) {
#if defined (H3)
		irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(irq_timer1_midi_handler));
#elif defined (GD32)
		platform::ltc::timer10_config();
#endif
	}

	if (!ltc::g_DisabledOutputs.bOled) {
		Display::Get()->TextLine(2, ltc::get_type(ltc::Type::UNKNOWN), ltc::timecode::TYPE_MAX_LENGTH);
	}
}

void LtcOutputs::Update(const struct ltc::TimeCode *ptLtcTimeCode) {
	assert(ptLtcTimeCode != nullptr);

	if (ptLtcTimeCode->nType != static_cast<uint8_t>(m_TypePrevious)) {
		m_TypePrevious = static_cast<ltc::Type>(ptLtcTimeCode->nType);

		if (!ltc::g_DisabledOutputs.bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(ptLtcTimeCode));
		}

		if (!ltc::g_DisabledOutputs.bMidi) {
			Midi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(ptLtcTimeCode));
		}

#if defined (H3)
		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[ptLtcTimeCode->nType] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
		TIMER_CAR(TIMER10) = TimeCodeConst::TMR_INTV[ptLtcTimeCode->nType] / 4;
		TIMER_CNT(TIMER10) = 0;
		TIMER_CTL0(TIMER10) |= TIMER_CTL0_CEN;
#endif

		m_nMidiQuarterFramePiece = 0;
		m_nRtpMidiQuarterFramePiece = 0;

		if (!ltc::g_DisabledOutputs.bOled) {
			Display::Get()->TextLine(2, ltc::get_type(static_cast<ltc::Type>(ptLtcTimeCode->nType)), ltc::timecode::TYPE_MAX_LENGTH);
		}

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
		if (!ltc::g_DisabledOutputs.bRgbPanel) {
			LtcDisplayRgb::Get()->ShowFPS(static_cast<ltc::Type>(ptLtcTimeCode->nType));
		}
#endif

		m_aTimeCode[ltc::timecode::index::COLON_3] = (ptLtcTimeCode->nType != static_cast<uint8_t>(ltc::Type::DF) ? ':' : ';');
	}

	if (!ltc::g_DisabledOutputs.bNtp) {
		NtpServer::Get()->SetTimeCode(ptLtcTimeCode);
	}

	ltc::itoa_base10(ptLtcTimeCode, m_aTimeCode);

	if (!ltc::g_DisabledOutputs.bOled) {
		Display::Get()->TextLine(1, m_aTimeCode, ltc::timecode::CODE_MAX_LENGTH);
	}

	if (!ltc::g_DisabledOutputs.bMax7219) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	}

#if !defined(LTC_NO_DISPLAY_RGB)
	if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}
#endif
}

void LtcOutputs::UpdateMidiQuarterFrameMessage(const struct ltc::TimeCode *pltcTimeCode) {
	__DMB();

	if (__builtin_expect((sv_isMidiQuarterFrameMessage), 0)) {
		sv_isMidiQuarterFrameMessage = false;

		if (!ltc::g_DisabledOutputs.bRtpMidi) {
			RtpMidi::Get()->SendQf(reinterpret_cast<const struct midi::Timecode *>(pltcTimeCode), m_nRtpMidiQuarterFramePiece);
		}

		if (!ltc::g_DisabledOutputs.bMidi) {
			Midi::Get()->SendQf(reinterpret_cast<const struct midi::Timecode *>(pltcTimeCode), m_nMidiQuarterFramePiece);
		}
	}
}

void LtcOutputs::ShowSysTime() {
	if (m_bShowSysTime) {
		const auto nTime = time(nullptr);
		const auto *pTm = localtime(&nTime);

		if (__builtin_expect((m_nSecondsPrevious == pTm->tm_sec), 1)) {
			return;
		}

		m_nSecondsPrevious = pTm->tm_sec;

		ltc::itoa_base10(pTm, m_aSystemTime);

		if (!ltc::g_DisabledOutputs.bOled) {
			Display::Get()->TextLine(1, m_aSystemTime, ltc::timecode::SYSTIME_MAX_LENGTH);
		}

		if (!ltc::g_DisabledOutputs.bMax7219) {
			LtcDisplayMax7219::Get()->ShowSysTime(m_aSystemTime);
		}

#if !defined(LTC_NO_DISPLAY_RGB)
		if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
			LtcDisplayRgb::Get()->ShowSysTime(m_aSystemTime);
		}
#endif
		ResetTimeCodeTypePrevious();
	}
}

void LtcOutputs::ShowBPM(uint32_t nBPM) {
	if ((nBPM < midi::bpm::MIN) || (nBPM > midi::bpm::MAX)) {
		m_cBPM[5] = '-';
		m_cBPM[6] = '-';
		m_cBPM[7] = '-';
	} else {
		m_cBPM[7] = static_cast<char>(nBPM % 10U + '0');
		nBPM /= 10U;
		const uint32_t nDigit = nBPM % 10U;

		if (nBPM != 0) {
			m_cBPM[6] = static_cast<char>(nDigit + '0');
			nBPM /= 10U;
			const uint32_t nDigit = nBPM % 10U;

			if (nBPM != 0) {
				m_cBPM[5] = static_cast<char>(nDigit + '0');
			} else {
				m_cBPM[5] = ' ';
			}
		} else {
			m_cBPM[6] = ' ';
			m_cBPM[5] = ' ';
		}
	}

	if (!ltc::g_DisabledOutputs.bOled) {
		Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 3U), 1);
		Display::Get()->PutString(&m_cBPM[5]);
	}

#if !defined(LTC_NO_DISPLAY_RGB)
	if (!ltc::g_DisabledOutputs.bRgbPanel) {
		LtcDisplayRgb::Get()->ShowInfo(m_cBPM);
	}
#endif
}

static void print_disabled(const bool isDisabled, const char *pString) {
	if (isDisabled) {
		printf(" %s output is disabled\n", pString);
	}
}

void LtcOutputs::Print() {
	print_disabled(ltc::g_DisabledOutputs.bLtc, "LTC");
	print_disabled(ltc::g_DisabledOutputs.bArtNet, "Art-Net");
	print_disabled(ltc::g_DisabledOutputs.bRtpMidi, "AppleMIDI");
	print_disabled(ltc::g_DisabledOutputs.bMidi, "MIDI");
	print_disabled(ltc::g_DisabledOutputs.bEtc, "ETC");
	print_disabled(ltc::g_DisabledOutputs.bNtp, "NTP");
	print_disabled(ltc::g_DisabledOutputs.bOled, "OLED");
	print_disabled(ltc::g_DisabledOutputs.bMax7219, "Max7219");
	print_disabled(ltc::g_DisabledOutputs.bWS28xx, "WS28xx");
	print_disabled(ltc::g_DisabledOutputs.bRgbPanel, "RGB panel");
}
