/*
 * networkdisplay.cpp
 */

#include "network.h"


void NetworkDisplay::ShowEmacStart() {
//	DisplayUdf::Get()->ShowEmacStart();
}

void NetworkDisplay::ShowIp() {
//	DisplayUdf::Get()->ShowIpAddress();
}

void NetworkDisplay::ShowNetMask() {
//	DisplayUdf::Get()->ShowNetmask();
}

void NetworkDisplay::ShowGatewayIp() {
//	DisplayUdf::Get()->ShowGatewayIp();
}

void NetworkDisplay::ShowHostName() {
//	DisplayUdf::Get()->ShowHostName();
}

void NetworkDisplay::ShowShutdown() {
//	DisplayUdf::Get()->ShowShutdown();
}

// DHCP Client
void NetworkDisplay::ShowDhcpStatus(__attribute__((unused)) network::dhcp::ClientStatus nStatus) {
//	DisplayUdf::Get()->ShowDhcpStatus(nStatus);
}
