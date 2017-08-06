#if 0
#if !defined(__circle__)
#define __circle__
#endif
#endif
#if defined (__circle__)
/**
 * @file rpi_circle.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <circle/net/netsubsystem.h>
#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#include <circle/net/in.h>
#include <circle/usb/macaddress.h>
#include <circle/logger.h>
#include <circle/util.h>

#include <stdint.h>
#include <assert.h>

#include "network.h"

#ifndef HOST_NAME_MAX
	#define HOST_NAME_MAX	64
#endif

static char _hostname[HOST_NAME_MAX + 1];
static uint8_t _net_macaddr[NETWORK_MAC_SIZE];
static uint32_t _local_ip;
static uint32_t _netmask;
static uint32_t _broadcast_ip;
static bool _is_dhcp_used;

static CNetSubSystem *_pNet;
static CSocket *_pSocket = 0;

static const char FromArtNetNet[] = "artnet_net";

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

extern "C" {

void network_init(CNetSubSystem *pNet) {
	assert(pNet != 0);
	assert(_pNet == 0);

	pNet->GetNetDeviceLayer()->GetMACAddress()->CopyTo(_net_macaddr);
	_is_dhcp_used = pNet->GetConfig()->IsDHCPUsed();

	pNet->GetConfig()->GetIPAddress()->CopyTo(ip.u8);
	_local_ip = ip.u32;

	pNet->GetConfig()->GetBroadcastAddress()->CopyTo(ip.u8);
	_broadcast_ip = ip.u32;

	memcpy (&ip.u8, pNet->GetConfig()->GetNetMask(), 4);
	_netmask = ip.u32;

	_pNet = pNet;

	if (_pSocket != 0 ) {
		delete _pSocket;
		_pSocket = 0;
	}

	memset(_hostname, 0, sizeof(_hostname));
}

void network_begin(const uint16_t port) {
	assert(_pSocket == 0);

	if (_pNet == 0) {
		CLogger::Get()->Write(FromArtNetNet, LogPanic, "CNetSubSystem is not available");
	}

	_pSocket = new CSocket(_pNet, IPPROTO_UDP);

	if (_pSocket == 0) {
		CLogger::Get()->Write(FromArtNetNet, LogPanic, "Cannot create socket");
	}

	if (_pSocket->Bind(port) < 0) {
		CLogger::Get()->Write(FromArtNetNet, LogPanic, "Cannot bind socket (port %u)", port);
	}

#if CIRCLE_MAJOR_VERSION >= 27
	_pSocket->SetOptionBroadcast(TRUE);
#endif
}

const bool network_get_macaddr(/*@out@*/const uint8_t *macaddr) {
	assert(macaddr != 0);

	memcpy((void *)macaddr, _net_macaddr , NETWORK_MAC_SIZE);
	return true;
}

const uint32_t network_get_ip(void) {
	return _local_ip;
}

const uint32_t network_get_netmask(void) {
	return _netmask;
}

const uint32_t network_get_bcast(void) {
	return _broadcast_ip;
}

const char *network_get_hostname(void) {
	return _hostname;
}

bool network_is_dhcp_used(void) {
	return _is_dhcp_used;
}

uint16_t network_recvfrom(const uint8_t *packet, const uint16_t size, uint32_t *from_ip, uint16_t *from_port) {

	CIPAddress IPAddressFrom;
	uint32_t ip = 0;

	const int bytes_received = _pSocket->ReceiveFrom((void *) packet, size, MSG_DONTWAIT, &IPAddressFrom, (u16 *) from_port);

	if (bytes_received < 0) 	{
		CLogger::Get()->Write(FromArtNetNet, LogPanic, "Cannot receive -> %i", bytes_received);
	} else {
		ip = IPAddressFrom;
	}

	*from_ip = ip;

	return bytes_received;
}

void network_sendto(const uint8_t *packet, const uint16_t size, const uint32_t to_ip, const uint16_t remote_port) {
	CIPAddress DestinationIP(to_ip);

	if ((_pSocket->SendTo((const void *) packet, (unsigned) size, MSG_DONTWAIT, DestinationIP, (u16) remote_port)) != size)	{
		CLogger::Get()->Write(FromArtNetNet, LogPanic, "Cannot send");
	}
}

}
#endif
