/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined (__circle__)
 #include <circle/util.h>
#elif defined (__linux__) || defined (__CYGWIN__)
 #include <string.h>
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
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
			m_bIsDhcpUsed = false;
			m_bSetList |= SET_IS_DHCP_MASK;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, PARAMS_IP_ADDRESS, &value32) == 1) {
		m_nLocalIp = value32;
		m_bSetList |= SET_IP_ADDRESS_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NET_MASK, &value32) == 1) {
		m_nNetmask = value32;
		m_bSetList |= SET_NET_MASK_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_DEFAULT_GATEWAY, &value32) == 1) {
		m_nGatewayIp = value32;
		m_bSetList |= SET_DEFAULT_GATEWAY_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NAME_SERVER, &value32) == 1) {
		m_nNameServerIp = value32;
		m_bSetList |= SET_NAME_SERVER_MASK;
	}
}

NetworkParams::NetworkParams(void): m_bSetList(0) {
	m_bIsDhcpUsed = true;
	m_nLocalIp = 0;
	m_nNetmask = 0;
	m_nGatewayIp = 0;
	m_nNameServerIp = 0;
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

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_IS_DHCP_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_USE_DHCP, (int) m_bIsDhcpUsed, BOOL2STRING(m_bIsDhcpUsed));
	}

	if (isMaskSet(SET_IP_ADDRESS_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_IP_ADDRESS, IP2STR(m_nLocalIp));
	}

	if (isMaskSet(SET_NET_MASK_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_NET_MASK, IP2STR(m_nNetmask));
	}

	if (isMaskSet(SET_DEFAULT_GATEWAY_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_DEFAULT_GATEWAY, IP2STR(m_nGatewayIp));
	}

	if (isMaskSet(SET_NAME_SERVER_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_NAME_SERVER, IP2STR(m_nNameServerIp));
	}
#endif
}

bool NetworkParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}

