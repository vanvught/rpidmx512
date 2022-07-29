/**
 * @file artnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNET_H_
#define ARTNET_H_

#include <cstdint>

namespace artnet {
static constexpr char NODE_ID[] = "Art-Net";			///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
static constexpr auto MERGE_TIMEOUT_SECONDS = 10;
static constexpr auto NETWORK_DATA_LOSS_TIMEOUT = 10;	///< Seconds

enum class PortProtocol {
	ARTNET,	///< Output both DMX512 and RDM packets from the Art-Net protocol (default).
	SACN	///< Output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
};

/**
 * Table 4 – Style Codes
 * The Style code defines the general functionality of a Controller.
 * The Style code is returned in ArtPollReply.
 */
struct StyleCode {
	static constexpr uint8_t ST_NODE = 0x00;	///< A DMX to / from Art-Net device
	static constexpr uint8_t ST_SERVER = 0x01;	///< A lighting console.
	static constexpr uint8_t ST_MEDIA = 0x02;	///< A Media Server.
	static constexpr uint8_t ST_ROUTE = 0x03;	///< A network routing device.
	static constexpr uint8_t ST_BACKUP = 0x04;	///< A backup device.
	static constexpr uint8_t ST_CONFIG = 0x05;	///< A configuration or diagnostic tool.
	static constexpr uint8_t ST_VISUAL = 0x06;	///< A visualiser.
};

/**
 * Table 5 – Priority Codes
 * Diagnostics Priority codes.
 * These are used in ArtPoll and ArtDiagData.
 */
struct PriorityCodes {
	static constexpr uint8_t DP_LOW = 0x10;		///< Low priority message.
	static constexpr uint8_t DP_MED = 0x40;		///< Medium priority message.
	static constexpr uint8_t DP_HIGH = 0x80;	///< High priority message.
	static constexpr uint8_t DP_CRITICAL = 0xE0;///< Critical priority message.
	static constexpr uint8_t DP_VOLATILE = 0xF0;///< Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types are displayed in a list box.
};

/**
 * An enum for setting the behavior of a port.
 * Ports can either input data (DMX -> ArtNet) or
 * output (ArtNet -> DMX) data.
 */
struct PortSettings {
	static constexpr uint8_t ENABLE_INPUT = 0x40;
	static constexpr uint8_t ENABLE_OUTPUT = 0x80;
};

struct PortDataCode {
	static constexpr uint8_t PORT_DMX = 0x00;	///< Data is DMX-512
	static constexpr uint8_t PORT_MIDI = 0x01; 	///< Data is MIDI
	static constexpr uint8_t PORT_AVAB = 0x02;	///< Data is Avab
	static constexpr uint8_t PORT_CMX = 0x03;	///< Data is Colortran CMX
	static constexpr uint8_t PORT_ADB = 0x04;	///< Data is ABD 62.5
	static constexpr uint8_t PORT_ARTNET = 0x05;///< Data is ArtNet
};

/**
 * Node configuration commands
 */
struct PortCommand {
	static constexpr uint8_t PC_NONE = 0x00;		///< No action
	static constexpr uint8_t PC_CANCEL = 0x01;		///< If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
	static constexpr uint8_t PC_LED_NORMAL = 0x02;	///< The front panel indicators of the Node operate normally.
	static constexpr uint8_t PC_LED_MUTE = 0x03;	///< The front panel indicators of the Node are disabled and switched off.
	static constexpr uint8_t PC_LED_LOCATE = 0x04;	///< Rapid flashing of the Node’s front panel indicators. It is intended as an outlet locator for large installations.
	static constexpr uint8_t PC_RESET = 0x05;		///< Resets the Node’s Sip, Text, Test and data error flags.
	static constexpr uint8_t PC_MERGE_LTP_O = 0x10;	///< Set DMX Port 0 to Merge in LTP mode.
	static constexpr uint8_t PC_MERGE_LTP_1 = 0x11;	///< Set DMX Port 1 to Merge in LTP mode.
	static constexpr uint8_t PC_MERGE_LTP_2 = 0x12;	///< Set DMX Port 2 to Merge in LTP mode.
	static constexpr uint8_t PC_MERGE_LTP_3 = 0x13;	///< Set DMX Port 3 to Merge in LTP mode.
	static constexpr uint8_t PC_MERGE_HTP_0 = 0x50;	///< Set DMX Port 0 to Merge in HTP (default) mode.
	static constexpr uint8_t PC_MERGE_HTP_1 = 0x51;	///< Set DMX Port 1 to Merge in HTP (default) mode.
	static constexpr uint8_t PC_MERGE_HTP_2 = 0x52;	///< Set DMX Port 2 to Merge in HTP (default) mode.
	static constexpr uint8_t PC_MERGE_HTP_3 = 0x53;	///< Set DMX Port 3 to Merge in HTP (default) mode.
	static constexpr uint8_t PC_ARTNET_SEL0 = 0x60;	///< Set DMX Port 0 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t PC_ARTNET_SEL1 = 0x61;	///< Set DMX Port 1 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t PC_ARTNET_SEL2 = 0x62;	///< Set DMX Port 2 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t PC_ARTNET_SEL3 = 0x63;	///< Set DMX Port 3 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t PC_ACN_SEL0 = 0x70;	///< Set DMX Port 0 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t PC_ACN_SEL1 = 0x71;	///< Set DMX Port 1 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t PC_ACN_SEL2 = 0x72;	///< Set DMX Port 2 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t PC_ACN_SEL3 = 0x73;	///< Set DMX Port 3 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t PC_CLR_0 = 0x90;		///< Clear DMX Output buffer for Port 0
	static constexpr uint8_t PC_CLR_1 = 0x91;		///< Clear DMX Output buffer for Port 1
	static constexpr uint8_t PC_CLR_2 = 0x92;		///< Clear DMX Output buffer for Port 2
	static constexpr uint8_t PC_CLR_3 = 0x93;		///< Clear DMX Output buffer for Port 3
};

