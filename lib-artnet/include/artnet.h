/**
 * @file artnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

namespace artnet {
static constexpr char NODE_ID[] = "Art-Net";				///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
}  // namespace artnet

/**
 * Table 4 – Style Codes
 * The Style code defines the general functionality of a Controller.
 * The Style code is returned in ArtPollReply.
 */
enum TNodeStyleCode {
	ARTNET_ST_NODE = 0x00,		///< A DMX to / from Art-Net device
	ARTNET_ST_SERVER = 0x01,	///< A lighting console.
	ARTNET_ST_MEDIA = 0x02,		///< A Media Server.
	ARTNET_ST_ROUTE = 0x03,		///< A network routing device.
	ARTNET_ST_BACKUP = 0x04,	///< A backup device.
	ARTNET_ST_CONFIG = 0x05,	///< A configuration or diagnostic tool.
	ARTNET_ST_VISUAL = 0x06		///< A visualiser.
};

/**
 * Table 5 – Priority Codes
 * Diagnostics Priority codes.
 * These are used in ArtPoll and ArtDiagData.
 */
enum TPriorityCodes {
	ARTNET_DP_LOW = 0x10,		///< Low priority message.
	ARTNET_DP_MED = 0x40,		///< Medium priority message.
	ARTNET_DP_HIGH = 0x80,		///< High priority message.
	ARTNET_DP_CRITICAL = 0xE0,	///< Critical priority message.
	ARTNET_DP_VOLATILE = 0xF0	///< Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types are displayed in a list box.
};

/**
 * An enum for setting the behavior of a port.
 * Ports can either input data (DMX -> ArtNet) or
 * output (ArtNet -> DMX) data.
 */
enum TArtNetPortSettings {
	ARTNET_ENABLE_INPUT = 0x40,
	ARTNET_ENABLE_OUTPUT = 0x80
};

/**
 * An enum for referring to a particular input or output port.
 */
enum TArtNetPortDir{
	ARTNET_INPUT_PORT,
	ARTNET_OUTPUT_PORT,
	ARTNET_DISABLE_PORT
};

/**
 * Node configuration commands
 */
enum TArtnetPortCommand {
	ARTNET_PC_NONE = 0x00,			///< No action
	ARTNET_PC_CANCEL = 0x01,		///< If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
	ARTNET_PC_LED_NORMAL = 0x02,	///< The front panel indicators of the Node operate normally.
	ARTNET_PC_LED_MUTE = 0x03,		///< The front panel indicators of the Node are disabled and switched off.
	ARTNET_PC_LED_LOCATE = 0x04,	///< Rapid flashing of the Node’s front panel indicators. It is intended as an outlet locator for large installations.
	ARTNET_PC_RESET = 0x05,			///< Resets the Node’s Sip, Text, Test and data error flags.
	ARTNET_PC_MERGE_LTP_O = 0x10,	///< Set DMX Port 0 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_1 = 0x11,	///< Set DMX Port 1 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_2 = 0x12,	///< Set DMX Port 2 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_3 = 0x13,	///< Set DMX Port 3 to Merge in LTP mode.
	ARTNET_PC_MERGE_HTP_0 = 0x50,	///< Set DMX Port 0 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_1 = 0x51,	///< Set DMX Port 1 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_2 = 0x52,	///< Set DMX Port 2 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_3 = 0x53,	///< Set DMX Port 3 to Merge in HTP (default) mode.
	ARTNET_PC_ARTNET_SEL0 = 0x60,	///< Set DMX Port 0 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	ARTNET_PC_ARTNET_SEL1 = 0x61,	///< Set DMX Port 1 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	ARTNET_PC_ARTNET_SEL2 = 0x62,	///< Set DMX Port 2 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	ARTNET_PC_ARTNET_SEL3 = 0x63,	///< Set DMX Port 3 to output both DMX512 and RDM packets from the Art-Net protocol (default).
	ARTNET_PC_ACN_SEL0 = 0x70,		///< Set DMX Port 0 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	ARTNET_PC_ACN_SEL1 = 0x71,		///< Set DMX Port 1 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	ARTNET_PC_ACN_SEL2 = 0x72,		///< Set DMX Port 2 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	ARTNET_PC_ACN_SEL3 = 0x73,		///< Set DMX Port 3 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
	ARTNET_PC_CLR_0 = 0x90,			///< Clear DMX Output buffer for Port 0
	ARTNET_PC_CLR_1 = 0x91,			///< Clear DMX Output buffer for Port 1
	ARTNET_PC_CLR_2 = 0x92,			///< Clear DMX Output buffer for Port 2
	ARTNET_PC_CLR_3 = 0x93			///< Clear DMX Output buffer for Port 3
};

/**
 *
 */
struct ArtNetTalkToMe {
	static constexpr auto SEND_ARTP_ON_CHANGE = (1 << 1);	///< Bit 1 set : Send ArtPollReply whenever Node conditions change.
	static constexpr auto SEND_DIAG_MESSAGES = (1 << 2);	///< Bit 2 set : Send me diagnostics messages.
	static constexpr auto SEND_DIAG_UNICAST = (1 << 3);		///< Bit 3 : 0 = Diagnostics messages are broadcast. (if bit 2).													///< Bit 3 : 1 = Diagnostics messages are unicast. (if bit 2).
};

