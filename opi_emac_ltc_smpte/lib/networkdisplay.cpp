/*
 * networkdisplay.cpp
 */

#include "network.h"

#include "display.h"
#include "display7segment.h"

void NetworkDisplay::ShowEmacStart() {
	Display::Get()->ClearLine(3);
	Display::Get()->Printf(3, "Ethernet start");
}

void NetworkDisplay::ShowIp() {
	Display::Get()->ClearLine(3);
	Display::Get()->Printf(3, IPSTR "/%d %c", IP2STR(Network::Get()->GetIp()), static_cast<int>(Network::Get()->GetNetmaskCIDR()), Network::Get()->GetAddressingMode());
}

void NetworkDisplay::ShowNetMask() {
	ShowIp();
}

void NetworkDisplay::ShowGatewayIp() {
}

void NetworkDisplay::ShowHostName() {
}

void NetworkDisplay::ShowShutdown() {
}

// DHCP Client
void NetworkDisplay::ShowDhcpStatus(network::dhcp::ClientStatus nStatus) {
	switch (nStatus) {
	case network::dhcp::ClientStatus::IDLE:
		break;
	case network::dhcp::ClientStatus::RENEW:
		Display7Segment::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "DHCP renewing");
		break;
	case network::dhcp::ClientStatus::GOT_IP:
		Display7Segment::Get()->Status(Display7SegmentMessage::INFO_NONE);
		break;
	case network::dhcp::ClientStatus::RETRYING:
		Display7Segment::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "DHCP retrying");
		break;
	case network::dhcp::ClientStatus::FAILED:
		Display7Segment::Get()->Status(Display7SegmentMessage::ERROR_DHCP);
		break;
	default:
		break;
	}
}
