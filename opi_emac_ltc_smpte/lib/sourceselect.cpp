/**
 * @file sourceselect.cpp
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
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "hardware.h"
#include "network.h"

#include "arm/synchronize.h"

#include "sourceselect.h"
#include "sourceselectconst.h"

#include "ltcparams.h"

#include "display.h"
#include "display7segment.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

#include "rotaryencoder.h"

#include "hal_i2c.h"
#include "device/mcp23x17.h"

#include "h3/ltcgenerator.h"

// Interrupt
#include "board/h3_opi_zero.h"
#include "h3_gpio.h"	//TODO Remove H3 dependency

#include "spiflashstore.h"
#include "storeltc.h"

#include "debug.h"

static constexpr Display7SegmentMessage s_7Segment[] = {
		Display7SegmentMessage::GENERIC_1,
		Display7SegmentMessage::GENERIC_2,
		Display7SegmentMessage::GENERIC_3,
		Display7SegmentMessage::GENERIC_4,
		Display7SegmentMessage::GENERIC_5,
		Display7SegmentMessage::GENERIC_6,
		Display7SegmentMessage::GENERIC_7,
		Display7SegmentMessage::GENERIC_8
};

namespace mcp23017 {
static constexpr auto I2C_ADDRESS = 0x20;
}

namespace gpio {
static constexpr auto INTA = GPIO_EXT_12; // PA7
}

namespace button {
static constexpr auto SELECT = 2;
static constexpr auto LEFT = 3;
static constexpr auto RIGHT = 4;
static constexpr auto START = 5;
static constexpr auto STOP = 6;
static constexpr auto RESUME = 7;
}

#define BUTTON(x)			((nButtonsChanged >> x) & 0x01)
#define BUTTON_STATE(x)		((nButtonsChanged & (1U << x)) == (1U << x))

SourceSelect::SourceSelect(TLtcReaderSource tLtcReaderSource, struct TLtcDisabledOutputs *ptLtcDisabledOutput):
	m_I2C(mcp23017::I2C_ADDRESS),
	m_tLtcReaderSource(tLtcReaderSource),
	m_ptLtcDisabledOutputs(ptLtcDisabledOutput),
	m_bIsConnected(false),
	m_nPortA(0),
	m_nPortAPrevious(0),
	m_nPortB(0),
	m_nMillisPrevious(0),
	m_tRotaryDirection(RotaryEncoder::NONE),
	m_tRunStatus(RUN_STATUS_IDLE),
	m_nSelectMillis(0)
{
}

SourceSelect::~SourceSelect(void) {
}

void SourceSelect::LedBlink(uint8_t nPortB) {
	const uint32_t nMillisNow = Hardware::Get()->Millis();

	if (__builtin_expect(((nMillisNow - m_nMillisPrevious) < 500), 1)) {
		return;
	}

	m_nMillisPrevious = nMillisNow;
	m_nPortB ^= nPortB;
	m_I2C.WriteRegister(mcp23x17::GPIOB, m_nPortB);
}

void SourceSelect::HandleActionLeft(TLtcReaderSource &tLtcReaderSource) {
	if (tLtcReaderSource == 0) {
		tLtcReaderSource = static_cast<TLtcReaderSource>((LTC_READER_SOURCE_UNDEFINED - 1));
	} else {
		tLtcReaderSource = static_cast<TLtcReaderSource>((tLtcReaderSource - 1));
	}
}

void SourceSelect::HandleActionRight(TLtcReaderSource &tLtcReaderSource) {
	if (tLtcReaderSource == static_cast<TLtcReaderSource>((LTC_READER_SOURCE_UNDEFINED - 1))) {
		tLtcReaderSource = static_cast<TLtcReaderSource>(0);
	} else {
		tLtcReaderSource = static_cast<TLtcReaderSource>((tLtcReaderSource + 1));
	}
}

void SourceSelect::HandleRotary(uint8_t nInputAB, TLtcReaderSource &tLtcReaderSource) {
	m_tRotaryDirection = m_RotaryEncoder.Process(nInputAB);

	if (m_tRotaryDirection == RotaryEncoder::CW) {
		HandleActionRight(tLtcReaderSource);
	} else if (m_tRotaryDirection == RotaryEncoder::CCW) {
		HandleActionLeft(tLtcReaderSource);
	}
}

void SourceSelect::UpdateDisaplays(TLtcReaderSource tLtcReaderSource) {
	Display::Get()->TextStatus(SourceSelectConst::SOURCE[tLtcReaderSource], s_7Segment[tLtcReaderSource]);

	if (!m_ptLtcDisabledOutputs->bMax7219) {
		LtcDisplayMax7219::Get()->WriteChar(tLtcReaderSource);
	}

	if(!m_ptLtcDisabledOutputs->bWS28xx) {
		LtcDisplayWS28xx::Get()->WriteChar(tLtcReaderSource);
	}
}

bool SourceSelect::Check(void) {
	DEBUG_ENTRY

	m_bIsConnected = m_I2C.IsConnected();

	if (!m_bIsConnected) {
		DEBUG_EXIT
		return false;
	}

	// Rotary and switches
	m_I2C.WriteRegister(mcp23x17::IODIRA, static_cast<uint8_t>(0xFF)); 	// All input
	m_I2C.WriteRegister(mcp23x17::GPPUA, static_cast<uint8_t>(0xFF));	// Pull-up
	m_I2C.WriteRegister(mcp23x17::GPINTENA, static_cast<uint8_t>(0xFF));// Interrupt on Change
	m_I2C.ReadRegister(mcp23x17::INTCAPA);								// Clear interrupts
	// Led's
	m_I2C.WriteRegister(mcp23x17::IODIRB, static_cast<uint8_t>(0x00)); 	// All output
	m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(1u << m_tLtcReaderSource));

	UpdateDisaplays(m_tLtcReaderSource);

	h3_gpio_fsel(gpio::INTA, GPIO_FSEL_INPUT); // PA7
	h3_gpio_pud(gpio::INTA, GPIO_PULL_UP);

	DEBUG_EXIT
	return true;
}

bool SourceSelect::Wait(TLtcReaderSource &tLtcReaderSource) {
	LedBlink(1 << tLtcReaderSource);

	if (__builtin_expect(h3_gpio_lev(gpio::INTA) == LOW, 0)) {

		m_nPortAPrevious = m_nPortA;
		m_nPortA = ~m_I2C.ReadRegister(mcp23x17::GPIOA);

		const uint8_t nButtonsChanged = (m_nPortA ^ m_nPortAPrevious) & m_nPortA;

		/* P = m_nPortAPrevious
		 * N = m_nPortA
		 * X = m_nPortA ^ m_nPortAPrevious
		 * C = nButtonsChanged
		 *
		 * P N	X N	C
		 * 0 0	0 0	0
		 * 0 1	1 1	1
		 * 1 0	1 0	0
		 * 1 1	0 1	0
		 */

		if (BUTTON_STATE(button::LEFT)) {
			HandleActionLeft(tLtcReaderSource);
		} else if (BUTTON_STATE(button::RIGHT)) {
			HandleActionRight(tLtcReaderSource);
		} else if (BUTTON_STATE(button::SELECT)) {
			if (m_tLtcReaderSource != tLtcReaderSource) {
				m_tLtcReaderSource = tLtcReaderSource;
				StoreLtc::Get()->SaveSource(m_tLtcReaderSource);
			}

			m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(1u << tLtcReaderSource));
			m_I2C.ReadRegister(mcp23x17::INTCAPA);	// Clear interrupts
			return false;
		} else {
			HandleRotary(m_nPortA, tLtcReaderSource);
		}

		m_nPortB = 0;

		UpdateDisaplays(tLtcReaderSource);
	}

	return true;
}

