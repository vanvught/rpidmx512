/**
 * @file displayudf.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAYUDF_H_
#define DISPLAYUDF_H_

#include <cstdint>
#include <cstdarg>

#include "display.h"

#if !defined (NO_EMAC)
# include "network.h"
#endif

#if defined (NODE_ARTNET_MULTI)
# define NODE_ARTNET
#endif

#if defined (NODE_E131_MULTI)
# define NODE_E131
#endif

#if defined (NODE_NODE)
# include "node.h"
#endif

#if defined (NODE_ARTNET)
# include "artnetnode.h"
#endif

#if defined (NODE_E131)
# include "e131bridge.h"
#endif

#if defined (NODE_ARTNET) || defined (NODE_E131)
# define DISPLAYUDF_DMX_INFO
#endif

#if defined (RDM_RESPONDER)
# if defined (NODE_ARTNET)
#   include "artnetrdmresponder.h"
# else
#   include "rdmresponder.h"
# endif
#endif

namespace displayudf {
static constexpr auto LABEL_MAX_ROWS = 6U;

#if !defined(NODE_NODE)
enum class Labels {
	TITLE,
	BOARDNAME,
	IP,
	VERSION,
	NOT_USED1,
	AP,
	NOT_USED2,
	HOSTNAME,
	UNIVERSE_PORT_A,
	UNIVERSE_PORT_B,
	UNIVERSE_PORT_C,
	UNIVERSE_PORT_D,
	NETMASK,
	DMX_START_ADDRESS,
	DESTINATION_IP_PORT_A,
	DESTINATION_IP_PORT_B,
	DESTINATION_IP_PORT_C,
	DESTINATION_IP_PORT_D,
	DEFAULT_GATEWAY,
	DMX_DIRECTION,
	UNKNOWN
};
#else
# if LIGHTSET_PORTS > 8
#  define MAX_ARRAY 4
# else
#  define MAX_ARRAY LIGHTSET_PORTS
# endif
enum class Labels {
	TITLE,
	BOARDNAME,
	VERSION,
	HOSTNAME,
	IP,
	NETMASK,
	DEFAULT_GATEWAY,
	NOT_USED,
	UNIVERSE_PORT_A,
# if MAX_ARRAY >= 2
	UNIVERSE_PORT_B,
# endif
# if MAX_ARRAY >= 3
	UNIVERSE_PORT_C,
# endif
# if MAX_ARRAY == 4
	UNIVERSE_PORT_D,
# endif
# if MAX_ARRAY >= 2
	DESTINATION_IP_PORT_A,
# endif
# if MAX_ARRAY >= 2
	DESTINATION_IP_PORT_B,
# endif
# if MAX_ARRAY >= 3
	DESTINATION_IP_PORT_C,
# endif
# if MAX_ARRAY == 4
	DESTINATION_IP_PORT_D,
# endif
	UNKNOWN
};
# undef MAX_ARRAY
#endif

namespace defaults {
static constexpr auto INTENSITY = 0x7F;
}  // namespace defaults
namespace dmx {
enum class PortDir {
	INPUT, OUTPUT, DISABLE
};
}  // namespace dmx
}  // namespace displayudf

class DisplayUdf final: public Display {
public:
	DisplayUdf();

	void SetTitle(const char *format, ...);

	void Set(uint32_t nLine, displayudf::Labels tLabel);

	uint8_t GetLabel(uint32_t nIndex) const {
		if (nIndex < static_cast<uint32_t>(displayudf::Labels::UNKNOWN)) {
			return m_aLabels[nIndex];
		}

		return m_aLabels[0];
	}

	void Show();

	/**
	 * Node
	 */

#if defined (NODE_NODE)
	void Show(Node *pNode);
	void ShowNodeName();
	void ShowUniverse();
	void ShowDestinationIp();
#endif

	/**
	 * Art-Net
	 */

#if defined (NODE_ARTNET)
	void Show(ArtNetNode *pArtNetNode, uint32_t nPortIndexOffset = 0);
	void ShowUniverse(ArtNetNode *pArtNetNode);
	void ShowDestinationIp(ArtNetNode *pArtNetNode);
#endif

	/**
	 * sACN E1.31
	 */

#if defined (NODE_E131)
	void Show(E131Bridge *pE131Bridge, uint32_t nPortIndexOffset = 0);
