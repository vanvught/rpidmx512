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

struct Display7Segment {
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
};

enum TDisplay7SegmentMessages {
	// Generic Digits
	DISPLAY_7SEGMENT_MSG_GENERIC_0 = Display7Segment::Msg(Display7Segment::CH_0, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_1 = Display7Segment::Msg(Display7Segment::CH_1, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_2 = Display7Segment::Msg(Display7Segment::CH_2, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_3 = Display7Segment::Msg(Display7Segment::CH_3, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_4 = Display7Segment::Msg(Display7Segment::CH_4, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_5 = Display7Segment::Msg(Display7Segment::CH_5, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_6 = Display7Segment::Msg(Display7Segment::CH_6, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_7 = Display7Segment::Msg(Display7Segment::CH_7, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_8 = Display7Segment::Msg(Display7Segment::CH_8, Display7Segment::CH_BLANK),
	DISPLAY_7SEGMENT_MSG_GENERIC_9 = Display7Segment::Msg(Display7Segment::CH_9, Display7Segment::CH_BLANK),
	// Startup messages
	DISPLAY_7SEGMENT_MSG_INFO_STARTUP = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_0),
	DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_1),
	DISPLAY_7SEGMENT_MSG_INFO_DHCP = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_2),
	DISPLAY_7SEGMENT_MSG_INFO_IP = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_3),
	DISPLAY_7SEGMENT_MSG_INFO_NTP = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_INFO_SPARKFUN = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_5),
	//
	DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_PARMAMS = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_4),
	//
	DISPLAY_7SEGMENT_MSG_INFO_RDM_RUN = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_5),
	DISPLAY_7SEGMENT_MSG_INFO_NODE_START = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_6),
	DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_START = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_6),
	DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_START = Display7Segment::Msg(Display7Segment::CH_BLANK, Display7Segment::CH_6),
	//
	DISPLAY_7SEGMENT_MSG_INFO_NONE = Display7Segment::Msg(Display7Segment::CH_DP, Display7Segment::CH_DP),
	DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED = Display7Segment::Msg(Display7Segment::CH_DP, Display7Segment::CH_DP),
	DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_STARTED = Display7Segment::Msg(Display7Segment::CH_DP, Display7Segment::CH_DP),
	DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_STARTED = Display7Segment::Msg(Display7Segment::CH_DP, Display7Segment::CH_DP),
	// SPI Flash messages
	DISPLAY_7SEGMENT_MSG_INFO_SPI_NONE = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_MIN),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_CHECK = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_0),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_ERASE = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_1),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_WRITING = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_2),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_NODIFF = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_3),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_DONE = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_C),
	DISPLAY_7SEGMENT_MSG_INFO_SPI_UPDATE = Display7Segment::Msg(Display7Segment::CH_C, Display7Segment::CH_F),
	// Firmware TFTP messages
	DISPLAY_7SEGMENT_MSG_INFO_TFTP_ON = Display7Segment::Msg(Display7Segment::CH_F, Display7Segment::CH_MIN),
	DISPLAY_7SEGMENT_MSG_INFO_TFTP_STARTED = Display7Segment::Msg(Display7Segment::CH_F, Display7Segment::CH_1),
	DISPLAY_7SEGMENT_MSG_INFO_TFTP_ENDED = Display7Segment::Msg(Display7Segment::CH_F, Display7Segment::CH_2),
	DISPLAY_7SEGMENT_MSG_INFO_TFTP_OFF = Display7Segment::Msg(Display7Segment::CH_F, Display7Segment::CH_DP),
	// Informational / Warning messages
	DISPLAY_7SEGMENT_MSG_INFO_REBOOTING = Display7Segment::Msg(Display7Segment::CH_MIN, Display7Segment::CH_MIN),
	DISPLAY_7SEGMENT_MSG_INFO_DATALOSS =  Display7Segment::Msg(Display7Segment::CH_D, Display7Segment::CH_MIN),
	// Error messages
	DISPLAY_7SEGMENT_MSG_ERROR_DHCP = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_2),
	DISPLAY_7SEGMENT_MSG_ERROR_NTP = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_ERROR_SPARKFUN = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_5),
	DISPLAY_7SEGMENT_MSG_ERROR_MCP23S017 = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_8),
	DISPLAY_7SEGMENT_MSG_ERROR_SI5351A = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_9),
	DISPLAY_7SEGMENT_MSG_ERROR_NEXTION = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_A),
	DISPLAY_7SEGMENT_MSG_ERROR_SPI = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_C),
	DISPLAY_7SEGMENT_MSG_ERROR_FATAL = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_E),
	DISPLAY_7SEGMENT_MSG_ERROR_TFTP = Display7Segment::Msg(Display7Segment::CH_E, Display7Segment::CH_F),
	// LTC messages
	DISPLAY_7SEGMENT_MSG_LTC_WAITING = Display7Segment::Msg(Display7Segment::CH_DP, Display7Segment::CH_DP),
	DISPLAY_7SEGMENT_MSG_LTC_FILM = Display7Segment::Msg(Display7Segment::CH_2, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_LTC_EBU = Display7Segment::Msg(Display7Segment::CH_2, Display7Segment::CH_5),
	DISPLAY_7SEGMENT_MSG_LTC_DF = Display7Segment::Msg(Display7Segment::CH_2, Display7Segment::CH_9),
	DISPLAY_7SEGMENT_MSG_LTC_SMPTE = Display7Segment::Msg(Display7Segment::CH_3, Display7Segment::CH_0),
	// OSC Client messages
	DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_PING_PONG = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_P),
	DISPLAY_7SEGMENT_MSG_ERROR_OSCCLIENT_PING_PONG = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_E),
	// Apple MIDI - rtpMIDI messages
	// TODO Apple MIDI - rtpMIDI messages
	// Show File player
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_IDLE = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_0),
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_RUNNING = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_1),
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_RUNNING_LOOP = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_2),
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_STOPPED = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_3),
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_STOPPED_LOOP = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_4),
	DISPLAY_7SEGMENT_MSG_INFO_PLAYER_ENDED = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_9),
	DISPLAY_7SEGMENT_MSG_ERROR_PLAYER = Display7Segment::Msg(Display7Segment::CH_P, Display7Segment::CH_E)
};

#endif /* DISPLAY7SEGMENT_H_ */