void SourceSelect::SetRunState(TRunStatus tRunState) {
	m_tRunStatus = tRunState;

	switch (tRunState) {
	case RUN_STATUS_IDLE:
		m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(1u << m_tLtcReaderSource));
		Display::Get()->TextStatus(SourceSelectConst::SOURCE[m_tLtcReaderSource]);
		break;
	case RUN_STATUS_CONTINUE:
		m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(0x0F));
		Display::Get()->TextStatus(">CONTINUE?<");
		break;
	case RUN_STATUS_REBOOT:
		m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(0xF0));
		Display::Get()->TextStatus(">REBOOT?  <");
		break;
	default:
		break;
	}
}

void SourceSelect::HandleActionSelect(void) {
	const uint32_t nMillisNow = Hardware::Get()->Millis();

	if ((nMillisNow - m_nSelectMillis) < 300) {
		return;
	}

	m_nSelectMillis = nMillisNow;

	if (m_tRunStatus == RUN_STATUS_IDLE) {
		SetRunState(RUN_STATUS_CONTINUE);
	} else if (m_tRunStatus == RUN_STATUS_CONTINUE) {
		SetRunState(RUN_STATUS_IDLE);
	} else if (m_tRunStatus == RUN_STATUS_REBOOT) {

		while (SpiFlashStore::Get()->Flash())
			;

		printf("Reboot ...\n");

		m_I2C.WriteRegister(mcp23x17::GPIOB, static_cast<uint8_t>(0xFF));

		Display::Get()->Cls();
		Display::Get()->TextStatus("Reboot ...", Display7SegmentMessage::INFO_REBOOTING);

		Network::Get()->Shutdown();
		Hardware::Get()->Reboot();
	}
}

void SourceSelect::Run(void) {
	if (__builtin_expect(!m_bIsConnected, 0)) {
		return;
	}

	if (__builtin_expect(h3_gpio_lev(gpio::INTA) == LOW, 0)) {

		m_nPortAPrevious = m_nPortA;
		m_nPortA = ~m_I2C.ReadRegister(mcp23x17::GPIOA);

		const uint8_t nButtonsChanged = (m_nPortA ^ m_nPortAPrevious) & m_nPortA;

		if (BUTTON_STATE(button::START)) {
			LtcGenerator::Get()->ActionStart();
		} else if (BUTTON_STATE(button::STOP)) {
			LtcGenerator::Get()->ActionStop();
		} else if (BUTTON_STATE(button::RESUME)) {
			LtcGenerator::Get()->ActionResume();
		} else if (BUTTON_STATE(button::SELECT)) {
			HandleActionSelect();
		} else if (BUTTON_STATE(button::LEFT)) {
			if (m_tRunStatus == RUN_STATUS_REBOOT) {
				SetRunState(RUN_STATUS_CONTINUE);
			}
		} else if (BUTTON_STATE(button::RIGHT)) {
			if (m_tRunStatus == RUN_STATUS_CONTINUE) {
				SetRunState(RUN_STATUS_REBOOT);
			}
		} else {
			if ((m_tRunStatus == RUN_STATUS_CONTINUE) || (m_tRunStatus == RUN_STATUS_REBOOT)) {
				m_tRotaryDirection = m_RotaryEncoder.Process(m_nPortA);
				if (m_tRotaryDirection == RotaryEncoder::CCW) {
					if (m_tRunStatus == RUN_STATUS_REBOOT) {
						SetRunState(RUN_STATUS_CONTINUE);
					}
				} else if (m_tRotaryDirection == RotaryEncoder::CW) {
					if (m_tRunStatus == RUN_STATUS_CONTINUE) {
						SetRunState(RUN_STATUS_REBOOT);
					}
				}
			}
		}

	}
}

