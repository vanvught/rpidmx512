/**
 * @file display7segment.h
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

#ifndef DISPLAY7SEGMENT_H_
#define DISPLAY7SEGMENT_H_

#include <stdint.h>

#include "hal_i2c.h"

namespace display7segment {
static constexpr uint8_t CH_0 = 0x3F;		// 0b00111111
static constexpr uint8_t CH_1 = 0x06;		// 0b00000110
static constexpr uint8_t CH_2 = 0x5B;		// 0b01011011
static constexpr uint8_t CH_3 = 0x4F;		// 0b01001111
static constexpr uint8_t CH_4 = 0x66;		// 0b01100110
static constexpr uint8_t CH_5 = 0x6D;		// 0b01101101
static constexpr uint8_t CH_6 = 0x7D;		// 0b01111101
static constexpr uint8_t CH_7 = 0x07;		// 0b00000111
static constexpr uint8_t CH_8 = 0x7F;		// 0b01111111
static constexpr uint8_t CH_9 = 0x6F;		// 0b01101111
static constexpr uint8_t CH_A = 0x77;		// 0b01110111
static constexpr uint8_t CH_B = 0x7C;		// 0b01111100
static constexpr uint8_t CH_C = 0x39;		// 0b00111001
static constexpr uint8_t CH_D = 0x5E;		// 0b01011110
static constexpr uint8_t CH_E = 0x79;		// 0b01111001
static constexpr uint8_t CH_F = 0x71;		// 0b01110001
static constexpr uint8_t CH_P = 0x73;		// 0b01110011
static constexpr uint8_t CH_MIN = 0x40;		// 0b01000000
static constexpr uint8_t CH_DP = 0x80;		// 0b10000000
static constexpr uint8_t CH_BLANK = 0x00;	// 0b00000000

static constexpr uint16_t Msg(uint8_t nDigitRight, uint8_t nDigitLeft) {
	return (nDigitLeft << 8) | nDigitRight;
}
}  // namespace display7segment

enum class Display7SegmentMessage {
	// Generic Digits
	GENERIC_0 = display7segment::Msg(display7segment::CH_0, display7segment::CH_BLANK),
	GENERIC_1 = display7segment::Msg(display7segment::CH_1, display7segment::CH_BLANK),
	GENERIC_2 = display7segment::Msg(display7segment::CH_2, display7segment::CH_BLANK),
	GENERIC_3 = display7segment::Msg(display7segment::CH_3, display7segment::CH_BLANK),
	GENERIC_4 = display7segment::Msg(display7segment::CH_4, display7segment::CH_BLANK),
	GENERIC_5 = display7segment::Msg(display7segment::CH_5, display7segment::CH_BLANK),
	GENERIC_6 = display7segment::Msg(display7segment::CH_6, display7segment::CH_BLANK),
	GENERIC_7 = display7segment::Msg(display7segment::CH_7, display7segment::CH_BLANK),
	GENERIC_8 = display7segment::Msg(display7segment::CH_8, display7segment::CH_BLANK),
	GENERIC_9 = display7segment::Msg(display7segment::CH_9, display7segment::CH_BLANK),
	// Startup messages
	INFO_STARTUP = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_0),
	INFO_NETWORK_INIT = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_1),
	INFO_DHCP = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_2),
	INFO_IP = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_3),
	INFO_NTP = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_4),
	INFO_SPARKFUN = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_5),
	//
	INFO_NETWORK_SHUTDOWN = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_9),
	//
	INFO_NODE_PARMAMS = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_4),
	INFO_BRIDGE_PARMAMS = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_4),
	INFO_OSCCLIENT_PARMAMS = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_4),
	//
	INFO_RDM_RUN = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_5),
	INFO_NODE_START = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_6),
	INFO_BRIDGE_START = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_6),
	INFO_OSCCLIENT_START = display7segment::Msg(display7segment::CH_BLANK, display7segment::CH_6),
	//
	INFO_NONE = display7segment::Msg(display7segment::CH_DP, display7segment::CH_DP),
	INFO_NODE_STARTED = display7segment::Msg(display7segment::CH_DP, display7segment::CH_DP),
	INFO_BRIDGE_STARTED = display7segment::Msg(display7segment::CH_DP, display7segment::CH_DP),
	INFO_OSCCLIENT_STARTED = display7segment::Msg(display7segment::CH_DP, display7segment::CH_DP),
	// SPI Flash messages
	INFO_SPI_NONE = display7segment::Msg(display7segment::CH_C, display7segment::CH_MIN),
	INFO_SPI_CHECK = display7segment::Msg(display7segment::CH_C, display7segment::CH_0),
	INFO_SPI_ERASE = display7segment::Msg(display7segment::CH_C, display7segment::CH_1),
	INFO_SPI_WRITING = display7segment::Msg(display7segment::CH_C, display7segment::CH_2),
	INFO_SPI_NODIFF = display7segment::Msg(display7segment::CH_C, display7segment::CH_3),
	INFO_SPI_DONE = display7segment::Msg(display7segment::CH_C, display7segment::CH_C),
	INFO_SPI_UPDATE = display7segment::Msg(display7segment::CH_C, display7segment::CH_F),
	// Firmware TFTP messages
	INFO_TFTP_ON = display7segment::Msg(display7segment::CH_F, display7segment::CH_MIN),
	INFO_TFTP_STARTED = display7segment::Msg(display7segment::CH_F, display7segment::CH_1),
	INFO_TFTP_ENDED = display7segment::Msg(display7segment::CH_F, display7segment::CH_2),
	INFO_TFTP_OFF = display7segment::Msg(display7segment::CH_F, display7segment::CH_DP),
	// Informational / Warning messages
	INFO_REBOOTING = display7segment::Msg(display7segment::CH_MIN, display7segment::CH_MIN),
	INFO_DATALOSS =  display7segment::Msg(display7segment::CH_D, display7segment::CH_MIN),
	// Error messages
	ERROR_DHCP = display7segment::Msg(display7segment::CH_E, display7segment::CH_2),
	ERROR_NTP = display7segment::Msg(display7segment::CH_E, display7segment::CH_4),
	ERROR_SPARKFUN = display7segment::Msg(display7segment::CH_E, display7segment::CH_5),
	ERROR_MCP23S017 = display7segment::Msg(display7segment::CH_E, display7segment::CH_8),
	ERROR_SI5351A = display7segment::Msg(display7segment::CH_E, display7segment::CH_9),
	ERROR_NEXTION = display7segment::Msg(display7segment::CH_E, display7segment::CH_A),
	ERROR_SPI = display7segment::Msg(display7segment::CH_E, display7segment::CH_C),
	ERROR_FATAL = display7segment::Msg(display7segment::CH_E, display7segment::CH_E),
	ERROR_TFTP = display7segment::Msg(display7segment::CH_E, display7segment::CH_F),
	// LTC messages
	LTC_WAITING = display7segment::Msg(display7segment::CH_DP, display7segment::CH_DP),
	LTC_FILM = display7segment::Msg(display7segment::CH_2, display7segment::CH_4),
	LTC_EBU = display7segment::Msg(display7segment::CH_2, display7segment::CH_5),
	LTC_DF = display7segment::Msg(display7segment::CH_2, display7segment::CH_9),
	LTC_SMPTE = display7segment::Msg(display7segment::CH_3, display7segment::CH_0),
	// OSC Client messages
	INFO_OSCCLIENT_PING_PONG = display7segment::Msg(display7segment::CH_P, display7segment::CH_P),
	ERROR_OSCCLIENT_PING_PONG = display7segment::Msg(display7segment::CH_P, display7segment::CH_E),
	// Apple MIDI - rtpMIDI messages
	// TODO Apple MIDI - rtpMIDI messages
	// Show File player
	INFO_PLAYER_IDLE = display7segment::Msg(display7segment::CH_P, display7segment::CH_0),
	INFO_PLAYER_RUNNING = display7segment::Msg(display7segment::CH_P, display7segment::CH_1),
	INFO_PLAYER_RUNNING_LOOP = display7segment::Msg(display7segment::CH_P, display7segment::CH_2),
	INFO_PLAYER_STOPPED = display7segment::Msg(display7segment::CH_P, display7segment::CH_3),
	INFO_PLAYER_STOPPED_LOOP = display7segment::Msg(display7segment::CH_P, display7segment::CH_4),
	INFO_PLAYER_ENDED = display7segment::Msg(display7segment::CH_P, display7segment::CH_9),
	ERROR_PLAYER = display7segment::Msg(display7segment::CH_P, display7segment::CH_E)
};

class Display7Segment {
public:
	Display7Segment();

	bool Have7Segment() {
		return m_bHave7Segment;
	}

	void Status(Display7SegmentMessage msg);
	void Status(uint8_t nValue, bool bHex);

	uint16_t GetData(uint8_t nHexValue) {

		switch (nHexValue) {
		case 0:
			return display7segment::CH_0;
			break;
		case 1:
			return display7segment::CH_1;
			break;
		case 2:
			return display7segment::CH_2;
			break;
		case 3:
			return display7segment::CH_3;
			break;
		case 4:
			return display7segment::CH_4;
			break;
		case 5:
			return display7segment::CH_5;
			break;
		case 6:
			return display7segment::CH_6;
			break;
		case 7:
			return display7segment::CH_7;
			break;
		case 8:
			return display7segment::CH_8;
			break;
		case 9:
			return display7segment::CH_9;
			break;
		case 0xa:
			return display7segment::CH_A;
			break;
		case 0xb:
			return display7segment::CH_B;
			break;
		case 0xc:
			return display7segment::CH_C;
			break;
		case 0xd:
			return display7segment::CH_D;
			break;
		case 0xe:
			return display7segment::CH_E;
			break;
		case 0xf:
			return display7segment::CH_F;
			break;
		default:
			break;
		}

		return display7segment::CH_BLANK;
	}

	static Display7Segment* Get() {
		return s_pThis;
	}

private:
	HAL_I2C m_I2C;
	bool m_bHave7Segment{false};

	static Display7Segment *s_pThis;
};

#endif /* DISPLAY7SEGMENT_H_ */
