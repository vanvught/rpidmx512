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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <cassert>

#include "ltcoutputs.h"

#include "ltc.h"
#include "timecodeconst.h"

// Outputs
#include "ltcsender.h"
#include "rtpmidi.h"
#include "midi.h"
#include "ntpserver.h"
#include "ltc7segment.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#if !(defined(CONFIG_LTC_DISABLE_RGB_PANEL) && defined (CONFIG_LTC_DISABLE_WS28XX))
# include "ltcdisplayrgb.h"
#else
# define LTC_NO_DISPLAY_RGB
#endif

#include "platform_ltc.h"

static volatile bool sv_isMidiQuarterFrameMessage;

#if defined (H3)
static void irq_timer1_midi_handler([[maybe_unused]] uint32_t clo) {
	sv_isMidiQuarterFrameMessage = true;
}
#elif defined (GD32)
void TIMER12_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER12);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
		sv_isMidiQuarterFrameMessage = true;
	}

	timer_interrupt_flag_clear(TIMER12, nIntFlag);
}
#endif

LtcOutputs *LtcOutputs::s_pThis;

LtcOutputs::LtcOutputs(ltc::Source source, bool bShowSysTime): m_bShowSysTime(bShowSysTime) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(m_cBPM, 0, sizeof(m_cBPM));
	memcpy(m_cBPM, "BPM: ", 5);

	g_ltc_ptLtcDisabledOutputs.bMidi |= (source == ltc::Source::MIDI);
	g_ltc_ptLtcDisabledOutputs.bArtNet |= (source == ltc::Source::ARTNET);
	g_ltc_ptLtcDisabledOutputs.bLtc |= (source == ltc::Source::LTC);
	g_ltc_ptLtcDisabledOutputs.bRtpMidi |= (source == ltc::Source::APPLEMIDI);
	g_ltc_ptLtcDisabledOutputs.bEtc |= (source == ltc::Source::ETC);
	// Display's
	g_ltc_ptLtcDisabledOutputs.bRgbPanel |= ((source == ltc::Source::LTC) || (source == ltc::Source::MIDI));
	g_ltc_ptLtcDisabledOutputs.bMax7219 |= (!g_ltc_ptLtcDisabledOutputs.bWS28xx || !g_ltc_ptLtcDisabledOutputs.bRgbPanel);
	g_ltc_ptLtcDisabledOutputs.bOled |= (!g_ltc_ptLtcDisabledOutputs.bRgbPanel);
	//
	g_ltc_ptLtcDisabledOutputs.bMidi |= (!g_ltc_ptLtcDisabledOutputs.bRgbPanel);
	g_ltc_ptLtcDisabledOutputs.bLtc |= (!g_ltc_ptLtcDisabledOutputs.bRgbPanel);

	ltc::init_timecode(m_aTimeCode);
	ltc::init_systemtime(m_aSystemTime);
}

void LtcOutputs::Init() {
	if (!g_ltc_ptLtcDisabledOutputs.bMidi) {
#if defined (H3)
		irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(irq_timer1_midi_handler));
#elif defined (GD32)
#endif
	}

	if (!g_ltc_ptLtcDisabledOutputs.bOled) {
		Display::Get()->TextLine(2, ltc::get_type(ltc::Type::UNKNOWN), ltc::timecode::TYPE_MAX_LENGTH);
	}
}

