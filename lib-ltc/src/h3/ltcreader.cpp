/**
 * @file ltcreader.cpp
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
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "ltcreader.h"
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

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "midi.h"
#include "ltcetc.h"
#include "ltcoutputs.h"

#pragma GCC target ("general-regs-only")

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	380	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63
#define END_SYNC_POSITION	77
#define END_SMPTE_POSITION	80

static volatile bool IsMidiQuarterFrameMessage;
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

static volatile bool bTimeCodeAvailable;
static volatile struct midi::Timecode s_tMidiTimeCode = { 0, 0, 0, 0, static_cast<uint8_t>(midi::TimecodeType::EBU) };

// ARM Generic Timer
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static void __attribute__((interrupt("FIQ"))) fiq_handler() {
	dmb();

	nFiqUsCurrent = h3_hs_timer_lo_us();

	H3_PIO_PA_INT->STA = static_cast<uint32_t>(~0x0);

	if (nFiqUsPrevious >= nFiqUsCurrent) {
		nBitTime = nFiqUsPrevious - nFiqUsCurrent;
		nBitTime = 42949672 - nBitTime;
	} else {
		nBitTime = nFiqUsCurrent - nFiqUsPrevious;
	}

	nFiqUsPrevious = nFiqUsCurrent;

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
				aTimeCodeBits[0] = static_cast<uint8_t>(aTimeCodeBits[0] >> 1);
				for (uint32_t n = 1; n < 8; n++) {
					if (aTimeCodeBits[n] & 1) {
						aTimeCodeBits[n - 1] |= 0x80;
					}
					aTimeCodeBits[n] = static_cast<uint8_t>(aTimeCodeBits[n] >> 1);
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

			s_tMidiTimeCode.nFrames  = static_cast<uint8_t>((10 * (aTimeCodeBits[1] & 0x03)) + (aTimeCodeBits[0] & 0x0F));
			s_tMidiTimeCode.nSeconds = static_cast<uint8_t>((10 * (aTimeCodeBits[3] & 0x07)) + (aTimeCodeBits[2] & 0x0F));
			s_tMidiTimeCode.nMinutes = static_cast<uint8_t>((10 * (aTimeCodeBits[5] & 0x07)) + (aTimeCodeBits[4] & 0x0F));
			s_tMidiTimeCode.nHours   = static_cast<uint8_t>((10 * (aTimeCodeBits[7] & 0x03)) + (aTimeCodeBits[6] & 0x0F));

			bIsDropFrameFlagSet = (aTimeCodeBits[1] & (1 << 2));

			bTimeCodeAvailable = true;
			dmb();
		}
	}

	dmb();
}

static void arm_timer_handler(void) {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

static void irq_timer1_midi_handler(__attribute__((unused)) uint32_t clo) {
	IsMidiQuarterFrameMessage = true;
}

LtcReader::LtcReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs): m_ptLtcDisabledOutputs(pLtcDisabledOutputs), m_tTimeCodeTypePrevious(ltc::type::INVALID) {
}

void LtcReader::Start() {
	bTimeCodeAvailable = false;
	IsMidiQuarterFrameMessage = false;

	/**
	 * IRQ
	 */
	irq_timer_arm_physical_set(static_cast<thunk_irq_timer_arm_t>(arm_timer_handler));

	irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(irq_timer1_midi_handler));
	H3_TIMER->TMR1_CTRL &= ~TIMER_CTRL_SINGLE_MODE;

	irq_timer_init();

	/**
	 * FIQ
	 */
	h3_gpio_fsel(GPIO_EXT_26, GPIO_FSEL_EINT);

	arm_install_handler(reinterpret_cast<unsigned>(fiq_handler), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG1 = (GPIO_INT_CFG_DOUBLE_EDGE << 8);
	H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->STA = (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->DEB = 1;

	__enable_fiq();
}

void LtcReader::Run() {
	uint8_t TimeCodeType;

	dmb();
	if (bTimeCodeAvailable) {
		dmb();
		bTimeCodeAvailable = false;
		TimeCodeType = ltc::type::UNKNOWN;

		dmb();
		if (bIsDropFrameFlagSet) {
			TimeCodeType = ltc::type::DF;
		} else {
			if (nUpdatesPerSecond <= 24) {
				TimeCodeType = ltc::type::FILM;
			} else if (nUpdatesPerSecond <= 26) {
				TimeCodeType = ltc::type::EBU;
			} else if (nUpdatesPerSecond >= 28) {
				TimeCodeType = ltc::type::SMPTE;
			} else {
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
			RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(const_cast<struct midi::Timecode *>(&s_tMidiTimeCode)));
		}

		if (!m_ptLtcDisabledOutputs->bEtc) {
			LtcEtc::Get()->Send(reinterpret_cast<const struct midi::Timecode *>(const_cast<struct midi::Timecode *>(&s_tMidiTimeCode)));
		}

		if (m_tTimeCodeTypePrevious != TimeCodeType) {
			m_tTimeCodeTypePrevious = TimeCodeType;

			Midi::Get()->SendTimeCode(reinterpret_cast<const struct midi::Timecode *>(const_cast<struct midi::Timecode *>(&s_tMidiTimeCode)));

			H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[TimeCodeType] / 4;
			H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

			nMidiQuarterFramePiece = 0;
		}

		LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(&tLtcTimeCode));
	}

	dmb();
	if ((nUpdatesPerSecond >= 24) && (nUpdatesPerSecond <= 30)) {
		dmb();
		if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
			dmb();
			IsMidiQuarterFrameMessage = false;
			Midi::Get()->SendQf(reinterpret_cast<const struct midi::Timecode *>(const_cast<struct midi::Timecode *>(&s_tMidiTimeCode)), nMidiQuarterFramePiece);
		}
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
