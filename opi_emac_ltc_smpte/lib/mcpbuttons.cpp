/**
 * @file mcpbuttons.cpp
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

#include "mcpbuttons.h"
#include "mcpbuttonsconst.h"

#include "hardware.h"
#include "network.h"

#include "arm/synchronize.h"

#include "ltcdisplayrgb.h"
#include "ltcparams.h"

#include "display.h"
#include "display7segment.h"
#include "ltcdisplaymax7219.h"
#include "rotaryencoder.h"

#include "hal_i2c.h"
#include "mcp23x17.h"

#include "displayedittimecode.h"
#include "displayeditfps.h"
#include "input.h"

#include "h3/ltcgenerator.h"

// Interrupt
#include "board/h3_opi_zero.h"
#include "h3_gpio.h"

#include "spiflashstore.h"
#include "storeltc.h"

#include "debug.h"

static constexpr Display7SegmentMessage s_7Segment[] = {
		Display7SegmentMessage::GENERIC_1, Display7SegmentMessage::GENERIC_2,
		Display7SegmentMessage::GENERIC_3, Display7SegmentMessage::GENERIC_4,
		Display7SegmentMessage::GENERIC_5, Display7SegmentMessage::GENERIC_6,
		Display7SegmentMessage::GENERIC_7, Display7SegmentMessage::GENERIC_8 };

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

using namespace ltc;

McpButtons::McpButtons(source tLtcReaderSource, struct TLtcDisabledOutputs *ptLtcDisabledOutput, bool bUseAltFunction, int32_t nSkipSeconds):
	m_I2C(mcp23017::I2C_ADDRESS),
	m_tLtcReaderSource(tLtcReaderSource),
	m_ptLtcDisabledOutputs(ptLtcDisabledOutput),
	m_bUseAltFunction(bUseAltFunction),
	m_nSkipSeconds(nSkipSeconds)

{
	Ltc::InitTimeCode(m_aTimeCode);
}

uint32_t McpButtons::LedBlink(uint8_t nPortB) {
	const auto nMillisNow = Hardware::Get()->Millis();

	if (__builtin_expect(((nMillisNow - m_nMillisPrevious) < 500), 1)) {
		return m_nLedTicker;
	}

	m_nMillisPrevious = nMillisNow;
	m_nPortB ^= nPortB;
	m_I2C.WriteRegister(mcp23x17::reg::GPIOB, m_nPortB);

	return ++m_nLedTicker;
}

void McpButtons::HandleActionLeft(ltc::source &tLtcReaderSource) {
	if (m_State == SOURCE_SELECT) {
		if (tLtcReaderSource == 0) {
			tLtcReaderSource = static_cast<ltc::source>((ltc::source::UNDEFINED - 1));
		} else {
			tLtcReaderSource = static_cast<ltc::source>((tLtcReaderSource - 1));
		}
		UpdateDisplays(tLtcReaderSource);
		return;
	}

	m_nKey = INPUT_KEY_LEFT;
}

void McpButtons::HandleActionRight(ltc::source &tLtcReaderSource) {
	if (m_State == SOURCE_SELECT) {
		if (tLtcReaderSource == static_cast<ltc::source>((ltc::source::UNDEFINED - 1))) {
			tLtcReaderSource = static_cast<ltc::source>(0);
		} else {
			tLtcReaderSource = static_cast<ltc::source>((tLtcReaderSource + 1));
		}
		UpdateDisplays(tLtcReaderSource);
		return;
	}

	m_nKey = INPUT_KEY_RIGHT;
}

void McpButtons::HandleActionSelect(const ltc::source& tLtcReaderSource) {
	if (m_tLtcReaderSource != tLtcReaderSource) {
		m_tLtcReaderSource = tLtcReaderSource;
		StoreLtc::Get()->SaveSource(m_tLtcReaderSource);
	}

	m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(1u << tLtcReaderSource));
	m_I2C.ReadRegister(mcp23x17::reg::INTCAPA);	// Clear interrupts

	Display::Get()->SetCursor(display::cursor::OFF);
	Display::Get()->SetCursorPos(0, 0);
	Display::Get()->ClearLine(1);
	Display::Get()->ClearLine(2);
}

void McpButtons::HandleRotary(uint8_t nInputAB, ltc::source &tLtcReaderSource) {
	m_tRotaryDirection = m_RotaryEncoder.Process(nInputAB);

	if (m_State == SOURCE_SELECT) {
		if (m_tRotaryDirection == RotaryEncoder::CW) {
			HandleActionRight(tLtcReaderSource);
		} else if (m_tRotaryDirection == RotaryEncoder::CCW) {
			HandleActionLeft(tLtcReaderSource);
		}
		return;
	}

	if (m_tRotaryDirection == RotaryEncoder::CW) {
		m_nKey = INPUT_KEY_DOWN;
	} else if (m_tRotaryDirection == RotaryEncoder::CCW) {
		m_nKey = INPUT_KEY_UP;
	}
}

void McpButtons::UpdateDisplays(const ltc::source& tLtcReaderSource) {
	Display::Get()->TextStatus(McpButtonsConst::SOURCE[tLtcReaderSource], s_7Segment[tLtcReaderSource]);

	if (!m_ptLtcDisabledOutputs->bMax7219) {
		DEBUG_PUTS("");
		LtcDisplayMax7219::Get()->WriteChar(tLtcReaderSource);
	}

	if (!m_ptLtcDisabledOutputs->bWS28xx) {
		DEBUG_PUTS("");
		LtcDisplayRgb::Get()->WriteChar(tLtcReaderSource);
	}
}

bool McpButtons::Check() {
	DEBUG_ENTRY

	m_bIsConnected = m_I2C.IsConnected();

	if (!m_bIsConnected) {
		DEBUG_EXIT
		return false;
	}

	// Rotary and buttons
	m_I2C.WriteRegister(mcp23x17::reg::IODIRA, static_cast<uint8_t>(0xFF)); 	// All input
	m_I2C.WriteRegister(mcp23x17::reg::GPPUA, static_cast<uint8_t>(0xFF));		// Pull-up
	m_I2C.WriteRegister(mcp23x17::reg::IPOLA, static_cast<uint8_t>(0xFF));		// Invert read
	m_I2C.WriteRegister(mcp23x17::reg::INTCONA, static_cast<uint8_t>(0x00));
	m_I2C.WriteRegister(mcp23x17::reg::GPINTENA, static_cast<uint8_t>(0xFF));	// Interrupt on Change
	m_I2C.ReadRegister(mcp23x17::reg::INTCAPA);									// Clear interrupts
	// Led's
	m_I2C.WriteRegister(mcp23x17::reg::IODIRB, static_cast<uint8_t>(0x00)); 	// All output
	m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(1u << m_tLtcReaderSource));

	UpdateDisplays(m_tLtcReaderSource);

	h3_gpio_fsel(gpio::INTA, GPIO_FSEL_INPUT); // PA7
	h3_gpio_pud(gpio::INTA, GPIO_PULL_UP);

	DEBUG_EXIT
	return true;
}

bool McpButtons::Wait(ltc::source &tLtcReaderSource, TLtcTimeCode& StartTimeCode, TLtcTimeCode& StopTimeCode) {
	if (__builtin_expect((LedBlink(1 << tLtcReaderSource) >= m_nLedTickerMax), 0)) {
		return false;
	}
//	LedBlink(1 << tLtcReaderSource);

	if (__builtin_expect(h3_gpio_lev(gpio::INTA) == LOW, 0)) {
		m_nLedTickerMax = UINT32_MAX;

		const auto nPortA = m_I2C.ReadRegister(mcp23x17::reg::GPIOA);
		const uint8_t nButtonsChanged = (nPortA ^ m_nPortAPrevious) & nPortA;

		m_nPortAPrevious = nPortA;

		DEBUG_PRINTF("%.2x %.2x", nPortA, nButtonsChanged);

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

		m_nKey = INPUT_KEY_NOT_DEFINED;

		if (BUTTON_STATE(button::LEFT)) {
			HandleActionLeft(tLtcReaderSource);
		} else if (BUTTON_STATE(button::RIGHT)) {
			HandleActionRight(tLtcReaderSource);
		} else if (BUTTON_STATE(button::SELECT)) {
			HandleActionSelect(tLtcReaderSource);
			return false;
		} else if (BUTTON_STATE(button::START)) {
			if (tLtcReaderSource == ltc::source::INTERNAL) {
				if (m_State != EDIT_TIMECODE_START) {
					Display::Get()->SetCursorPos(14,0);
					Display::Get()->PutString("[Start]");
				}

				m_nKey = INPUT_KEY_ENTER;

				switch (m_State) {
					case SOURCE_SELECT:
						m_State = EDIT_TIMECODE_START;
						break;
					case EDIT_TIMECODE_START:
						m_State = EDIT_FPS;
						break;
					case EDIT_TIMECODE_STOP:
					case EDIT_FPS:
						m_State = EDIT_TIMECODE_START;
						break;
					default:
						break;
				}
			}
		} else if (BUTTON_STATE(button::STOP)) {
			if (tLtcReaderSource == ltc::source::INTERNAL) {
				if (m_State != EDIT_TIMECODE_STOP) {
					Display::Get()->SetCursorPos(14,0);
					Display::Get()->PutString("[Stop] ");
				}
				m_nKey = INPUT_KEY_ENTER;
				m_State = EDIT_TIMECODE_STOP;

			}
		} else if (BUTTON_STATE(button::RESUME)) {
			if (tLtcReaderSource == ltc::source::INTERNAL) {
				m_nKey = INPUT_KEY_ESC;
			}
		} else {
			HandleRotary(nPortA, tLtcReaderSource);
		}

		switch (m_State) {
			case EDIT_TIMECODE_START:
				HandleInternalTimeCodeStart(StartTimeCode);
				break;
			case EDIT_TIMECODE_STOP:
				HandleInternalTimeCodeStop(StopTimeCode);
				break;
			case EDIT_FPS:
				HandleInternalTimeCodeFps(StartTimeCode);
				break;
			default:
				break;
		}

		m_nPortB = 0;
	}

	return true;
}

/*
 * Run state
 */