#endif

	/**
	 * DMX
	 */

#if defined (DISPLAYUDF_DMX_INFO)
	void SetDmxInfo(displayudf::dmx::PortDir portDir, uint32_t nPorts) {
		m_dmxInfo.portDir = portDir;
		m_dmxInfo.nPorts = nPorts;
	}

	void ShowDmxInfo() {
		if ((m_dmxInfo.portDir == displayudf::dmx::PortDir::DISABLE) || (m_dmxInfo.nPorts == 0)) {
			Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DMX_DIRECTION)], "No DMX");
			return;
		}

		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DMX_DIRECTION)], "DMX %s %d", m_dmxInfo.portDir == displayudf::dmx::PortDir::INPUT ? "Input" : "Output",  m_dmxInfo.nPorts);
	}
#endif

	/**
	 * RDM Responder
	 */

#if defined (RDM_RESPONDER)
	void ShowDmxStartAddress() {
# if defined (NODE_ARTNET)
		const auto nDmxStartAddress = ArtNetRdmResponder::Get()->GetDmxStartAddress();
		const auto nDmxFootprint = ArtNetRdmResponder::Get()->GetDmxFootPrint();
# else
		const auto nDmxStartAddress = RDMResponder::Get()->GetDmxStartAddress();
		const auto nDmxFootprint = RDMResponder::Get()->GetDmxFootPrint();
# endif
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DMX_START_ADDRESS)], "DMX S:%3u F:%3u", nDmxStartAddress, nDmxFootprint);
	}
#endif

	/**
	 * Network
	 */

#if !defined (NO_EMAC)
	void ShowEmacInit() {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "Ethernet init");
	}

	void ShowEmacStart() {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "Ethernet start");
	}

	void ShowEmacStatus(const bool isLinkUp) {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "Ethernet Link %s", isLinkUp ? "UP" : "DOWN");
	}

	void ShowIpAddress() {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "" IPSTR "/%d %c", IP2STR(Network::Get()->GetIp()), Network::Get()->GetNetmaskCIDR(), Network::Get()->GetAddressingMode());
	}

	void ShowNetmask() {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::NETMASK)], "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
		ShowIpAddress();
	}

	void ShowGatewayIp() {
		ClearEndOfLine();
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DEFAULT_GATEWAY)], "G: " IPSTR "", IP2STR(Network::Get()->GetGatewayIp()));
	}

	void ShowHostName() {
		ClearEndOfLine();
		Write(m_aLabels[static_cast<uint32_t>(displayudf::Labels::HOSTNAME)], Network::Get()->GetHostName());
	}

	void ShowDhcpStatus(network::dhcp::ClientStatus nStatus) {
		switch (nStatus) {
		case network::dhcp::ClientStatus::IDLE:
			break;
		case network::dhcp::ClientStatus::RENEW:
			Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
			ClearEndOfLine();
			Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "DHCP renewing");
			break;
		case network::dhcp::ClientStatus::GOT_IP:
			Display::Get()->Status(Display7SegmentMessage::INFO_NONE);
			break;
		case network::dhcp::ClientStatus::RETRYING:
			Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
			ClearEndOfLine();
			Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::IP)], "DHCP retrying");
			break;
		case network::dhcp::ClientStatus::FAILED:
			Display::Get()->Status(Display7SegmentMessage::ERROR_DHCP);
			break;
		default:
			break;
		}
	}

	void ShowShutdown() {
		TextStatus("Network shutdown", Display7SegmentMessage::INFO_NETWORK_SHUTDOWN);
	}
#endif

	static DisplayUdf *Get() {
		return s_pThis;
	}

private:
	char m_aTitle[32];
	uint8_t m_aLabels[static_cast<uint32_t>(displayudf::Labels::UNKNOWN)];
#if defined (NODE_ARTNET) || defined (NODE_E131)	
	uint32_t m_nPortIndexOffset { 0 };
#endif
#if defined (DISPLAYUDF_DMX_INFO)
	struct DmxInfo {
		displayudf::dmx::PortDir portDir;
		uint32_t nPorts;
	};
	DmxInfo m_dmxInfo {displayudf::dmx::PortDir::DISABLE, 0};
#endif

	static DisplayUdf *s_pThis;
};

#endif /* DISPLAYUDF_H_ */
