/**
 * @file networkparams.cpp
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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#if defined (__circle__)
#include <circle/logger.h>
#include <circle/stdarg.h>
#include <circle/util.h>
#define ALIGNED
#elif defined (__linux__) || defined (__CYGWIN__)
#include <string.h>
#define ALIGNED
#else
#include "util.h"
#endif

#include "networkparams.h"
#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#define SET_IS_DHCP_MASK			1<<0
#define SET_IP_ADDRESS_MASK			1<<1
#define SET_NET_MASK_MASK			1<<2
#define SET_DEFAULT_GATEWAY_MASK	1<<3
#define SET_NAME_SERVER_MASK		1<<4

static const char PARAMS_FILE_NAME[] ALIGNED = "network.txt";
static const char PARAMS_USE_DHCP[] ALIGNED = "use_dhcp";
static const char PARAMS_IP_ADDRESS[] ALIGNED = "ip_address";
static const char PARAMS_NET_MASK[] ALIGNED = "net_mask";
static const char PARAMS_DEFAULT_GATEWAY[] ALIGNED = "default_gateway";
static const char PARAMS_NAME_SERVER[] ALIGNED = "name_server";

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((NetworkParams *) p)->callbackFunction(s);
}

void NetworkParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint32_t value32;

	if (Sscan::Uint8(pLine, PARAMS_USE_DHCP, &value8) == SSCAN_OK) {
		if (value8 == 0) {
			m_isDhcpUsed = false;
			m_bSetList |= SET_IS_DHCP_MASK;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, PARAMS_IP_ADDRESS, &value32) == 1) {
		m_IpAddress = value32;
		m_bSetList |= SET_IP_ADDRESS_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NET_MASK, &value32) == 1) {
		m_NetMask = value32;
		m_bSetList |= SET_NET_MASK_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_DEFAULT_GATEWAY, &value32) == 1) {
		m_DefaultGateway = value32;
		m_bSetList |= SET_DEFAULT_GATEWAY_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NAME_SERVER, &value32) == 1) {
		m_NameServer = value32;
		m_bSetList |= SET_NAME_SERVER_MASK;
	}
}

NetworkParams::NetworkParams(void): m_bSetList(0) {
	m_isDhcpUsed = true;
	m_IpAddress = 0;
	m_NetMask = 0;
	m_DefaultGateway = 0;
	m_NameServer = 0;
}

NetworkParams::~NetworkParams(void) {
}

bool NetworkParams::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void NetworkParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("Network parameters \'%s\':\n", PARAMS_FILE_NAME);

	if (isMaskSet(SET_IS_DHCP_MASK)) {
		printf(" DHCP : [%s]\n", BOOL2STRING(m_isDhcpUsed));
	}

	if (isMaskSet(SET_IP_ADDRESS_MASK)) {
		printf(" IP Address : " IPSTR "\n", IP2STR(m_IpAddress));
	}

	if (isMaskSet(SET_NET_MASK_MASK)) {
		printf(" Netmask : " IPSTR "\n", IP2STR(m_NetMask));
	}

	if (isMaskSet(SET_DEFAULT_GATEWAY_MASK)) {
		printf(" Default Gateway : " IPSTR "\n", IP2STR(m_DefaultGateway));
	}

	if (isMaskSet(SET_NAME_SERVER_MASK)) {
		printf(" Name Server : " IPSTR "\n", IP2STR(m_NameServer));
	}
#endif
}

bool NetworkParams::isDhcpUsed(void) const {
	return m_isDhcpUsed;
}

uint32_t NetworkParams::GetIpAddress(void) const {
	return m_IpAddress;
}

uint32_t NetworkParams::GetNetMask(void) const {
	return m_NetMask;
}

uint32_t NetworkParams::GetDefaultGateway(void) const {
	return m_DefaultGateway;
}

uint32_t NetworkParams::GetNameServer(void) const {
	return m_NameServer;
}

bool NetworkParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}