struct Program {
	static constexpr uint8_t NO_CHANGE = 0x7F;
	static constexpr uint8_t DEFAULTS = 0x00;
	static constexpr uint8_t CHANGE_MASK = 0x80;
};

struct Status1 {
	static constexpr uint8_t INDICATOR_MASK = (3 << 6);			///< 0b11 bit 7-6, Indicator state.
	static constexpr uint8_t INDICATOR_LOCATE_MODE = (1 << 6);	///< 0b01 Indicators in Locate Mode.
	static constexpr uint8_t INDICATOR_MUTE_MODE = (2 << 6);	///< 0b10 Indicators in Mute Mode.
	static constexpr uint8_t INDICATOR_NORMAL_MODE = (3 << 6);	///< 0b11 Indicators in Normal Mode.
	static constexpr uint8_t PAP_MASK = (3 << 4);				///< 0b11 bit 5-4, Port Address Programming Authority
	static constexpr uint8_t PAP_UNKNOWN = (0 << 4);			///< 0b00 Port Address Programming Authority unknown.
	static constexpr uint8_t PAP_FRONT_PANEL = (1 << 4);		///< 0b01 All Port Address set by front panel controls.
	static constexpr uint8_t PAP_NETWORK = (2 << 4);			///< 0b10 All or part of Port Address programmed by network or Web browser.
	static constexpr uint8_t PAP_NOTUSED = (3 << 4);			///< 0b11 Not used.
	static constexpr uint8_t NORMAL_FIRMWARE_BOOT = (0 << 2);	///< 0 = Normal firmware boot (from flash).
	static constexpr uint8_t ROM_BOOT = (1 << 2);				///< 1 = Booted from ROM.
	static constexpr uint8_t RDM_CAPABLE = (1 << 1);			///< 1 = Capable of Remote Device Management
	static constexpr uint8_t UBEA_PRESENT = (1 << 0);			///< 1 = UBEA present
};

struct Status2 {
	static constexpr uint8_t WEB_BROWSER_SUPPORT = (1U << 0);	///< Bit 0, Set = Product supports web browser configuration
	static constexpr uint8_t IP_MANUALY = (0 << 1);				///< Bit 1, Clr = Node’s IP is manually configured.
	static constexpr uint8_t IP_DHCP = (1U << 1);				///< Bit 1, Set = Node’s IP is DHCP configured.
	static constexpr uint8_t DHCP_NOT_CAPABLE = (0 << 2);		///< Bit 2, Clr = Node is not DHCP capable.
	static constexpr uint8_t DHCP_CAPABLE = (1U << 2);			///< Bit 2, Set = Node is DHCP capable.
	static constexpr uint8_t PORT_ADDRESS_8BIT = (0 << 3);		///< Bit 3, Clr = Node supports 8 bit Port-Address (Art-Net II).
	static constexpr uint8_t PORT_ADDRESS_15BIT = (1U << 3);	///< Bit 3, Set = Node supports 15 bit Port-Address (Art-Net 3 or 4).
	static constexpr uint8_t SACN_NO_SWITCH = (0 << 4);			///< Bit 4, Clr = Node not able to switch between Art-Net and sACN.
	static constexpr uint8_t SACN_ABLE_TO_SWITCH = (1U << 4);	///< Bit 4, Set = Node is able to switch between Art-Net and sACN.
};