void McpButtons::SetRunState(RunStatus tRunState) {
	m_tRunStatus = tRunState;

	switch (tRunState) {
	case RunStatus::IDLE:
		m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(1u << m_tLtcReaderSource));
		Display::Get()->TextStatus(McpButtonsConst::SOURCE[m_tLtcReaderSource]);
		break;
	case RunStatus::CONTINUE:
		m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(0x0F));
		Display::Get()->TextStatus(">CONTINUE?<");
		break;
	case RunStatus::REBOOT:
		m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(0xF0));
		Display::Get()->TextStatus(">REBOOT?  <");
		break;
	case RunStatus::TC_RESET:
		m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(0xAA));
		Display::Get()->TextStatus(">RESET TC?<");
		break;
	default:
		break;
	}
}

void McpButtons::HandleRunActionSelect() {
	const uint32_t nMillisNow = Hardware::Get()->Millis();

	if ((nMillisNow - m_nSelectMillis) < 300) {
		return;
	}

	m_nSelectMillis = nMillisNow;

	if (m_tRunStatus == RunStatus::IDLE) {
		SetRunState(RunStatus::CONTINUE);
		return;
	}

	if (m_tRunStatus == RunStatus::CONTINUE) {
		SetRunState(RunStatus::IDLE);
		return;
	}

	if (m_tRunStatus == RunStatus::REBOOT) {
		while (SpiFlashStore::Get()->Flash())
			;

		printf("Reboot ...\n");

		m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(0xFF));

		Display::Get()->Cls();
		Display::Get()->TextStatus("Reboot ...", Display7SegmentMessage::INFO_REBOOTING);

		Network::Get()->Shutdown();
		Hardware::Get()->Reboot();

		return;
	}

	if (m_tRunStatus == RunStatus::TC_RESET) {
		SetRunState(RunStatus::IDLE);
		LtcGenerator::Get()->ActionReset();

		return;
	}
}

