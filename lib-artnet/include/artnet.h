/**
 * @file artnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if  !defined (PACKED)
#define PACKED __attribute__((packed))
#endif

namespace artnet {
#if !defined(ARTNET_VERSION)
 static constexpr uint32_t VERSION = 4;
#else
 static constexpr uint32_t VERSION = ARTNET_VERSION;
#endif
static constexpr uint8_t PROTOCOL_REVISION = 14;
static constexpr uint32_t PORTS = 4;
static constexpr uint16_t UDP_PORT = 0x1936;
static constexpr uint32_t DMX_LENGTH = 512;
static constexpr uint32_t SHORT_NAME_LENGTH = 18;
static constexpr uint32_t LONG_NAME_LENGTH = 64;
static constexpr uint32_t REPORT_LENGTH = 64;
static constexpr uint32_t RDM_UID_WIDTH = 6;
static constexpr uint32_t MAC_SIZE = 6;
static constexpr uint32_t IP_SIZE = 4;
static constexpr uint32_t ESTA_SIZE = 2;

static constexpr char NODE_ID[] = "Art-Net";			///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
static constexpr uint32_t MERGE_TIMEOUT_SECONDS = 10;
static constexpr uint32_t NETWORK_DATA_LOSS_TIMEOUT = 10;	///< Seconds

enum class PortProtocol {
	ARTNET,	///< Output both DMX512 and RDM packets from the Art-Net protocol (default).
	SACN	///< Output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
};

/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum class ReportCode : uint8_t {
	RCDEBUG,
	RCPOWEROK,
	RCPOWERFAIL,
	RCSOCKETWR1,
	RCPARSEFAIL,
	RCUDPFAIL,
	RCSHNAMEOK,
	RCLONAMEOK,
	RCDMXERROR,
	RCDMXUDPFULL,
	RCDMXRXFULL,
	RCSWITCHERR,
	RCCONFIGERR,
	RCDMXSHORT,
	RCFIRMWAREFAIL,
	RCUSERFAIL
};

enum class Status : uint8_t {
	OFF, STANDBY, ON
};

/**
 * Table 4 – Style Codes
 * The Style code defines the general functionality of a Controller.
 * The Style code is returned in ArtPollReply.
 */
enum class StyleCode: uint8_t {
	NODE = 0x00,	///< A DMX to / from Art-Net device
	SERVER = 0x01,	///< A lighting console.
	MEDIA = 0x02,	///< A Media Server.
	ROUTE = 0x03,	///< A network routing device.
	BACKUP = 0x04,	///< A backup device.
	CONFIG = 0x05,	///< A configuration or diagnostic tool.
	VISUAL = 0x06,	///< A visualizer.
};

/**
 * Table 5 – Priority Codes
 * Diagnostics Priority codes.
 * These are used in ArtPoll and ArtDiagData.
 */
enum class PriorityCodes: uint8_t {
	DIAG_LOW = 0x10,
	DIAG_MED = 0x40,
	DIAG_HIGH = 0x80,
	DIAG_CRITICAL = 0xE0,
	DIAG_VOLATILE = 0xF0,	///< Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types are displayed in a list box.
};

struct PortType {
	static constexpr uint8_t PROTOCOL_DMX = 0x00;	///< DMX-512
	static constexpr uint8_t PROTOCOL_MIDI = 0x01; 	///< MIDI
	static constexpr uint8_t PROTOCOL_AVAB = 0x02;	///< Avab
	static constexpr uint8_t PROTOCOL_CMX = 0x03;	///< Colortran CMX
	static constexpr uint8_t PROTOCOL_ADB = 0x04;	///< ABD 62.5
	static constexpr uint8_t PROTOCOL_ARTNET = 0x05;///< ArtNet
	static constexpr uint8_t INPUT_ARTNET = 0x40;	///< Set if this channel can input onto the Art-Net network
	static constexpr uint8_t OUTPUT_ARTNET = 0x80;	///< Set if this channel can output data from the Art-Net network
};

struct PortCommand {
	static constexpr uint8_t NONE = 0x00;			///< No action
	static constexpr uint8_t CANCEL = 0x01;			///< If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
	static constexpr uint8_t LED_NORMAL = 0x02;		///< The front panel indicators of the Node operate normally.
	static constexpr uint8_t LED_MUTE = 0x03;		///< The front panel indicators of the Node are disabled and switched off.
	static constexpr uint8_t LED_LOCATE = 0x04;		///< Rapid flashing of the Node’s front panel indicators. It is intended as an outlet locator for large installations.
	static constexpr uint8_t RESET = 0x05;			///< Resets the Node’s Sip, Text, Test and data error flags.

	static constexpr uint8_t FAIL_HOLD = 0x08;		///< Failsafe mode = hold last state
	static constexpr uint8_t FAIL_ZERO = 0x09;		///< Failsafe mode = clear outputs
	static constexpr uint8_t FAIL_FULL = 0x0a;		///< Failsafe mode = outputs to full
	static constexpr uint8_t FAIL_SCENE = 0x0b;		///< Failsafe mode = playback failsafe scene
	static constexpr uint8_t FAIL_RECORD = 0x0c;	///< Failsafe mode = record current output as failsafe scene

	static constexpr uint8_t MERGE_LTP_O = 0x10;	///< Set DMX Port 0 to Merge in LTP mode.
	static constexpr uint8_t MERGE_LTP_1 = 0x11;	///< Set DMX Port 1 to Merge in LTP mode.
	static constexpr uint8_t MERGE_LTP_2 = 0x12;	///< Set DMX Port 2 to Merge in LTP mode.
	static constexpr uint8_t MERGE_LTP_3 = 0x13;	///< Set DMX Port 3 to Merge in LTP mode.

