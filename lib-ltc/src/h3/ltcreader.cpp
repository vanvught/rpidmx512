/**
 * @file ltcreader.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "h3/ltcreader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "h3.h"
#include "h3_board.h"
#include "h3_gpio.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#ifndef NDEBUG
 #include "console.h"
#endif

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "midi.h"
//
#include "h3/ltcoutputs.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	380	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63
#define END_SYNC_POSITION	77
#define END_SMPTE_POSITION	80

static volatile char aTimeCode[TC_CODE_MAX_LENGTH] ALIGNED;

static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static volatile bool IsMidiQuarterFrameMessage = false;
static uint32_t nMidiQuarterFramePiece = 0;

static volatile uint32_t nFiqUsPrevious = 0;
static volatile uint32_t nFiqUsCurrent = 0;

static volatile uint32_t nBitTime = 0;
static volatile uint32_t nTotalBits = 0;
static volatile bool bOnesBitCount = false;
static volatile uint32_t nCurrentBit = 0;
static volatile uint32_t nSyncCount = 0;
static volatile bool bTimeCodeSync = false;
static volatile bool bTimeCodeValid = false;

static volatile uint8_t aTimeCodeBits[8] ALIGNED;
static volatile bool bIsDropFrameFlagSet = false;

static volatile bool bTimeCodeAvailable = false;
static volatile struct _midi_send_tc s_tMidiTimeCode = { 0, 0, 0, 0, MIDI_TC_TYPE_EBU };

static void __attribute__((interrupt("FIQ"))) fiq_handler() {
	dmb();

	nFiqUsCurrent = h3_hs_timer_lo_us();

	H3_PIO_PA_INT->STA = static_cast<uint32_t>(~0x0);

	nBitTime = nFiqUsCurrent - nFiqUsPrevious;

	if ((nBitTime < ONE_TIME_MIN) || (nBitTime > ZERO_TIME_MAX)) {
		nTotalBits = 0;
	} else {
		if (bOnesBitCount) {
			bOnesBitCount = false;
		} else {
			if (nBitTime > ZERO_TIME_MIN) {
				nCurrentBit = 0;
				nSyncCount = 0;
			} else {
				nCurrentBit = 1;
				bOnesBitCount = true;
				nSyncCount++;

				if (nSyncCount == 12) {
					nSyncCount = 0;
					bTimeCodeSync = true;
					nTotalBits = END_SYNC_POSITION;
				}
			}

			if (nTotalBits <= END_DATA_POSITION) {
				aTimeCodeBits[0] = aTimeCodeBits[0] >> 1;
				for (uint32_t n = 1; n < 8; n++) {
					if (aTimeCodeBits[n] & 1) {
						aTimeCodeBits[n - 1] |= 0x80;
					}
					aTimeCodeBits[n] = aTimeCodeBits[n] >> 1;
				}

				if (nCurrentBit == 1) {
					aTimeCodeBits[7] |= 0x80;
				}
			}

			nTotalBits++;
		}

		if (nTotalBits == END_SMPTE_POSITION) {

			nTotalBits = 0;

			if (bTimeCodeSync) {
				bTimeCodeSync = false;
				bTimeCodeValid = true;
			}
		}

		if (bTimeCodeValid) {
			nUpdates++;

			bTimeCodeValid = false;

			s_tMidiTimeCode.nFrames  = (10 * (aTimeCodeBits[1] & 0x03)) + (aTimeCodeBits[0] & 0x0F);
			s_tMidiTimeCode.nSeconds = (10 * (aTimeCodeBits[3] & 0x07)) + (aTimeCodeBits[2] & 0x0F);
			s_tMidiTimeCode.nMinutes = (10 * (aTimeCodeBits[5] & 0x07)) + (aTimeCodeBits[4] & 0x0F);
			s_tMidiTimeCode.nHours   = (10 * (aTimeCodeBits[7] & 0x03)) + (aTimeCodeBits[6] & 0x0F);

			aTimeCode[10] = (aTimeCodeBits[0] & 0x0F) + '0';	// frames
			aTimeCode[9]  = (aTimeCodeBits[1] & 0x03) + '0';	// 10's of frames
			aTimeCode[7]  = (aTimeCodeBits[2] & 0x0F) + '0';	// seconds
			aTimeCode[6]  = (aTimeCodeBits[3] & 0x07) + '0';	// 10's of seconds
			aTimeCode[4]  = (aTimeCodeBits[4] & 0x0F) + '0';	// minutes
			aTimeCode[3]  = (aTimeCodeBits[5] & 0x07) + '0';	// 10's of minutes
			aTimeCode[1]  = (aTimeCodeBits[6] & 0x0F) + '0';	// hours
			aTimeCode[0]  = (aTimeCodeBits[7] & 0x03) + '0';	// 10's of hours

			bIsDropFrameFlagSet = (aTimeCodeBits[1] & (1 << 2));

			bTimeCodeAvailable = true;
		}
	}

	nFiqUsPrevious = nFiqUsCurrent;

	dmb();
}

static void irq_timer0_update_handler(__attribute__((unused)) uint32_t clo) {
	dmb();
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

static void irq_timer1_midi_handler(__attribute__((unused)) uint32_t clo) {
	IsMidiQuarterFrameMessage = true;
}

LtcReader::LtcReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_tTimeCodeTypePrevious(ltc::type::INVALID)
{
	Ltc::InitTimeCode(const_cast<char*>(aTimeCode));
}

void LtcReader::Start() {
	h3_gpio_fsel(GPIO_EXT_26, GPIO_FSEL_EINT);

	arm_install_handler(reinterpret_cast<unsigned>(fiq_handler), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG1 = (GPIO_INT_CFG_DOUBLE_EDGE << 8);
	H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->STA = (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->DEB = 1;

	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, static_cast<thunk_irq_timer_t>(irq_timer0_update_handler));
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(irq_timer1_midi_handler));
	H3_TIMER->TMR1_CTRL &= ~TIMER_CTRL_SINGLE_MODE;

	__enable_fiq();
}

void LtcReader::Run() {
	uint8_t TimeCodeType;
#ifndef NDEBUG
	char aLimitWarning[16] ALIGNED;
	uint32_t nLimitUs = 0;
	uint32_t nNowUs =  0;
#endif

	dmb();
	if (bTimeCodeAvailable) {
		dmb();
		bTimeCodeAvailable = false;

#ifndef NDEBUG
		nNowUs =  h3_hs_timer_lo_us();
#endif
		TimeCodeType = ltc::type::UNKNOWN;

		dmb();
		if (bIsDropFrameFlagSet) {
			TimeCodeType = ltc::type::DF;
#ifndef NDEBUG
			nLimitUs = (1000000.0 / 30);
#endif
		} else {
			if (nUpdatesPerSecond == 24) {
				TimeCodeType = ltc::type::FILM;
#ifndef NDEBUG
				nLimitUs = (1000000.0 / 24);
#endif
			} else if (nUpdatesPerSecond == 25) {
				TimeCodeType = ltc::type::EBU;
#ifndef NDEBUG
				nLimitUs = 1000000 / 25;
#endif
			} else if (nUpdatesPerSecond == 30) {
#ifndef NDEBUG
				nLimitUs = (1000000.0 / 30);
#endif
				TimeCodeType = ltc::type::SMPTE;
			}
		}

		s_tMidiTimeCode.nType = TimeCodeType;

		struct TLtcTimeCode tLtcTimeCode;

		tLtcTimeCode.nFrames = s_tMidiTimeCode.nFrames;
		tLtcTimeCode.nSeconds = s_tMidiTimeCode.nSeconds;
		tLtcTimeCode.nMinutes = s_tMidiTimeCode.nMinutes;
		tLtcTimeCode.nHours = s_tMidiTimeCode.nHours;
		tLtcTimeCode.nType = s_tMidiTimeCode.nType;

		if (!m_ptLtcDisabledOutputs->bArtNet) {
			ArtNetNode::Get()->SendTimeCode(reinterpret_cast<const struct TArtNetTimeCode*>(&tLtcTimeCode));
		}

		if (!m_ptLtcDisabledOutputs->bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(const_cast<struct _midi_send_tc*>(&s_tMidiTimeCode)));
		}

		if (m_tTimeCodeTypePrevious != TimeCodeType) {
			m_tTimeCodeTypePrevious = TimeCodeType;

			Midi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(const_cast<struct _midi_send_tc*>(&s_tMidiTimeCode)));

			H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[TimeCodeType] / 4;
			H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

			nMidiQuarterFramePiece = 0;
		}

		LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&tLtcTimeCode));

#ifndef NDEBUG
		const uint32_t delta_us = h3_hs_timer_lo_us() - nNowUs;

		if (nLimitUs == 0) {
			sprintf(aLimitWarning, "%.2d:-----:%.5d",
					static_cast<int>(nUpdatesPerSecond),
					static_cast<int>(delta_us));
			console_status(CONSOLE_CYAN, aLimitWarning);
		} else {
			sprintf(aLimitWarning, "%.2d:%.5d:%.5d",
					static_cast<int>(nUpdatesPerSecond),
					static_cast<int>(nLimitUs),
					static_cast<int>(delta_us));
			console_status(delta_us < nLimitUs ? CONSOLE_YELLOW : CONSOLE_RED, aLimitWarning);
		}
#endif
	}

	dmb();
	if ((nUpdatesPerSecond >= 24) && (nUpdatesPerSecond <= 30)) {
		dmb();
		if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
			dmb();
			IsMidiQuarterFrameMessage = false;
			Midi::Get()->SendQf(reinterpret_cast<const struct _midi_send_tc*>(const_cast<struct _midi_send_tc*>(&s_tMidiTimeCode)), nMidiQuarterFramePiece);
		}
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
