/**
 * @file artnetnode_internal.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETNODE_INTERNAL_H_
#define ARTNETNODE_INTERNAL_H_

enum TErrorCodes {
	ARTNET_EOK = 0,
	ARTNET_ENET = -1,							///< network error
	ARTNET_EMEM = -2,							///< memory error
	ARTNET_EARG = -3,							///< argument error
	ARTNET_ESTATE = -4,							///< state error
	ARTNET_EACTION = -5							///< invalid action
};

enum TArtNetPortDataCode {
	ARTNET_PORT_DMX = 0x00,		///< Data is DMX-512
	ARTNET_PORT_MIDI = 0x01, 	///< Data is MIDI
	ARTNET_PORT_AVAB = 0x02,	///< Data is Avab
	ARTNET_PORT_CMX = 0x03,		///< Data is Colortran CMX
	ARTNET_PORT_ADB = 0x04,		///< Data is ABD 62.5
	ARTNET_PORT_ARTNET = 0x05	///< Data is ArtNet
};

enum TProgram {
	PROGRAM_NO_CHANGE = 0x7F,					///<
	PROGRAM_DEFAULTS = 0x00,					///<
	PROGRAM_CHANGE_MASK = 0x80					///<
};

/**
 * ArtPollReply packet, Field 12
 */
enum TStatus1 {
	STATUS1_INDICATOR_MASK = (3 << 6),			///< 0b11 bit 7-6, Indicator state.
	STATUS1_INDICATOR_LOCATE_MODE = (1 << 6),	///< 0b01 Indicators in Locate Mode.
	STATUS1_INDICATOR_MUTE_MODE = (2 << 6),		///< 0b10 Indicators in Mute Mode.
	STATUS1_INDICATOR_NORMAL_MODE = (3 << 6),	///< 0b11 Indicators in Normal Mode.
	STATUS1_PAP_MASK = (3 << 4),				///< 0b11 bit 5-4, Port Address Programming Authority
	STATUS1_PAP_UNKNOWN = (0 << 4),				///< 0b00 Port Address Programming Authority unknown.
	STATUS1_PAP_FRONT_PANEL = (1 << 4),			///< 0b01 All Port Address set by front panel controls.
	STATUS1_PAP_NETWORK = (2 << 4),				///< 0b10 All or part of Port Address programmed by network or Web browser.
	STATUS1_PAP_NOTUSED = (3 << 4),				///< 0b11 Not used.
	STATUS1_NORMAL_FIRMWARE_BOOT = (0 << 2),	///< 0 = Normal firmware boot (from flash).
	STATUS1_ROM_BOOT = (1 << 2),				///< 1 = Booted from ROM.
	STATUS1_RDM_CAPABLE = (1 << 1),				///< 1 = Capable of Remote Device Management
	STATUS1_UBEA_PRESENT = (1 << 0)				///< 1 = UBEA present
};

#endif /* ARTNETNODE_INTERNAL_H_ */