void LtcOutputs::Update(const struct ltc::TimeCode *ptLtcTimeCode) {
	assert(ptLtcTimeCode != nullptr);

	if (!g_ltc_ptLtcDisabledOutputs.bNtp) {
		NtpServer::Get()->SetTimeCode(ptLtcTimeCode);
	}

	if (ptLtcTimeCode->nType != static_cast<uint8_t>(m_tTimeCodeTypePrevious)) {
		m_tTimeCodeTypePrevious = static_cast<ltc::Type>(ptLtcTimeCode->nType);

		if (!g_ltc_ptLtcDisabledOutputs.bMidi) {
			Midi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(ptLtcTimeCode));
		}

#if defined (H3)
		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[ptLtcTimeCode->nType] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
#elif defined (GD32)
		TIMER_CNT(TIMER11) = 0;
		TIMER_CH0CV(TIMER11) = TimeCodeConst::TMR_INTV[ptLtcTimeCode->nType] / 4;
#endif

		m_nMidiQuarterFramePiece = 0;

		if (!g_ltc_ptLtcDisabledOutputs.bOled) {
			Display::Get()->TextLine(2, ltc::get_type(static_cast<ltc::Type>(ptLtcTimeCode->nType)), ltc::timecode::TYPE_MAX_LENGTH);
		}

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
		if (!g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
			LtcDisplayRgb::Get()->ShowFPS(static_cast<ltc::Type>(ptLtcTimeCode->nType));
		}
#endif
		Ltc7segment::Get()->Show(static_cast<ltc::Type>(ptLtcTimeCode->nType));

		m_aTimeCode[ltc::timecode::index::COLON_3] = (ptLtcTimeCode->nType != static_cast<uint8_t>(ltc::Type::DF) ? ':' : ';');
	}

	ltc::itoa_base10(ptLtcTimeCode, m_aTimeCode);

	if (!g_ltc_ptLtcDisabledOutputs.bOled) {
		Display::Get()->TextLine(1, m_aTimeCode, ltc::timecode::CODE_MAX_LENGTH);
	}

	if (!g_ltc_ptLtcDisabledOutputs.bMax7219) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	}

#if !defined(LTC_NO_DISPLAY_RGB)
	if ((!g_ltc_ptLtcDisabledOutputs.bWS28xx) || (!g_ltc_ptLtcDisabledOutputs.bRgbPanel)) {
		LtcDisplayRgb::Get()->Show(m_aTimeCode);
	}
#endif
}

void LtcOutputs::UpdateMidiQuarterFrameMessage(const struct ltc::TimeCode *pltcTimeCode) {
	__DMB();

	if (__builtin_expect((sv_isMidiQuarterFrameMessage), 0)) {
		sv_isMidiQuarterFrameMessage = false;
		Midi::Get()->SendQf(reinterpret_cast<const struct midi::Timecode *>(pltcTimeCode), m_nMidiQuarterFramePiece);
	}
}

void LtcOutputs::ShowSysTime() {
	if (m_bShowSysTime) {
		const auto tTime = time(nullptr);
		const auto pLocalTime = localtime(&tTime);

		if (__builtin_expect((m_nSecondsPrevious == pLocalTime->tm_sec), 1)) {
			return;
		}

		m_nSecondsPrevious = pLocalTime->tm_sec;

		ltc::itoa_base10(pLocalTime, m_aSystemTime);

		if (!g_ltc_ptLtcDisabledOutputs.bOled) {
			Display::Get()->TextLine(1, m_aSystemTime, ltc::timecode::SYSTIME_MAX_LENGTH);
		}

		Ltc7segment::Get()->Show(ltc::Type::UNKNOWN);

		if (!g_ltc_ptLtcDisabledOutputs.bMax7219) {
			LtcDisplayMax7219::Get()->ShowSysTime(m_aSystemTime);
		}

#if !defined(LTC_NO_DISPLAY_RGB)
		if ((!g_ltc_ptLtcDisabledOutputs.bWS28xx) || (!g_ltc_ptLtcDisabledOutputs.bRgbPanel)) {
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
		m_cBPM[7] = static_cast<char>(nBPM % 10 + '0');
		nBPM /= 10;
		const uint32_t nDigit = nBPM % 10;

		if (nBPM != 0) {
			m_cBPM[6] = static_cast<char>(nDigit + '0');
			nBPM /= 10;
			const uint32_t nDigit = nBPM % 10;

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

	if (!g_ltc_ptLtcDisabledOutputs.bOled) {
		Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 3U), 1);
		Display::Get()->PutString(&m_cBPM[5]);
	}

#if !defined(LTC_NO_DISPLAY_RGB)
	if (!g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
		LtcDisplayRgb::Get()->ShowInfo(m_cBPM);
	}
#endif
}

static void print_disabled(const bool IsDisabled, const char *pString) {
	if (IsDisabled) {
		printf(" %s output is disabled\n", pString);
	}
}

void LtcOutputs::Print() {
	print_disabled(g_ltc_ptLtcDisabledOutputs.bLtc, "LTC");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bArtNet, "Art-Net");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bRtpMidi, "AppleMIDI");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bMidi, "MIDI");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bEtc, "ETC");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bNtp, "NTP");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bOled, "OLED");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bMax7219, "Max7219");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bWS28xx, "WS28xx");
	print_disabled(g_ltc_ptLtcDisabledOutputs.bRgbPanel, "RGB panel");
}
