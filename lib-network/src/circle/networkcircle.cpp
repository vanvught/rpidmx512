/**
 * @file networkcircle.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <assert.h>

#include "circle/net/ipaddress.h"
#include "circle/net/in.h"
#include "circle/usb/macaddress.h"
#include "circle/logger.h"
#include "circle/util.h"
#include "circle/version.h"

#include "networkcircle.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

static const char FromNetwork[] = "network";

NetworkCircle::NetworkCircle(void) : m_pNet(0), m_pSocket(0) {
}

NetworkCircle::~NetworkCircle(void) {
}

void NetworkCircle::Init(CNetSubSystem *pNet) {
	assert(pNet != 0);
	assert(m_pNet == 0);

	pNet->GetNetDeviceLayer()->GetMACAddress()->CopyTo(m_aNetMacaddr);
	m_IsDhcpUsed = pNet->GetConfig()->IsDHCPUsed();

	pNet->GetConfig()->GetIPAddress()->CopyTo(ip.u8);
	m_nLocalIp = ip.u32;

	pNet->GetConfig()->GetBroadcastAddress()->CopyTo(ip.u8);
	m_nBroadcastIp = ip.u32;

	memcpy (&ip.u8, pNet->GetConfig()->GetNetMask(), 4);
	m_nNetmask = ip.u32;

	m_pNet = pNet;

	if (m_pSocket != 0 ) {
		delete m_pSocket;
		m_pSocket = 0;
	}

	CString Hostname;

	m_pNet->GetConfig ()->GetIPAddress ()->Format (&Hostname);

	strncpy(m_aHostName, (const char *)Hostname, sizeof(m_aHostName) - 1);
}

int32_t NetworkCircle::Begin(uint16_t nPort) {
	assert(m_pSocket == 0);

	if (m_pNet == 0) {
		CLogger::Get()->Write(FromNetwork, LogPanic, "CNetSubSystem is not available");
	}

	m_pSocket = new CSocket(m_pNet, IPPROTO_UDP);

	if (m_pSocket == 0) {
		CLogger::Get()->Write(FromNetwork, LogPanic, "Cannot create socket");
	}

	if (m_pSocket->Bind(nPort) < 0) {
		CLogger::Get()->Write(FromNetwork, LogPanic, "Cannot bind socket (port %u)", nPort);
	}
#ifndef CIRCLE_MAJOR_VERSION
 #error CIRCLE_MAJOR_VERSION not defined
#endif
#if CIRCLE_MAJOR_VERSION >= 27
	m_pSocket->SetOptionBroadcast(TRUE);
#endif

	return 0;
}

int32_t NetworkCircle::End(uint16_t nPort) {
	if (m_pSocket != 0 ) {
		delete m_pSocket;
		m_pSocket = 0;
	}

	return 0;
}

void NetworkCircle::MacAddressCopyTo(uint8_t* pMacAddress) {
	assert(pMacAddress != 0);

	memcpy((void *)pMacAddress, m_aNetMacaddr , NETWORK_MAC_SIZE);
}

uint16_t NetworkCircle::RecvFrom(uint32_t nHandle, uint8_t* pPacket, uint16_t nSize, uint32_t* pFromIp, uint16_t* pFromPort) {
	assert(pPacket != 0);
	assert(pFromIp != 0);
	assert(pFromPort != 0);

	CIPAddress IPAddressFrom;
	uint32_t nIpAddressFrom = 0;

	const int nBytesReceived = m_pSocket->ReceiveFrom((void *) pPacket, nSize, MSG_DONTWAIT, &IPAddressFrom, (u16 *) pFromPort);

	if (nBytesReceived < 0) 	{
		CLogger::Get()->Write(FromNetwork, LogError, "Cannot receive -> %u", nBytesReceived);
	} else if (nBytesReceived > 0) {
		nIpAddressFrom = IPAddressFrom;
	}

	*pFromIp = nIpAddressFrom;

	return nBytesReceived;
}

void NetworkCircle::SendTo(uint32_t nHandle, const uint8_t* pPacket, uint16_t nSize, uint32_t nToIp, uint16_t nRemotePort) {
	assert(pPacket != 0);

	CIPAddress DestinationIP(nToIp);

	if ((m_pSocket->SendTo((const void *) pPacket, (unsigned) nSize, MSG_DONTWAIT, DestinationIP, (u16) nRemotePort)) != nSize)	{
		CLogger::Get()->Write(FromNetwork, LogError, "Cannot send");
	}
}