	static constexpr uint8_t DIRECTION_TX_O = 0x20;	///< Set Port 0 direction to Tx.
	static constexpr uint8_t DIRECTION_TX_1 = 0x21;	///< Set Port 1 direction to Tx.
	static constexpr uint8_t DIRECTION_TX_2 = 0x22;	///< Set Port 2 direction to Tx.
	static constexpr uint8_t DIRECTION_TX_3 = 0x23;	///< Set Port 3 direction to Tx.

	static constexpr uint8_t DIRECTION_RX_O = 0x30;	///< Set Port 0 direction to Rx.
	static constexpr uint8_t DIRECTION_RX_1 = 0x31;	///< Set Port 1 direction to Rx.
	static constexpr uint8_t DIRECTION_RX_2 = 0x32;	///< Set Port 2 direction to Rx.
	static constexpr uint8_t DIRECTION_RX_3 = 0x33;	///< Set Port 3 direction to Rx.

	static constexpr uint8_t MERGE_HTP_0 = 0x50;	///< Set DMX Port 0 to Merge in HTP (default) mode.
	static constexpr uint8_t MERGE_HTP_1 = 0x51;	///< Set DMX Port 1 to Merge in HTP (default) mode.
	static constexpr uint8_t MERGE_HTP_2 = 0x52;	///< Set DMX Port 2 to Merge in HTP (default) mode.
	static constexpr uint8_t MERGE_HTP_3 = 0x53;	///< Set DMX Port 3 to Merge in HTP (default) mode.

	static constexpr uint8_t ARTNET_SEL0 = 0x60;	///< Set DMX Port 0 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t ARTNET_SEL1 = 0x61;	///< Set DMX Port 1 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t ARTNET_SEL2 = 0x62;	///< Set DMX Port 2 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	static constexpr uint8_t ARTNET_SEL3 = 0x63;	///< Set DMX Port 3 to output both DMX512 and RDM packets from the Art-Net protocol (default).

	static constexpr uint8_t ACN_SEL0 = 0x70;		///< Set DMX Port 0 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t ACN_SEL1 = 0x71;		///< Set DMX Port 1 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t ACN_SEL2 = 0x72;		///< Set DMX Port 2 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	static constexpr uint8_t ACN_SEL3 = 0x73;		///< Set DMX Port 3 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.

	static constexpr uint8_t CLR_0 = 0x90;			///< Clear DMX Output buffer for Port 0
	static constexpr uint8_t CLR_1 = 0x91;			///< Clear DMX Output buffer for Port 1
	static constexpr uint8_t CLR_2 = 0x92;			///< Clear DMX Output buffer for Port 2
	static constexpr uint8_t CLR_3 = 0x93;			///< Clear DMX Output buffer for Port 3

	static constexpr uint8_t STYLE_DELTA0 = 0xa0;	///< Set output style to delta on output port 0
	static constexpr uint8_t STYLE_DELTA1 = 0xa1;	///< Set output style to delta on output port 1
	static constexpr uint8_t STYLE_DELTA2 = 0xa2;	///< Set output style to delta on output port 2
	static constexpr uint8_t STYLE_DELTA3 = 0xa3;	///< Set output style to delta on output port 3

	static constexpr uint8_t STYLE_CONSTANT0 = 0xb0;///< Set output style to continuous on output port 0
	static constexpr uint8_t STYLE_CONSTANT1 = 0xb1;///< Set output style to continuous on output port 1
	static constexpr uint8_t STYLE_CONSTANT2 = 0xb2;///< Set output style to continuous on output port 2
	static constexpr uint8_t STYLE_CONSTANT3 = 0xb3;///< Set output style to continuous on output port 3

	static constexpr uint8_t RDM_ENABLE0 = 0xc0;	///< Enable RDM on output port 0
	static constexpr uint8_t RDM_ENABLE1 = 0xc1;	///< Enable RDM on output port 1
	static constexpr uint8_t RDM_ENABLE2 = 0xc2;	///< Enable RDM on output port 2
	static constexpr uint8_t RDM_ENABLE3 = 0xc3;	///< Enable RDM on output port 3

	static constexpr uint8_t RDM_DISABLE0 = 0xd0;	///< Disable RDM on output port 0
	static constexpr uint8_t RDM_DISABLE1 = 0xd1;	///< Disable RDM on output port 1
	static constexpr uint8_t RDM_DISABLE2 = 0xd2;	///< Disable RDM on output port 2
	static constexpr uint8_t RDM_DISABLE3 = 0xd3;	///< Disable RDM on output port 3
};

struct TodControlCommand {
	static constexpr uint8_t ATC_NONE = 0x00;	///< No action.
	static constexpr uint8_t ATC_FLUSH = 0x01;	///< The port flushes its TOD and instigates full discovery.
	static constexpr uint8_t ATC_END = 0x02;	///< The port ends current discovery but does not flush ToD.
	static constexpr uint8_t ATC_INCON = 0x03;	///< The port enables incremental discovery.
	static constexpr uint8_t ATC_INCOFF = 0x04;	///< The port disables incremental discovery.
};

struct Program {
	static constexpr uint8_t NO_CHANGE = 0x7F;
	static constexpr uint8_t DEFAULTS = 0x00;
	static constexpr uint8_t CHANGE_MASK = 0x80;
};

