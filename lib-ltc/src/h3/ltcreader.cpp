/**
 * @file ltcreader.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "h3/ltcreader.h"
#include "ltc.h"

#include "hardwarebaremetal.h"

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
#include "ltcleds.h"
#include "artnetnode.h"
#include "display.h"
#include "displaymax7219.h"
#include "midi.h"
#include "ntpserver.h"

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

static volatile uint32_t nLedToggle = 0;

static volatile uint32_t nMidiQuarterFrameUs = 0;
static volatile bool IsMidiQuarterFrameMessage = false;
static volatile uint32_t nMidiQuarterFramePiece = 0;

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
static volatile struct _midi_send_tc tMidiTimeCode = { 0, 0, 0, 0, MIDI_TC_TYPE_EBU };

static void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	dmb();

	nFiqUsCurrent = h3_hs_timer_lo_us();

	H3_PIO_PA_INT->STA = ~0x0;

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
						aTimeCodeBits[n - 1] |= (uint8_t) 0x80;
					}
					aTimeCodeBits[n] = aTimeCodeBits[n] >> 1;
				}

				if (nCurrentBit == 1) {
					aTimeCodeBits[7] |= (uint8_t) 0x80;
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

			tMidiTimeCode.frame  = (10 * (aTimeCodeBits[1] & 0x03)) + (aTimeCodeBits[0] & 0x0F);
			tMidiTimeCode.second = (10 * (aTimeCodeBits[3] & 0x07)) + (aTimeCodeBits[2] & 0x0F);
			tMidiTimeCode.minute = (10 * (aTimeCodeBits[5] & 0x07)) + (aTimeCodeBits[4] & 0x0F);
			tMidiTimeCode.hour   = (10 * (aTimeCodeBits[7] & 0x03)) + (aTimeCodeBits[6] & 0x0F);

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

static void irq_timer0_update_handler(uint32_t clo) {
	dmb();
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;

	if ((nUpdatesPerSecond >= 24) && (nUpdatesPerSecond <= 30)) {
		if (nLedToggle++ & 0x01) {
			Hardware::Get()->SetLed(HARDWARE_LED_ON);
		} else {
			Hardware::Get()->SetLed(HARDWARE_LED_OFF);
		}
	} else {
		Hardware::Get()->SetLed(HARDWARE_LED_ON);
	}
}

static void irq_timer1_midi_handler(uint32_t clo) {
	H3_TIMER->TMR1_INTV = nMidiQuarterFrameUs * 12;
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	dmb();
	IsMidiQuarterFrameMessage = true;
}

LtcReader::LtcReader(ArtNetNode *pNode, struct TLtcDisabledOutputs *pLtcDisabledOutputs):
	m_pNode(pNode),
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_tTimeCodeTypePrevious(TC_TYPE_INVALID)
{
	for (uint32_t i = 0; i < sizeof(aTimeCode) / sizeof(aTimeCode[0]) ; i++) {
		aTimeCode[i] = ' ';
	}

	aTimeCode[2] = ':';
	aTimeCode[5] = ':';
	aTimeCode[8] = '.';
}

LtcReader::~LtcReader(void) {
}

void LtcReader::Start(void) {
	h3_gpio_fsel(GPIO_EXT_26, GPIO_FSEL_EINT);

	arm_install_handler((unsigned) fiq_handler, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG1 = (0x4 << 8);
	H3_PIO_PA_INT->CTL = (1 << GPIO_EXT_26);
	H3_PIO_PA_INT->STA = ~0x0;
	H3_PIO_PA_INT->DEB = 1;

	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, irq_timer0_update_handler);
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	irq_timer_set(IRQ_TIMER_1, irq_timer1_midi_handler);
	H3_TIMER->TMR1_CTRL |= TIMER_CTRL_SINGLE_MODE;

	__enable_fiq();
}

void LtcReader::Run(void) {
	uint8_t TimeCodeType;
	char *pTimeCodeType;
	uint32_t nLimitUs =  0;
#ifndef NDEBUG
	char aLimitWarning[16] ALIGNED;
	uint32_t nNowUs =  0;
#endif

	dmb();
	if (bTimeCodeAvailable) {
		dmb();
		bTimeCodeAvailable = false;

#ifndef NDEBUG
		nNowUs =  h3_hs_timer_lo_us();
#endif
		TimeCodeType = TC_TYPE_UNKNOWN;

		dmb();
		if (bIsDropFrameFlagSet) {
			TimeCodeType = TC_TYPE_DF;
			nLimitUs = (uint32_t) ((double) 1000000 / (double) 30);
		} else {
			if (nUpdatesPerSecond == 24) {
				TimeCodeType = TC_TYPE_FILM;
				nLimitUs = (uint32_t) ((double) 1000000 / (double) 24);
			} else if (nUpdatesPerSecond == 25) {
				TimeCodeType = TC_TYPE_EBU;
				nLimitUs = (uint32_t) ((double) 1000000 / (double) 25);
			} else if (nUpdatesPerSecond == 30) {
				nLimitUs = (uint32_t) ((double) 1000000 / (double) 30);
				TimeCodeType = TC_TYPE_SMPTE;
			}
		}

		tMidiTimeCode.rate = (_midi_timecode_type)TimeCodeType;

		if (!m_ptLtcDisabledOutputs->bArtNet) {
			m_pNode->SendTimeCode((const struct TArtNetTimeCode *) &tMidiTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bNtp) {
			NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *) &tMidiTimeCode);
		}

		if (m_tTimeCodeTypePrevious != TimeCodeType) {
			m_tTimeCodeTypePrevious = TimeCodeType;

			Midi::Get()->SendTimeCode((const struct _midi_send_tc *) &tMidiTimeCode);

			nMidiQuarterFramePiece = 0;
			nMidiQuarterFrameUs = nLimitUs / 4;

			H3_TIMER->TMR1_INTV = nMidiQuarterFrameUs * 12;
			H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

			pTimeCodeType = (char *) Ltc::GetType((TTimecodeTypes) TimeCodeType);

			if (!m_ptLtcDisabledOutputs->bDisplay) {
				Display::Get()->TextLine(2, pTimeCodeType, TC_TYPE_MAX_LENGTH);
			}
			LtcLeds::Get()->Show((TTimecodeTypes) TimeCodeType);
		}

		if (!m_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(1, (const char *) aTimeCode, TC_CODE_MAX_LENGTH);
		}
		if (!m_ptLtcDisabledOutputs->bMax7219) {
			DisplayMax7219::Get()->Show((const char *) aTimeCode);
		}

#ifndef NDEBUG
		const uint32_t delta_us = h3_hs_timer_lo_us() - nNowUs;

		if (nLimitUs == 0) {
			sprintf(aLimitWarning, "%.2d:-----:%.5d", (int) nUpdatesPerSecond, (int) delta_us);
			console_status(CONSOLE_CYAN, aLimitWarning);
		} else {
			sprintf(aLimitWarning, "%.2d:%.5d:%.5d", (int) nUpdatesPerSecond, (int) nLimitUs, (int) delta_us);
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

			uint8_t bytes[2] = { 0xF1, 0x00 };
			const uint8_t data = nMidiQuarterFramePiece << 4;

			switch (nMidiQuarterFramePiece) {
				case 0:
					bytes[1] = data | (tMidiTimeCode.frame & 0x0F);
					break;
				case 1:
					bytes[1] = data | ((tMidiTimeCode.frame & 0x10) >> 4);
					break;
				case 2:
					bytes[1] = data | (tMidiTimeCode.second & 0x0F);
					break;
				case 3:
					bytes[1] = data | ((tMidiTimeCode.second & 0x30) >> 4);
					break;
				case 4:
					bytes[1] = data | (tMidiTimeCode.minute & 0x0F);
					break;
				case 5:
					bytes[1] = data | ((tMidiTimeCode.minute & 0x30) >> 4);
					break;
				case 6:
					bytes[1] = data | (tMidiTimeCode.hour & 0x0F);
					break;
				case 7:
					bytes[1] = data | (tMidiTimeCode.rate << 1) |((tMidiTimeCode.hour & 0x10) >> 4);;
					break;
				default:
					break;
			}

			Midi::Get()->SendRaw(bytes, 2);
			nMidiQuarterFramePiece = (nMidiQuarterFramePiece + 1) & 0x07;
		}
	}
}