struct Status3 {
	static constexpr uint8_t NETWORKLOSS_OFF_STATE = (1U << 6);	///< bit 76 = 01 If network data is lost, it will set all outputs to off state
	static constexpr uint8_t SUPPORTS_LLRP = (1U << 4);			///< bit 4 = 1 Node supports LLRP (Low Level Recovery Protocol
};

struct TalkToMe {
	static constexpr auto SEND_ARTP_ON_CHANGE = (1 << 1);	///< Bit 1 set : Send ArtPollReply whenever Node conditions change.
	static constexpr auto SEND_DIAG_MESSAGES = (1 << 2);	///< Bit 2 set : Send me diagnostics messages.
	static constexpr auto SEND_DIAG_UNICAST = (1 << 3);		///< Bit 3 : 0 = Diagnostics messages are broadcast. (if bit 2).													///< Bit 3 : 1 = Diagnostics messages are unicast. (if bit 2).
};

struct GoodOutput {
	static constexpr uint8_t GO_DATA_IS_BEING_TRANSMITTED = (1 << 7);	///< Bit 7 Set – Data is being transmitted.
	static constexpr uint8_t GO_INCLUDES_DMX_TEST_PACKETS = (1 << 6);	///< Bit 6 Set – Channel includes DMX512 test packets.
	static constexpr uint8_t GO_INCLUDES_DMX_SIP = (1 << 5);			///< Bit 5 Set – Channel includes DMX512 SIP’s.
	static constexpr uint8_t GO_INCLUDES_DMX_TEXT_PACKETS = (1 << 4);	///< Bit 4 Channel includes DMX512 text packets.
	static constexpr uint8_t GO_OUTPUT_IS_MERGING = (1 << 3);			///< Bit 3 Set – Output is merging ArtNet data.
	static constexpr uint8_t GO_DMX_SHORT_DETECTED = (1 << 2);			///< Bit 2 Set – DMX output short detected on power up
	static constexpr uint8_t GO_MERGE_MODE_LTP = (1 << 1);				///< Bit 1 Set – Merge Mode is LTP.
	static constexpr uint8_t GO_OUTPUT_IS_SACN  = (1 << 0);				///< Bit 0 Set – Output is selected to transmit sACN.
	static constexpr uint8_t GO_OUTPUT_NONE = 0;
};

struct GoodInput {
	static constexpr uint8_t GI_DATA_RECIEVED  = (1U << 7);	///< Bit 7 Set – Data received.
	static constexpr uint8_t GI_DISABLED  = (1U << 3);		///< Bit 3 Set – Input is disabled.
};

}  // namespace artnet

struct ArtNet {
#if !defined(ARTNET_VERSION)
	static constexpr uint8_t VERSION = 4;
#else
	static constexpr uint8_t VERSION = ARTNET_VERSION;
#endif
	static constexpr uint8_t PROTOCOL_REVISION = 14;
	static constexpr uint16_t UDP_PORT = 0x1936;
	static constexpr uint8_t PORTS = 4;
#if !defined(LIGHTSET_PORTS)
	static constexpr uint8_t PAGES = 1;
#else
	static constexpr uint8_t PAGES = ((LIGHTSET_PORTS + 3) / 4);
#endif
	static constexpr uint16_t DMX_LENGTH = 512;
	static constexpr uint32_t SHORT_NAME_LENGTH = 18;
	static constexpr uint32_t LONG_NAME_LENGTH = 64;
	static constexpr uint32_t REPORT_LENGTH = 64;
	static constexpr uint32_t RDM_UID_WIDTH = 6;
	static constexpr uint32_t MAC_SIZE = 6;
	static constexpr uint32_t IP_SIZE = 4;
	static constexpr uint32_t ESTA_SIZE = 2;

	static const char* GetProtocolMode(artnet::PortProtocol p, bool bToUpper = false) {
		if (bToUpper) {
			return (p == artnet::PortProtocol::ARTNET) ? "Art-Net" : "sACN";
		}
		return (p == artnet::PortProtocol::ARTNET) ? "artnet" : "sacn";
	}

	static const char* GetProtocolMode(unsigned p, bool bToUpper = false) {
		return GetProtocolMode(static_cast<artnet::PortProtocol>(p), bToUpper);
	}
};

#endif /* ARTNET_H_ */