struct Status1 {
	static constexpr uint8_t INDICATOR_MASK = (3U << 6);		///< 0b11 bit 7-6, Indicator state.
	static constexpr uint8_t INDICATOR_LOCATE_MODE = (1U << 6);	///< 0b01 Indicators in Locate Mode.
	static constexpr uint8_t INDICATOR_MUTE_MODE = (2U << 6);	///< 0b10 Indicators in Mute Mode.
	static constexpr uint8_t INDICATOR_NORMAL_MODE = (3U << 6);	///< 0b11 Indicators in Normal Mode.
	static constexpr uint8_t PAP_MASK = (3U << 4);				///< 0b11 bit 5-4, Port Address Programming Authority
	static constexpr uint8_t PAP_UNKNOWN = (0 << 4);			///< 0b00 Port Address Programming Authority unknown.
	static constexpr uint8_t PAP_FRONT_PANEL = (1U << 4);		///< 0b01 All Port Address set by front panel controls.
	static constexpr uint8_t PAP_NETWORK = (2U << 4);			///< 0b10 All or part of Port Address programmed by network or Web browser.
	static constexpr uint8_t PAP_NOTUSED = (3U << 4);			///< 0b11 Not used.
	static constexpr uint8_t NORMAL_FIRMWARE_BOOT = (0 << 2);	///< 0 = Normal firmware boot (from flash).
	static constexpr uint8_t ROM_BOOT = (1U << 2);				///< 1 = Booted from ROM.
	static constexpr uint8_t RDM_CAPABLE = (1U << 1);			///< 1 = Capable of Remote Device Management
	static constexpr uint8_t UBEA_PRESENT = (1U << 0);			///< 1 = UBEA present
};

struct Status2 {
	static constexpr uint8_t WEB_BROWSER_NO_SUPPORT = (0 << 0);	///< bit 0 = 0 Node does not support web browser
	static constexpr uint8_t WEB_BROWSER_SUPPORT = (1U << 0);	///< bit 0 = 1 Node supports web browser configuration
	static constexpr uint8_t IP_MANUALY = (0 << 1);				///< bit 1 = 0 Node's IP address is manually configured
	static constexpr uint8_t IP_DHCP = (1U << 1);				///< bit 1 = 1 Node's IP address is DHCP configured
	static constexpr uint8_t DHCP_NOT_CAPABLE = (0 << 2);		///< bit 2 = 0 Node is not DHCP capable
	static constexpr uint8_t DHCP_CAPABLE = (1U << 2);			///< bit 2 = 1 Node is DHCP capable
	static constexpr uint8_t PORT_ADDRESS_8BIT = (0 << 3);		///< bit 3 = 0 Node supports 8 bit Port-Address (Art-Net II).
	static constexpr uint8_t PORT_ADDRESS_15BIT = (1U << 3);	///< bit 3 = 1 Node supports 15 bit Port-Address (Art-Net 3 or 4).
	static constexpr uint8_t SACN_NO_SWITCH = (0 << 4);			///< bit 4 = 0 Node is not able to switch Art-Net/sACN
	static constexpr uint8_t SACN_ABLE_TO_SWITCH = (1U << 4);	///< bit 4 = 1 Node is able to switch Art-Net/sACN
	static constexpr uint8_t OUTPUT_STYLE_NO_SWITCH = (0 << 6);	///< bit 6 = 0 Node is not able to switch output style by ArtCommand
	static constexpr uint8_t OUTPUT_STYLE_SWITCH = (1U << 6);	///< bit 6 = 1 Node is able to switch output style by ArtCommand
	static constexpr uint8_t RDM_NO_SWITCH = (0 << 7);			///< bit 7 = 0 Node is not able to enable or disable RDM by ArtCommand
	static constexpr uint8_t RDM_SWITCH = (1U << 7);			///< bit 7 = 1 Node is able to enable or disable RDM by ArtCommand
};

struct Status3 {
	static constexpr uint8_t NETWORKLOSS_MASK = (3U << 6);				///< bit 76
	static constexpr uint8_t NETWORKLOSS_LAST_STATE = (0 << 6);			///< bit 76 = 00 If network data is lost, it will hold last state
	static constexpr uint8_t NETWORKLOSS_OFF_STATE = (1U << 6);			///< bit 76 = 01 If network data is lost, it will set all outputs to off state
	static constexpr uint8_t NETWORKLOSS_ON_STATE = (2U << 6);			///< bit 76 = 10 If network data is lost, it will set all outputs to full on
	static constexpr uint8_t NETWORKLOSS_PLAYBACK = (3U << 6);			///< bit 76 = 11 If network data is lost, it will playback the fail-over scene
	static constexpr uint8_t FAILSAFE_NO_CONTROL = (0 << 5);			///< bit 5 = 0 Node is not able to control failsafe mode by ArtCommand
	static constexpr uint8_t FAILSAFE_CONTROL = (1U << 5);				///< bit 5 = 1 Node is able to control failsafe mode by ArtCommand
	static constexpr uint8_t SUPPORTS_LLRP = (1U << 4);					///< bit 4 = 1 Node supports LLRP (Low Level Recovery Protocol
	static constexpr uint8_t OUTPUT_NO_SWITCH = (0 << 3);  				///< bit 3 = 0 Outputs cannot be switched to an input
	static constexpr uint8_t OUTPUT_SWITCH = (1U << 3);  				///< bit 3 = 1 Outputs can be switched to an input
	static constexpr uint8_t SUPPORTS_RDMNET = (1U << 2);				///< bit 2 = 1 Node supports RDMnet
	static constexpr uint8_t SUPPORTS_BACKGROUNDQUEUE = (1U << 1);		///< bit 1 = 1 BackgroundQueue is supported
	static constexpr uint8_t SUPPORTS_BACKGROUNDDISCOVERY = (1U << 0);	///< bit 0 = 1 Programmable background discovery is supported.
};

