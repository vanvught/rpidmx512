/**
 * @file network.c
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdbool.h>

#include "network.h"

Network *Network::s_pThis = 0;

Network::Network(void) :
	m_nLocalIp(0),
	m_nGatewayIp(0),
	m_nNetmask(0),
	m_nBroadcastIp(0),
	m_IsDhcpCapable(true),
	m_IsDhcpUsed(false),
	m_nIfIndex(1)
{
	s_pThis = this;

	m_aNetMacaddr[0] = '\0';
	m_aHostName[0] = '\0';
	m_aIfName[0] = '\0';
}

Network::~Network(void) {
	m_nLocalIp = 0;
	m_nGatewayIp = 0;
	m_nNetmask = 0;
	m_nBroadcastIp = 0;
	m_IsDhcpUsed = false;
	m_aHostName[0] = '\0';
	m_aIfName[0] = '\0';

	s_pThis = 0;
}
