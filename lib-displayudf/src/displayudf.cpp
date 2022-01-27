/**
 * @file displayudf.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cassert>

#include "displayudf.h"

#include "display.h"
#include "display7segment.h"

#include "lightset.h"
#include "network.h"

#include "hardware.h"
#include "firmwareversion.h"

#include "debug.h"

using namespace displayudf;

DisplayUdf *DisplayUdf::s_pThis = nullptr;

DisplayUdf::DisplayUdf() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		m_aLabels[i] = static_cast<uint8_t>(i + 1);
	}

	DEBUG_EXIT
}

void DisplayUdf::SetTitle(const char *format, ...) {
	va_list arp;
	va_start(arp, format);

	const auto i = vsnprintf(m_aTitle, sizeof(m_aTitle) / sizeof(m_aTitle[0]) - 1, format, arp);

	va_end(arp);

	m_aTitle[i] = '\0';

	DEBUG_PUTS(m_aTitle);
}

void DisplayUdf::Show() {
	uint8_t nHwTextLength;

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (m_aLabels[i] > LABEL_MAX_ROWS) {
			m_aLabels[i] = 0xFF;
		}

		DEBUG_PRINTF("m_aLabels[%d]=%d", i, m_aLabels[i]);
	}

	for (uint8_t i = 1; i < LABEL_MAX_ROWS; i++) {
		ClearLine(i);
	}

	Write(m_aLabels[static_cast<uint32_t>(Labels::TITLE)], m_aTitle);
	Write(m_aLabels[static_cast<uint32_t>(Labels::BOARDNAME)], Hardware::Get()->GetBoardName(nHwTextLength));
	Printf(m_aLabels[static_cast<uint32_t>(Labels::VERSION)], "Firmware V%.*s", firmwareversion::length::SOFTWARE_VERSION, FirmwareVersion::Get()->GetVersion()->SoftwareVersion);

	// RDM Responder
	ShowDmxStartAddress();

#if !defined (NO_EMAC)
	ShowIpAddress();
	ShowGatewayIp();
	ShowNetmask();
	ShowHostName();
#endif
}

#if !defined (NO_EMAC)
void DisplayUdf::ShowEmacStart() {
	ClearLine(m_aLabels[static_cast<uint32_t>(Labels::IP)]);
	Printf(m_aLabels[static_cast<uint32_t>(Labels::IP)], "Ethernet start");	
}

void DisplayUdf::ShowIpAddress() {
	ClearLine(m_aLabels[static_cast<uint32_t>(Labels::IP)]);
	Printf(m_aLabels[static_cast<uint32_t>(Labels::IP)], "" IPSTR "/%d %c", IP2STR(Network::Get()->GetIp()), Network::Get()->GetNetmaskCIDR(), Network::Get()->GetAddressingMode());
}

void DisplayUdf::ShowNetmask() {
	Printf(m_aLabels[static_cast<uint32_t>(Labels::NETMASK)], "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	ShowIpAddress();
}

void DisplayUdf::ShowGatewayIp() {
	ClearLine(m_aLabels[static_cast<uint32_t>(Labels::DEFAULT_GATEWAY)]);
	Printf(m_aLabels[static_cast<uint32_t>(Labels::DEFAULT_GATEWAY)], "G: " IPSTR "", IP2STR(Network::Get()->GetGatewayIp()));
}

void DisplayUdf::ShowHostName() {
	ClearLine(m_aLabels[static_cast<uint32_t>(Labels::HOSTNAME)]);
	Write(m_aLabels[static_cast<uint32_t>(Labels::HOSTNAME)], Network::Get()->GetHostName());
}

void DisplayUdf::ShowDhcpStatus(network::dhcp::ClientStatus nStatus) {
	switch (nStatus) {
	case network::dhcp::ClientStatus::IDLE:
		break;
	case network::dhcp::ClientStatus::RENEW:
		Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		ClearLine(m_aLabels[static_cast<uint32_t>(Labels::IP)]);
		Printf(m_aLabels[static_cast<uint32_t>(Labels::IP)], "DHCP renewing");
		break;
	case network::dhcp::ClientStatus::GOT_IP:
		Display::Get()->Status(Display7SegmentMessage::INFO_NONE);
		break;
	case network::dhcp::ClientStatus::RETRYING:
		Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		ClearLine(m_aLabels[static_cast<uint32_t>(Labels::IP)]);
		Printf(m_aLabels[static_cast<uint32_t>(Labels::IP)], "DHCP retrying");
		break;
	case network::dhcp::ClientStatus::FAILED:
		Display::Get()->Status(Display7SegmentMessage::ERROR_DHCP);
		break;
	default:
		break;
	}
}

void DisplayUdf::ShowShutdown() {
	TextStatus("Network shutdown", Display7SegmentMessage::INFO_NETWORK_SHUTDOWN);
}
#endif

void DisplayUdf::Set(uint8_t nLine, Labels tLabel) {
	if (!((nLine > 0) && (nLine <= LABEL_MAX_ROWS))) {
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (m_aLabels[i] == nLine) {
			m_aLabels[i] = m_aLabels[static_cast<uint32_t>(tLabel)];
			break;
		}
	}

	m_aLabels[static_cast<uint32_t>(tLabel)] = nLine;
}