struct Flags {
																///< bit 1 = 0 Node only sends ArtPollReply when polled
	static constexpr uint8_t SEND_ARTP_ON_CHANGE = (1U << 1);	///< bit 1 = 1 Node sends ArtPollReply when it needs to
	 	 	 	 	 	 	 	 	 	 	 	 	 	 		///< bit 2 = 0 Do not send me diagnostic messages
	static constexpr uint8_t SEND_DIAG_MESSAGES = (1U << 2);	///< bit 2 = 1 Send me diagnostics messages.
																///< bit 3 = 0 Diagnostics messages are broadcast. (if bit 2).
	static constexpr uint8_t SEND_DIAG_UNICAST = (1U << 3);		///< bit 3 = 1 Diagnostics messages are unicast. (if bit 2).
																///< bit 5 = 0 Ignore TargetPortAddress
	static constexpr uint8_t USE_TARGET_PORT_ADDRESS = (1U < 5);///< bit 5 = 1 Only reply if device has a Port-Address that is
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	///<		   inclusively in the range TargetPortAddressBottom to
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	///<		   TargetPortAddressTop.
};

struct GoodOutput {
	static constexpr uint8_t DATA_IS_BEING_TRANSMITTED = (1U << 7);	///< bit 7 data is transmitting
	static constexpr uint8_t INCLUDES_DMX_TEST_PACKETS = (1U << 6);	///< bit 6 data includes test packets
	static constexpr uint8_t INCLUDES_DMX_SIP = (1U << 5);			///< bit 5 data includes SIP's
	static constexpr uint8_t INCLUDES_DMX_TEXT_PACKETS = (1U << 4);	///< bit 4 data includes text
	static constexpr uint8_t OUTPUT_IS_MERGING = (1U << 3);			///< bit 3 output is merging data.
	static constexpr uint8_t DMX_SHORT_DETECTED = (1U << 2);		///< bit 2 set if DMX output short detected on power up
	static constexpr uint8_t MERGE_MODE_LTP = (1U << 1);			///< bit 1 set if DMX output merge mode is LTP
	static constexpr uint8_t OUTPUT_IS_SACN  = (1U << 0);			///< bit 0 set if outputting sACN.
	static constexpr uint8_t OUTPUT_NONE = 0;
};

struct GoodOutputB {
	static constexpr uint8_t RDM_DISABLED = (1U << 7);			///< bit 7 = 1 RDM is disabled.
	static constexpr uint8_t RDM_ENABLED = (0 << 7);			///< bit 7 = 0 RDM is enabled.
	static constexpr uint8_t STYLE_CONSTANT = (1U << 6);		///< bit 6 = 1 Output style is continuous.
	static constexpr uint8_t STYLE_DELTA = (0 << 6);			///< bit 6 = 0 Output style is delta.
	static constexpr uint8_t DISCOVERY_NOT_RUNNING = (1U << 5);	///< bit 5 = 1 Discovery is currently not running.
	static constexpr uint8_t DISCOVERY_IS_RUNNING = (0 << 5);	///< bit 5 = 0 Discovery is currently running.
	static constexpr uint8_t DISCOVERY_DISABLED = (1U << 4);	///< bit 4 = 1 Background discovery is disabled.
	static constexpr uint8_t DISCOVERY_ENABLED = (0 << 4);		///< bit 4 = 0 Background Discovery is enabled.
};

struct GoodInput {
	static constexpr uint8_t DATA_RECIEVED = (1U << 7);			///< bit 7 is data received
	static constexpr uint8_t INCLUDES_TEST_PACKETS = (1U << 6);	///< bit 6 is data includes test packets
	static constexpr uint8_t INCLUDES_SIP = (1U << 5);			///< bit 5 is data includes SIP's
	static constexpr uint8_t INCLUDES_TEXT = (1U << 4);			///< bit 4 is data includes text
	static constexpr uint8_t DISABLED = (1U << 3);				///< bit 3 set is input is disabled
	static constexpr uint8_t ERRORS = (1U << 2);				///< bit 2 is receive errors
};

inline const char *get_protocol_mode(const artnet::PortProtocol p, const bool bToUpper = false) {
	if (bToUpper) {
		return (p == artnet::PortProtocol::ARTNET) ? "Art-Net" : "sACN";
	}
	return (p == artnet::PortProtocol::ARTNET) ? "artnet" : "sacn";
}

inline const char *get_protocol_mode(const unsigned p, const bool bToUpper = false) {
	return get_protocol_mode(static_cast<artnet::PortProtocol>(p), bToUpper);
}

inline artnet::PortProtocol get_protocol_mode(const char *pProtocolMode) {
	if (pProtocolMode != nullptr) {
		if (((pProtocolMode[0] | 0x20) == 's')
		 && ((pProtocolMode[1] | 0x20) == 'a')
		 && ((pProtocolMode[2] | 0x20) == 'c')
		 && ((pProtocolMode[3] | 0x20) == 'n')) {
			return artnet::PortProtocol::SACN;
		}
	}
	return artnet::PortProtocol::ARTNET;
}

