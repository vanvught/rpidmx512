/**
 * @file handlenetwork.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nextion.h"

#include "network.h"

#include "debug.h"

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void Nextion::SetIp(const char *pObjectName, uint32_t nIp) {
	char componentText[64];

	snprintf(componentText, sizeof componentText - 1, "%s.txt=\"" IPSTR "\"", pObjectName, IP2STR(nIp));
	SendCommand(componentText);
}

uint32_t Nextion::GetIp(const char *pObjectName) {
	char componentValue[32];
	uint32_t nLength = (sizeof componentValue) - 1;

	if (GetText(pObjectName, componentValue, nLength)) {
		componentValue[nLength] = '\0';
		DEBUG_PRINTF("%d:%s", nLength, componentValue);

		struct in_addr addr;
		inet_aton(reinterpret_cast<char *>(componentValue), &addr);
		return addr.s_addr;
	}

	return 0;
}

void Nextion::HandleNetworkGet(void) {
	DEBUG2_ENTRY

	SetIp("n_ip", Network::Get()->GetIp());
	SetIp("n_netmask", Network::Get()->GetNetmask());
//	SetIp("n_gw", Network::Get()->GetGatewayIp()); // There is no SetGatewayIp
	SetText("n_hostname", Network::Get()->GetHostName());
	SetValue("n_dhcp", static_cast<uint32_t>(Network::Get()->IsDhcpUsed()));

	DEBUG2_EXIT
}

void Nextion::HandleNetworkSave(void) {
	DEBUG2_ENTRY

	uint32_t nIp;

	if ((nIp = GetIp("n_ip")) != 0) {
		//Network::Get()->SetIp(nIp);
	}

	if ((nIp = GetIp("n_netmask")) != 0) {
		//Network::Get()->SetNetmask(nIp);
	}

	char aHostName[NETWORK_HOSTNAME_SIZE];
	uint32_t nLength = (sizeof aHostName) - 1;

	if (GetText("n_hostname", aHostName, nLength)) {
		aHostName[MIN(nLength, NETWORK_HOSTNAME_SIZE - 1)] = '\0';
		DEBUG_PRINTF("%d:%s", nLength, aHostName);

		Network::Get()->SetHostName(aHostName);
	}

	uint32_t nValue;

	if(GetValue("n_dhcp", nValue)) {
		if (nValue != 0) {
			Network::Get()->EnableDhcp();
		}
	}

	DEBUG2_EXIT
}
