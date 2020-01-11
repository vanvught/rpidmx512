/**
 * @file sourceselect.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "arm/synchronize.h"

#include "sourceselect.h"
#include "sourceselectconst.h"

#include "ltcparams.h"

#include "display.h"
#include "display7segment.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

#include "rotaryencoder.h"

#include "i2c.h"

#include "h3/ltcgenerator.h"

// Interrupt
#include "h3_board.h"
#include "h3_gpio.h"

#include "spiflashstore.h"

#include "debug.h"

#define MCP23017_I2C_ADDRESS	0x20

#define MCP23X17_IOCON			0x0A

#define MCP23X17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPINTENA		0x04	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENA) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23X17_GPPUA			0x0C	///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23X17_INTCAPA		0x10	///< INTERRUPT CAPTURE (INTCAPA) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23X17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

#define MCP23X17_IODIRB			0x01	///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPIOB			0x13	///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

#define BUTTON(x)				((nButtonsChanged >> x) & 0x01)
#define BUTTON_STATE(x)			((nButtonsChanged & (1 << x)) == (1 << x))

#define GPIO_INTA				GPIO_EXT_12 // PA7

static const TDisplay7SegmentMessages s_7Segment[8] = {
		DISPLAY_7SEGMENT_MSG_GENERIC_1, DISPLAY_7SEGMENT_MSG_GENERIC_2,
		DISPLAY_7SEGMENT_MSG_GENERIC_3, DISPLAY_7SEGMENT_MSG_GENERIC_4,
		DISPLAY_7SEGMENT_MSG_GENERIC_5, DISPLAY_7SEGMENT_MSG_GENERIC_6,
		DISPLAY_7SEGMENT_MSG_GENERIC_7, DISPLAY_7SEGMENT_MSG_GENERIC_8 };

enum TButtons {
	BUTTON_SELECT = 2,
	BUTTON_LEFT = 3,
	BUTTON_RIGHT = 4,
	BUTTON_START = 5,
	BUTTON_STOP = 6,
	BUTTON_RESUME = 7
};

SourceSelect::SourceSelect(TLtcReaderSource tLtcReaderSource, struct TLtcDisabledOutputs *ptLtcDisabledOutput):
	m_tLtcReaderSource(tLtcReaderSource),
	m_ptLtcDisabledOutputs(ptLtcDisabledOutput),
	m_bIsConnected(false),
	m_nPortA(0),
	m_nPortAPrevious(0),
	m_nPortB(0),
	m_nMillisPrevious(0),
	m_tRotaryDirection(ROTARY_DIRECTION_NONE),
	m_tRunStatus(RUN_STATUS_IDLE),
	m_nSelectMillis(0)
{

}

SourceSelect::~SourceSelect(void) {

}

void SourceSelect::LedBlink(uint8_t nPortB) {
	const uint32_t nMillisNow = Hardware::Get()->Millis();

	if ((nMillisNow - m_nMillisPrevious) < 500) {
		return;
	}

	m_nMillisPrevious = nMillisNow;

	m_nPortB ^= nPortB;

	i2c_set_address(MCP23017_I2C_ADDRESS);
	i2c_write_reg_uint8(MCP23X17_GPIOB, m_nPortB);
}

void SourceSelect::HandleActionLeft(TLtcReaderSource &tLtcReaderSource) {
	if (tLtcReaderSource == 0) {
		tLtcReaderSource = (TLtcReaderSource) (LTC_READER_SOURCE_UNDEFINED - 1);
	} else {
		tLtcReaderSource = (TLtcReaderSource) (tLtcReaderSource - 1);
	}
}

void SourceSelect::HandleActionRight(TLtcReaderSource &tLtcReaderSource) {
	if (tLtcReaderSource == (TLtcReaderSource) (LTC_READER_SOURCE_UNDEFINED - 1)) {
		tLtcReaderSource = (TLtcReaderSource) 0;
	} else {
		tLtcReaderSource = (TLtcReaderSource) (tLtcReaderSource + 1);
	}
}

void SourceSelect::HandleRotary(uint8_t nInputAB, TLtcReaderSource &tLtcReaderSource) {
	m_tRotaryDirection = m_RotaryEncoder.Process(nInputAB);

	if (m_tRotaryDirection == ROTARY_DIRECTION_CW) {
		HandleActionRight(tLtcReaderSource);
	} else if (m_tRotaryDirection == ROTARY_DIRECTION_CCW) {
		HandleActionLeft(tLtcReaderSource);
	}
}

void SourceSelect::UpdateDisaplays(TLtcReaderSource tLtcReaderSource) {
	Display::Get()->TextStatus(SourceSelectConst::SOURCE[tLtcReaderSource], s_7Segment[tLtcReaderSource]);

	if (!m_ptLtcDisabledOutputs->bMax7219) {
		LtcDisplayMax7219::Get()->WriteChar((uint8_t) tLtcReaderSource);
	}

	if(!m_ptLtcDisabledOutputs->bWS28xx) {
		LtcDisplayWS28xx::Get()->WriteChar((uint8_t) tLtcReaderSource);
	}
}

bool SourceSelect::Check(void) {
	DEBUG_ENTRY

	i2c_set_baudrate(I2C_FULL_SPEED);

	m_bIsConnected = i2c_is_connected(MCP23017_I2C_ADDRESS);

	if (!m_bIsConnected) {
		DEBUG_EXIT
		return false;
	}

	i2c_set_address(MCP23017_I2C_ADDRESS);

	// Rotary and switches
	i2c_write_reg_uint8(MCP23X17_IODIRA, 0xFF); 	// All input
	i2c_write_reg_uint8(MCP23X17_GPPUA, 0xFF);		// Pull-up
	i2c_write_reg_uint8(MCP23X17_GPINTENA, 0xFF);	// Interrupt on Change
	(void) i2c_read_reg_uint8(MCP23X17_INTCAPA);	// Clear interrupts
	// Led's
	i2c_write_reg_uint8(MCP23X17_IODIRB, 0x00); 	// All output
	i2c_write_reg_uint8(MCP23X17_GPIOB, 1 << m_tLtcReaderSource);

	UpdateDisaplays(m_tLtcReaderSource);

	h3_gpio_fsel(GPIO_INTA, GPIO_FSEL_INPUT); // PA7

	uint32_t value = H3_PIO_PORTA->PUL0;
	value &= ~(GPIO_PULL_MASK << 14);
	value |= (GPIO_PULL_UP << 14);
	H3_PIO_PORTA->PUL0 = value;

	DEBUG_EXIT
	return true;
}

bool SourceSelect::Wait(TLtcReaderSource &tLtcReaderSource) {
	LedBlink(1 << tLtcReaderSource);

	if (h3_gpio_lev(GPIO_INTA) == LOW) {

		i2c_set_address(MCP23017_I2C_ADDRESS);
		m_nPortAPrevious = m_nPortA;
		m_nPortA = ~i2c_read_reg_uint8(MCP23X17_GPIOA);

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

		if (BUTTON_STATE(BUTTON_LEFT)) {
			HandleActionLeft(tLtcReaderSource);
		} else if (BUTTON_STATE(BUTTON_RIGHT)) {
			HandleActionRight(tLtcReaderSource);
		} else if (BUTTON_STATE(BUTTON_SELECT)) {
			m_tLtcReaderSource = tLtcReaderSource;

			i2c_write_reg_uint8(MCP23X17_GPIOB, 1 << tLtcReaderSource);
			(void) i2c_read_reg_uint8(MCP23X17_INTCAPA);	// Clear interrupts

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
		i2c_set_address(MCP23017_I2C_ADDRESS);
		i2c_write_reg_uint8(MCP23X17_GPIOB, 1 << m_tLtcReaderSource);
		Display::Get()->TextStatus(SourceSelectConst::SOURCE[m_tLtcReaderSource]);
		break;
	case RUN_STATUS_CONTINUE:
		i2c_set_address(MCP23017_I2C_ADDRESS);
		i2c_write_reg_uint8(MCP23X17_GPIOB, 0x0F);
		Display::Get()->TextStatus(">CONTINUE?<");
		break;
	case RUN_STATUS_REBOOT:
		i2c_set_address(MCP23017_I2C_ADDRESS);
		i2c_write_reg_uint8(MCP23X17_GPIOB, 0xF0);
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

		printf("SoftReset ...\n");

		i2c_set_address(MCP23017_I2C_ADDRESS);
		i2c_write_reg_uint8(MCP23X17_GPIOB, 0xFF);

		Display::Get()->Cls();
		Display::Get()->TextStatus("SoftReset ...", DISPLAY_7SEGMENT_MSG_INFO_REBOOTING);

		Hardware::Get()->SoftReset();
	}
}

void SourceSelect::Run(void) {
	if (__builtin_expect(!m_bIsConnected, 0)) {
		return;
	}

	if (__builtin_expect(h3_gpio_lev(GPIO_INTA) == LOW, 0)) {

		i2c_set_address(MCP23017_I2C_ADDRESS);
		m_nPortAPrevious = m_nPortA;
		m_nPortA = ~i2c_read_reg_uint8(MCP23X17_GPIOA);

		const uint8_t nButtonsChanged = (m_nPortA ^ m_nPortAPrevious) & m_nPortA;

		if (BUTTON_STATE(BUTTON_START)) {
			LtcGenerator::Get()->ActionStart();
		} else if (BUTTON_STATE(BUTTON_STOP)) {
			LtcGenerator::Get()->ActionStop();
		} else if (BUTTON_STATE(BUTTON_RESUME)) {
			LtcGenerator::Get()->ActionResume();
		} else if (BUTTON_STATE(BUTTON_SELECT)) {
			HandleActionSelect();
		} else if (BUTTON_STATE(BUTTON_LEFT)) {
			if (m_tRunStatus == RUN_STATUS_REBOOT) {
				SetRunState(RUN_STATUS_CONTINUE);
			}
		} else if (BUTTON_STATE(BUTTON_RIGHT)) {
			if (m_tRunStatus == RUN_STATUS_CONTINUE) {
				SetRunState(RUN_STATUS_REBOOT);
			}
		} else {
			if ((m_tRunStatus == RUN_STATUS_CONTINUE) || (m_tRunStatus == RUN_STATUS_REBOOT)) {
				m_tRotaryDirection = m_RotaryEncoder.Process(m_nPortA);
				if (m_tRotaryDirection == ROTARY_DIRECTION_CCW) {
					if (m_tRunStatus == RUN_STATUS_REBOOT) {
						SetRunState(RUN_STATUS_CONTINUE);
					}
				} else if (m_tRotaryDirection == ROTARY_DIRECTION_CW) {
					if (m_tRunStatus == RUN_STATUS_CONTINUE) {
						SetRunState(RUN_STATUS_REBOOT);
					}
				}
			}
		}

	}
}