inline uint16_t make_port_address(const uint32_t nNetSwitch, const uint32_t nSubSwitch, const uint32_t nUniverse) {
	// PortAddress Bit 15 = 0
	uint16_t nPortAddress = (nNetSwitch & 0x7F) << 8;					// Net : Bits 14-8
	nPortAddress |= static_cast<uint16_t>((nSubSwitch & 0x0F) << 4);	// Sub-Net : Bits 7-4
	nPortAddress |= static_cast<uint16_t>(nUniverse & 0x0F);			// Universe : Bits 3-0

	return nPortAddress;
}

/**
 * Table 1 - OpCodes
 * The supported legal OpCode values used in Art-Net packets
 */
enum class OpCodes: uint16_t {
	OP_POLL = 0x2000,		///< This is an ArtPoll packet, no other data is contained in this UDP packet.
	OP_POLLREPLY = 0x2100,	///< This is an ArtPollReply Packet. It contains device status information.
	OP_DIAGDATA = 0x2300,	///< Diagnostics and data logging packet.
	OP_DMX = 0x5000,		///< This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
	OP_SYNC = 0x5200,		///< This is an ArtSync data packet. It is used to force synchronous transfer of ArtDmx packets to a node’s output.
	OP_ADDRESS = 0x6000,	///< This is an ArtAddress packet. It contains remote programming information for a Node.
	OP_INPUT = 0x7000,		///< This is an ArtInput packet. It contains enable – disable data for DMX inputs.
	OP_TODREQUEST = 0x8000,	///< This is an ArtTodRequest packet. It is used to request a Table of Devices (ToD) for RDM discovery.
	OP_TODDATA = 0x8100,	///< This is an ArtTodData packet. It is used to send a Table of Devices (ToD) for RDM discovery.
	OP_TODCONTROL = 0x8200,	///< This is an ArtTodControl packet. It is used to send RDM discovery control messages
	OP_RDM = 0x8300, 		///< This is an ArtRdm packet. It is used to send all non discovery RDM messages.
	OP_RDMSUB = 0x8400, 	///< This is an ArtRdmSub packet. It is used to send compressed, RDM Sub-Device data.
	OP_TIMECODE = 0x9700,	///< This is an ArtTimeCode packet. It is used to transport time code over the network.
	OP_TIMESYNC = 0x9800,	///< Used to synchronize real time date and clock
	OP_TRIGGER = 0x9900,	///< Used to send trigger macros
	OP_DIRECTORY = 0x9A00,	///< Requests a node's file list
	OP_IPPROG = 0xF800,		///< This is an ArtIpProg packet. It is used to re-programm the IP, Mask and Port address of the Node.
	OP_IPPROGREPLY = 0xF900,///< This is an ArtIpProgReply packet. It is returned by the node to acknowledge receipt of an ArtIpProg packet.
	OP_NOT_DEFINED = 0x0000	///< OP_NOT_DEFINED
};

struct ArtPoll {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the OpCode listing.
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Flags;			///< Set behavior of Node
	uint8_t DiagPriority;	///< The lowest priority of diagnostics message that should be sent. See \ref TPriorityCodes
	uint8_t TargetPortAddressTopHi;
	uint8_t TargetPortAddressTopLo;
	uint8_t TargetPortAddressBottomHi;
	uint8_t TargetPortAddressBottomLo;
}PACKED;

struct ArtPollReply {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpPollReply \ref TOpCodes
	uint8_t IPAddress[4];	///< Array containing the Node’s IP address. First array entry is most significant byte of address.
	uint16_t Port;			///< The Port is always 0x1936
	uint8_t VersInfoH;		///< High byte of Node’s firmware revision number.
	uint8_t VersInfoL;		///< Low byte of Node’s firmware revision number.
	uint8_t NetSwitch;		///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t SubSwitch;		///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	uint8_t OemHi;			///< The high byte of the Oem value
	uint8_t Oem;			///< The low byte of the Oem value. The Oem word describes the equipment vendor and the feature set available.
	uint8_t Ubea;			///< This field contains the firmware version of the User Bios Extension Area (UBEA). If the UBEA is not programmed, this field contains zero.
	uint8_t Status1;		///< General Status register
	uint8_t EstaMan[2];		///< The ESTA manufacturer code. These codes are used to represent equipment manufacturer. They are assigned by ESTA.
	uint8_t ShortName[artnet::SHORT_NAME_LENGTH];///< The array represents a null terminated short name for the Node.
	uint8_t LongName[artnet::LONG_NAME_LENGTH];	///< The array represents a null terminated long name for the Node.
	uint8_t NodeReport[artnet::REPORT_LENGTH];	///< The array is a textual report of the Node’s operating status or operational errors. It is primarily intended for ‘engineering’ data rather than ‘end user’ data.
	uint8_t NumPortsHi;		///< The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
	uint8_t NumPortsLo;		///< The low byte of the word describing the number of input or output ports.
	uint8_t PortTypes[artnet::PORTS];	///< This array defines the operation and protocol of each channel.
	uint8_t GoodInput[artnet::PORTS];	///< This array defines input status of the node.
	uint8_t GoodOutput[artnet::PORTS];	///< This array defines output status of the node.
	uint8_t SwIn[artnet::PORTS];			///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible input ports are encoded into the low nibble.
	uint8_t SwOut[artnet::PORTS];		///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible output ports are encoded into the low nibble.
	uint8_t	AcnPriority; 	///< sACN Priority for sACN generated by this node
	uint8_t SwMacro;		///< If the Node supports macro key inputs, this byte represents the trigger values.
	uint8_t SwRemote;		///< If the Node supports remote trigger inputs, this byte represents the trigger values.
	uint8_t Spare1;			///< Not used, set to zero
	uint8_t Spare2;			///< Not used, set to zero
	uint8_t Spare3;			///< Not used, set to zero
	uint8_t Style;			///< The Style code defines the equipment style of the device. See \ref TNodeStyleCode
	uint8_t MAC[artnet::MAC_SIZE];	///< MAC Address
	uint8_t BindIp[4];		///< If this unit is part of a larger or modular product, this is the IP of the root device.
	uint8_t BindIndex;		///< Set to zero if no binding, otherwise this number represents the order of bound devices. A lower number means closer to root device. A value of 1 means root device.
	uint8_t Status2;		///<
	uint8_t GoodOutputB[artnet::PORTS];	///< This array defines output status of the node.
	uint8_t Status3;					///< General Status register
	uint8_t DefaultUidResponder[6];		///< RDMnet & LLRP UID
	uint8_t UserHi;			///< Available for user specific data
	uint8_t UserLo;			///< Available for user specific data
	uint8_t RefreshRateHi;	///< Hi byte of RefreshRate
	uint8_t RefreshRateLo;	///< Lo Byte of RefreshRate.
	 	 	 	 	 	 	///< RefreshRate allows the device to specify the maximum refresh rate, expressed in Hz, at which it can process ArtDmx.
	 	 	 	 	 	 	///< This is designed to allow refresh rates above DMX512 rates, for gateways that implement other protocols such as SPI.
	 	 	 	 	 	 	///< A value of 0 to 44 represents the maximum DMX512 rate of 44Hz.
	uint8_t BackgroundQueuePolicy; ///< The BackgroundQueuePolicy defines the method by with the node retrieves STATUS_MESSAGE and QUEUED_MESSAGE pids from the connected RDM devices.
	uint8_t Filler[10];	///< Transmit as zero. For future expansion.
}PACKED;