/**
 * ArtPollReply packet, Field 40
 */
struct ArtNetStatus2 {
	static constexpr auto WEB_BROWSER_SUPPORT = (1 << 0);	///< Bit 0, Set = Product supports web browser configuration
	static constexpr auto IP_MANUALY = (0 << 1);			///< Bit 1, Clr = Node’s IP is manually configured.
	static constexpr auto IP_DHCP = (1 << 1);				///< Bit 1, Set = Node’s IP is DHCP configured.
	static constexpr auto DHCP_NOT_CAPABLE = (0 << 2);		///< Bit 2, Clr = Node is not DHCP capable.
	static constexpr auto DHCP_CAPABLE = (1 << 2);			///< Bit 2, Set = Node is DHCP capable.
	static constexpr auto PORT_ADDRESS_8BIT = (0 << 3);		///< Bit 3, Clr = Node supports 8 bit Port-Address (Art-Net II).
	static constexpr auto PORT_ADDRESS_15BIT = (1 << 3);	///< Bit 3, Set = Node supports 15 bit Port-Address (Art-Net 3 or 4).
	static constexpr auto SACN_NO_SWITCH = (0 << 4);		///< Bit 4, Clr = Node not able to switch between Art-Net and sACN.
	static constexpr auto SACN_ABLE_TO_SWITCH = (1 << 4);	///< Bit 4, Set = Node is able to switch between Art-Net and sACN.
};

enum TGoodOutput {
	GO_DATA_IS_BEING_TRANSMITTED = (1 << 7),	///< Bit 7 Set – Data is being transmitted.
	GO_INCLUDES_DMX_TEST_PACKETS = (1 << 6),	///< Bit 6 Set – Channel includes DMX512 test packets.
	GO_INCLUDES_DMX_SIP = (1 << 5),				///< Bit 5 Set – Channel includes DMX512 SIP’s.
	GO_INCLUDES_DMX_TEXT_PACKETS = (1 << 4),	///< Bit 4 Channel includes DMX512 text packets.
	GO_OUTPUT_IS_MERGING = (1 << 3),			///< Bit 3 Set – Output is merging ArtNet data.
	GO_DMX_SHORT_DETECTED = (1 << 2),			///< Bit 2 Set – DMX output short detected on power up
	GO_MERGE_MODE_LTP = (1 << 1),				///< Bit 1 Set – Merge Mode is LTP.
	GO_OUTPUT_IS_SACN  = (1 << 0)				///< Bit 0 Set – Output is selected to transmit sACN.
};

enum TGoodInput {
	GI_DATA_RECIEVED  = (1 << 7)				///< Bit 7 Set – Data received.
};

/**
 * Merge is implemented in either LTP or HTP mode as specified by the ArtAddress packet.
 */
enum class ArtNetMerge {
	HTP,	///< Highest Takes Precedence (HTP)
	LTP		///< Latest Takes Precedence (LTP)
};

enum TPortProtocol {
	PORT_ARTNET_ARTNET,		///< Output both DMX512 and RDM packets from the Art-Net protocol (default).
	PORT_ARTNET_SACN		///< Output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
};

struct ArtNet {
	static constexpr uint8_t PROTOCOL_REVISION = 14;
	static constexpr uint16_t UDP_PORT = 0x1936;
	static constexpr uint32_t MAX_PORTS = 4;
	static constexpr uint32_t MAX_PAGES = 8;
	static constexpr uint32_t DMX_LENGTH = 512;
	static constexpr uint32_t SHORT_NAME_LENGTH = 18;
	static constexpr uint32_t LONG_NAME_LENGTH = 64;
	static constexpr uint32_t REPORT_LENGTH = 64;
	static constexpr uint32_t RDM_UID_WIDTH = 6;
	static constexpr uint32_t MAC_SIZE = 6;
	static constexpr uint32_t IP_SIZE = 4;
	static constexpr uint32_t ESTA_SIZE = 2;

	static ArtNetMerge GetMergeMode(const char *pMergeMode) {
		if (pMergeMode != nullptr) {
			if (((pMergeMode[0] | 0x20) == 'l')
					&& ((pMergeMode[1] | 0x20) == 't')
					&& ((pMergeMode[2] | 0x20) == 'p')) {
				return ArtNetMerge::LTP;
			}
		}
		return ArtNetMerge::HTP;
	}

	static const char* GetMergeMode(ArtNetMerge m, bool bToUpper = false) {
		if (bToUpper) {
			return (m == ArtNetMerge::HTP) ? "HTP" : "LTP";
		}
		return (m == ArtNetMerge::HTP) ? "htp" : "ltp";
	}

	static const char* GetMergeMode(unsigned m, bool bToUpper = false) {
		return GetMergeMode(static_cast<ArtNetMerge>(m), bToUpper);
	}

	static const char* GetProtocolMode(TPortProtocol p, bool bToUpper = false) {
		if (bToUpper) {
			return (p == PORT_ARTNET_ARTNET) ? "Art-Net" : "sACN";
		}
		return (p == PORT_ARTNET_ARTNET) ? "artnet" : "sacn";
	}

	static const char* GetProtocolMode(unsigned p, bool bToUpper = false) {
		return GetProtocolMode(static_cast<TPortProtocol>(p), bToUpper);
	}
};

#endif /* ARTNET_H_ */