void McpButtons::Run() {
	if (__builtin_expect(!m_bIsConnected, 0)) {
		return;
	}

	if (__builtin_expect(h3_gpio_lev(gpio::INTA) == LOW, 0)) {

		const auto nPortA = m_I2C.ReadRegister(mcp23x17::reg::GPIOA);
		const uint8_t nButtonsChanged = (nPortA ^ m_nPortAPrevious) & nPortA;

		m_nPortAPrevious = nPortA;

		if (m_tLtcReaderSource == ltc::source::INTERNAL) {
			if (BUTTON_STATE(button::START)) {
				LtcGenerator::Get()->ActionStart(!m_bUseAltFunction);
				return;
			}

			if (BUTTON_STATE(button::STOP)) {
				LtcGenerator::Get()->ActionStop();
				return;
			}

			if (BUTTON_STATE(button::RESUME)) {
				if (!m_bUseAltFunction) {
					LtcGenerator::Get()->ActionResume();
					return;
				}

				if (m_tRunStatus == RunStatus::IDLE) {
					SetRunState(RunStatus::TC_RESET);
					return;
				}

				if (m_tRunStatus == RunStatus::TC_RESET) {
					SetRunState(RunStatus::IDLE);
					return;
				}

				return;
			}
		}

		if (BUTTON_STATE(button::SELECT)) {
			HandleRunActionSelect();
			return;
		}

		if (BUTTON_STATE(button::LEFT)) {
			if (m_tRunStatus == RunStatus::REBOOT) {
				SetRunState(RunStatus::CONTINUE);
				return;
			}

			if (m_tRunStatus == RunStatus::IDLE) {
				LtcGenerator::Get()->ActionBackward(m_nSkipSeconds);
				return;
			}

			return;
		}

		if (BUTTON_STATE(button::RIGHT)) {
			if (m_tRunStatus == RunStatus::CONTINUE) {
				SetRunState(RunStatus::REBOOT);
				return;
			}

			if (m_tRunStatus == RunStatus::IDLE) {
				LtcGenerator::Get()->ActionForward(m_nSkipSeconds);
				return;
			}

			return;
		}

		m_tRotaryDirection = m_RotaryEncoder.Process(nPortA);

		if (m_tRotaryDirection == RotaryEncoder::CCW) {
			if (m_tRunStatus == RunStatus::REBOOT) {
				SetRunState(RunStatus::CONTINUE);
				return;
			}

			if (m_tRunStatus == RunStatus::IDLE) {
				LtcGenerator::Get()->ActionBackward(m_nSkipSeconds);
				return;
			}

			return;
		}

		if (m_tRotaryDirection == RotaryEncoder::CW) {
			if (m_tRunStatus == RunStatus::CONTINUE) {
				SetRunState(RunStatus::REBOOT);
				return;
			}

			if (m_tRunStatus == RunStatus::IDLE) {
				LtcGenerator::Get()->ActionForward(m_nSkipSeconds);
				return;
			}

			return;
		}
	}
}