/**
 * ArtDmx is the data packet used to transfer DMX512 data.
 */
struct ArtDmx {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpDmx \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Sequence;		///< The sequence number is used to ensure that ArtDmx packets are used in the correct order.
	uint8_t Physical;		///< The physical input port from which DMX512 data was input.
	uint16_t PortAddress;	///< The 15 bit Port-Address to which this packet is destined.
	uint8_t LengthHi;		///< The length of the DMX512 data array. This value should be an even number in the range 2 – 512.
	uint8_t Length;			///< Low Byte of above.
	uint8_t Data[artnet::DMX_LENGTH];///< A variable length array of DMX512 lighting data.
}PACKED;

struct ArtDiagData {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpDiagData See \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Ignore by receiver, set to zero by sender
	uint8_t Priority;		///< The priority of this diagnostic data. See \ref TPriorityCodes
	uint8_t Filler2;		///< Ignore by receiver, set to zero by sender
	uint8_t Filler3;		///< Ignore by receiver, set to zero by sender
	uint8_t LengthHi;		///< The length of the text array below. High Byte.
	uint8_t LengthLo;		///< Low byte
	uint8_t Data[512];		///< ASCII text array, null terminated. Max length is 512 bytes including the null terminator.
}PACKED;

struct ArtSync {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpSync \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Aux1;			///< Transmit as zero.
	uint8_t Aux2;			///< Transmit as zero.
}PACKED;

/**
 * ArtAddress packet definition
 *
 * Fields 5 to 13 contain the data that will be programmed into the node.
 */
struct ArtAddress {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t NetSwitch;		///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t BindIndex;		///< The BindIndex defines the bound node which originated this packet and is used to uniquely identify the bound node when identical IP addresses are in use.
	uint8_t ShortName[artnet::SHORT_NAME_LENGTH];///< The Node will ignore this value if the string is null.
	uint8_t LongName[artnet::LONG_NAME_LENGTH];	///< The Node will ignore this value if the string is null.
	uint8_t SwIn[artnet::PORTS];	///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SwOut[artnet::PORTS];///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SubSwitch;		///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
	uint8_t SwVideo;		///< Reserved
	uint8_t Command;		///< Node configuration commands \ref TArtnetPortCommand
}PACKED;

/**
 * ArtInput packet definition
 *
 * A Controller or monitoring device on the network can
 * enable or disable individual DMX512 inputs on any of the network nodes.
 *
 * All nodes power on with all inputs enabled.
 */

struct ArtInput {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Pad length to match ArtPoll.
	uint8_t BindIndex;		///< The BindIndex defines the bound node which originated this packet and is used to uniquely identify the bound node when identical IP addresses are in use.
	uint8_t NumPortsHi;		///< The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
	uint8_t NumPortsLo;		///< The low byte of the word describing the number of input or output ports.
	uint8_t Input[artnet::PORTS];	///< This array defines input disable status of each channel.
}PACKED;

/**
 * ArtTimeCode packet definition
 *
 * ArtTimeCode allows time code to be transported over the network.
 * The data format is compatible with both longitudinal time code and MIDI time code.
 * The four key types of Film, EBU, Drop Frame and SMPTE are also encoded.
 *
 */
struct ArtTimeCode {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Ignore by receiver, set to zero by sender
	uint8_t Filler2;		///< Ignore by receiver, set to zero by sender
	uint8_t Frames;			///< Frames time. 0 – 29 depending on mode.
	uint8_t Seconds;		///< Seconds. 0 - 59.
	uint8_t Minutes;		///< Minutes. 0 - 59.
	uint8_t Hours;			///< Hours. 0 - 59.
	uint8_t Type;			///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
}PACKED;

