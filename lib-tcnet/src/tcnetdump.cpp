/**
 * @file tcnetdump.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "tcnet.h"

#include "debug.h"

static uint32_t s_nTimeStampPrevious = 0;

void TCNet::DumpManagementHeader() {
	printf("ManagementHeader\n");
	printf(" %.3s V%d.%d %.8s\n", m_TTCNet.TCNetPacket.ManagementHeader.Header, m_TTCNet.TCNetPacket.ManagementHeader.ProtocolVersionMajor, m_TTCNet.TCNetPacket.ManagementHeader.ProtocolVersionMinor,m_TTCNet.TCNetPacket.ManagementHeader.NodeName);
	printf(" %s\n", m_TTCNet.TCNetPacket.ManagementHeader.NodeType == TCNET_TYPE_SLAVE ? "SLAVE" : (m_TTCNet.TCNetPacket.ManagementHeader.NodeType == TCNET_TYPE_MASTER ? "MASTER" : "AUTO"));
	printf(" %u [%u] %d\n", m_TTCNet.TCNetPacket.ManagementHeader.TimeStamp, m_TTCNet.TCNetPacket.ManagementHeader.TimeStamp - s_nTimeStampPrevious, m_TTCNet.TCNetPacket.ManagementHeader.SEQ);

	s_nTimeStampPrevious = m_TTCNet.TCNetPacket.ManagementHeader.TimeStamp;
}

void TCNet::DumpOptIn() {
	DEBUG_ENTRY

	DumpManagementHeader();

	printf(" OptIn\n");
	printf("  %d %d\n", m_TTCNet.TCNetPacket.OptIn.NodeCount, m_TTCNet.TCNetPacket.OptIn.NodeListenerPort);
	printf("  %d %d\n", m_TTCNet.TCNetPacket.OptIn.NodeCount, m_TTCNet.TCNetPacket.OptIn.Uptime);
	printf("  %.16s %.16s %d.%d.%d [%d]\n", m_TTCNet.TCNetPacket.OptIn.VendorName, m_TTCNet.TCNetPacket.OptIn.DeviceName, m_TTCNet.TCNetPacket.OptIn.DeviceMajorVersion, m_TTCNet.TCNetPacket.OptIn.DeviceMinorVersion, m_TTCNet.TCNetPacket.OptIn.DeviceBugVersion, m_TTCNet.TCNetPacket.OptIn.NodeCount);

	DEBUG_EXIT
}