struct ArtTimeSync {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Ignore by receiver, set to zero by sender
	uint8_t Filler2;		///< Ignore by receiver, set to zero by sender
	uint8_t Prog;			///<
	uint8_t tm_sec;			///<
	uint8_t tm_min;			///<
	uint8_t tm_hour;		///<
	uint8_t tm_mday;		///<
	uint8_t tm_mon;			///<
	uint8_t tm_year_hi;		///<
	uint8_t tm_year_lo;		///<
	uint8_t tm_wday;		///<
	uint8_t tm_isdst;		///<
}PACKED;

/**
 * ArtTodRequest packet definition
 *
 * This packet is used to request the Table of RDM Devices (TOD).
 * A Node receiving this packet must not interpret it as forcing full discovery.
 * Full discovery is only initiated at power on or when an ArtTodControl.AtcFlush is received.
 * The response is ArtTodData.
 */
struct ArtTodRequest {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Pad length to match ArtPoll.
	uint8_t Filler2;		///< Pad length to match ArtPoll.
	uint8_t Spare1;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare2;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare3;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare4;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare5;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare6;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare7;			///< Transmit as zero, receivers don’t test.
	uint8_t Net;			///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
	uint8_t Command;		///< 0x00 TodFull Send the entire TOD.
	uint8_t AddCount;		///< The array size of the Address field. Max value is 32.
	uint8_t Address[32];	///< This array defines the low byte of the Port-Address of the Output Gateway nodes that must respond to this packet.
}PACKED;

/**
 * The ArtTodControl packet is used to send RDM control parameters over Art-Net.
 * The response is ArtTodData.
 */
struct ArtTodControl {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;		///< Pad length to match ArtPoll.
	uint8_t Filler2;		///< Pad length to match ArtPoll.
	uint8_t Spare1;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare2;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare3;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare4;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare5;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare6;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare7;			///< Transmit as zero, receivers don’t test.
	uint8_t Net;			///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
	uint8_t Command;		///< 0x00 AtcNone No action. 0x01 AtcFlush The node flushes its TOD and instigates full discovery.
	uint8_t Address;		///< The low byte of the 15 bit Port-Address of the DMX Port that should action this command.
}PACKED;

struct ArtTodData {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t RdmVer;			///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
	uint8_t Port;			///< Physical Port. Range 1-4.
	uint8_t Spare1;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare2;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare3;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare4;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare5;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare6;			///< Transmit as zero, receivers don’t test.
	uint8_t BindIndex;		///< The BindIndex defines the bound node which originated this packet. In combination with Port and Source IP address, it uniquely identifies the sender.
	uint8_t Net;			///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
	uint8_t CommandResponse;///< 0x00 TodFull The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
	uint8_t Address;		///< The low 8 bits of the Port-Address of the Output Gateway DMX Port that generated this packet. The high nibble is the Sub-Net switch. The low nibble corresponds to the Universe.
	uint8_t UidTotalHi;		///< The total number of RDM devices discovered by this Universe.
	uint8_t UidTotalLo;
	uint8_t BlockCount; 	///< The index number of this packet. When UidTotal exceeds 200, multiple ArtTodData packets are used.
	uint8_t UidCount;		///< The number of UIDs encoded in this packet. This is the index of the following array.
	uint8_t Tod[200][6];	///< 48 bit An array of RDM UID.
}PACKED;

/**
 * 	The ArtRdm packet is used to transport all non-discovery RDM messages over Art-Net.
 */
struct ArtRdm {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t RdmVer;			///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
	uint8_t Filler2;		///< Pad length to match ArtPoll.
	uint8_t Spare1;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare2;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare3;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare4;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare5;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare6;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare7;			///< Transmit as zero, receivers don’t test.
	uint8_t Net;			///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
	uint8_t Command;		///< 0x00 ArProcess Process RDM Packet0x00 AtcNone No action. 0x01 AtcFlush The node flushes its TOD and instigates full discovery.
	uint8_t Address;		///< The low 8 bits of the Port-Address that should action this command.
	uint8_t RdmPacket[256];	///< The RDM data packet excluding the DMX StartCode with Checksum
}PACKED;

/**
 * The ArtRdmSub packet is used to transfer Get, Set, GetResponse and SetResponse data to and from multiple sub-devices within an RDM device.
 */
struct ArtRdmSub {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpAddress \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t RdmVer;			///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
	uint8_t Filler2;		///< Transmit as zero, receivers don’t test.
	uint8_t UID[6];			///< UID of target RDM device.
	uint8_t Spare1;			///< Transmit as zero, receivers don’t test.
	uint8_t CommandClass;	///< As per RDM specification. This field defines whether this is a Get, Set, GetResponse, SetResponse.
	uint8_t ParameterId[2];	///< As per RDM specification. This field defines the type of parameter contained in this packet. Big- endian.
	uint8_t SubDevice[2];	///< Defines the first device information contained in packet. This follows the RDM convention that 0 = root device and 1 = first subdevice. Big-endian.
	uint8_t SubCount[2];	///< The number of sub devices packed into packet. Zero is illegal. Big-endian.
	uint8_t Spare2;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare3;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare4;			///< Transmit as zero, receivers don’t test.
	uint8_t Spare5;			///< Transmit as zero, receivers don’t test.
	uint8_t Data[231];
}PACKED;

struct ArtIpProg {
	uint8_t Id[8];		///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;	///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the OpCode listing.
	uint8_t ProtVerHi;	///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;	///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;	///< Pad length to match ArtPoll.
	uint8_t Filler2;	///< Pad length to match ArtPoll.
	uint8_t Command;	///< Defines the how this packet is processed.
	uint8_t Filler;		///< Set to zero. Pads data structure for word alignment.
	uint8_t ProgIpHi;	///< IP Address to be programmed into Node if enabled by Command Field
	uint8_t ProgIp2;
	uint8_t ProgIp1;
	uint8_t ProgIpLo;
	uint8_t ProgSmHi;	///< Subnet mask to be programmed into Node if enabled by Command Field
	uint8_t ProgSm2;
	uint8_t ProgSm1;
	uint8_t ProgSmLo;
	uint8_t ProgPortHi;	///< PortAddress to be programmed into Node if enabled by Command Field
	uint8_t ProgPortLo;
	uint8_t ProgGwHi;	///< Gateway to be programmed into Node if enabled by Command Field
	uint8_t ProgGw2;
	uint8_t ProgGw1;
	uint8_t ProgGwtLo;
}PACKED;

struct ArtIpProgReply {
	uint8_t Id[8];		///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;	///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the OpCode listing.
	uint8_t ProtVerHi;	///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;	///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;	///< Pad length to match ArtPoll.
	uint8_t Filler2;	///< Pad length to match ArtPoll.
	uint8_t Filler3;	///< Pad length to match ArtIpProg.
	uint8_t Filler4;	///< Pad length to match ArtIpProg.
	uint8_t ProgIpHi;	///< IP Address of Node
	uint8_t ProgIp2;
	uint8_t ProgIp1;
	uint8_t ProgIpLo;
	uint8_t ProgSmHi;	///< Subnet mask of Node
	uint8_t ProgSm2;
	uint8_t ProgSm1;
	uint8_t ProgSmLo;
	uint8_t ProgPortHi;	///< Port Address of Node
	uint8_t ProgPortLo;
	uint8_t Status;		///< Bit 6 DHCP enabled.
	uint8_t Spare2;
	uint8_t ProgGwHi;	///< Gateway of Node
	uint8_t ProgGw2;
	uint8_t ProgGw1;
	uint8_t ProgGwtLo;
	uint8_t Spare7;
	uint8_t Spare8;
}PACKED;

struct ArtTrigger {
	uint8_t Id[8];		///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;	///< OpPollReply
	uint8_t ProtVerHi;	///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;	///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;
	uint8_t Filler2;
	uint8_t OemCodeHi;	///< The manufacturer code (high byte) of nodes that shall accept this trigger.
	uint8_t OemCodeLo;	///< The manufacturer code (low byte) of nodes that shall accept this trigger.
	uint8_t Key;		///< The Trigger Key.
	uint8_t SubKey;		///< The Trigger SubKey.
	uint8_t Data[512];	///< The interpretation of the payload is defined by the Key.
}PACKED;

struct ArtDirectory {
	uint8_t Id[8];		///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;	///< OpPollReply \ref TOpCodes
	uint8_t ProtVerHi;	///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;	///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;
	uint8_t Filler2;
	uint8_t Command;	///< Defines the purpose of the packet
	uint8_t FileHi;		///< The most significant byte of the file number requested
	uint8_t FileLo;		///< The least significant byte of the file number requested
}PACKED;

struct ArtDirectoryReply {
	uint8_t Id[8];			///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
	uint16_t OpCode;		///< OpPollReply \ref TOpCodes
	uint8_t ProtVerHi;		///< High byte of the Art-Net protocol revision number.
	uint8_t ProtVerLo;		///< Low byte of the Art-Net protocol revision number. Current value 14.
	uint8_t Filler1;
	uint8_t Filler2;
	uint8_t Flags;			///< Bit fields
	uint8_t FileHi;			///< The most significant byte of the file number requested
	uint8_t FileLo;			///< The least significant byte of the file number requested
	uint8_t Name83[16];		///< The file's name
	uint8_t Description[64];	///< Description text for the file;
	uint8_t Length[8];		///< File length in bytes
	uint8_t Data[64];		///< Application specific data
}PACKED;

union UArtPacket {
	struct ArtPoll ArtPoll;					///< ArtPoll packet
	struct ArtPollReply ArtPollReply;			///< ArtPollReply packet
	struct ArtDmx ArtDmx;						///< ArtDmx packet
	struct ArtDiagData ArtDiagData;			///< ArtDiagData packet
	struct ArtSync ArtSync;					///< ArtSync packet
	struct ArtAddress ArtAddress;				///< ArtAddress packet
	struct ArtInput ArtInput;					///< ArtInput packet
	struct ArtTimeCode ArtTimeCode;			///< ArtTimeCode packet
	struct ArtTimeSync ArtTimeSync;			///< ArtTimeSync packet
	struct ArtTodRequest ArtTodRequest;		///< ArtTodRequest packet
	struct ArtTodControl ArtTodControl;		///< ArtTodControl packet
	struct ArtTodData ArtTodData;				///< ArtTodData packet
	struct ArtRdm ArtRdm;						///< ArtRdm packet
	struct ArtIpProg ArtIpProg;				///< ArtIpProg packet
	struct ArtIpProgReply ArtIpProgReply;		///< ArtIpProgReply packet
	struct ArtTrigger ArtTrigger;				///< ArtTrigger packet
	struct ArtDirectory ArtDirectory;			///< ArtDirectory packet
	struct ArtDirectoryReply ArtDirectoryReply;///< ArtDirectoryReply packet
};

struct ArtPollQueue {
	uint32_t ArtPollMillis;
	uint32_t ArtPollReplyIpAddress;
	struct {
		uint16_t TargetPortAddressTop;
		uint16_t TargetPortAddressBottom;
	} ArtPollReply;
};
}  // namespace artnet

#endif /* ARTNET_H_ */
