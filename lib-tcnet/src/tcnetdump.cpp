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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "tcnet.h"

#include "debug.h"

static uint32_t s_nTimeStampPrevious = 0;

void TCNet::DumpManagementHeader() {
	const auto *pManagementHeader = reinterpret_cast<struct TTCNetPacketManagementHeader *>(m_pReceiveBuffer);

	printf("ManagementHeader\n");
	printf(" %.3s V%d.%d %.8s\n", pManagementHeader->Header, pManagementHeader->ProtocolVersionMajor, pManagementHeader->ProtocolVersionMinor, pManagementHeader->NodeName);
	printf(" %s\n", pManagementHeader->NodeType == TCNET_TYPE_SLAVE ? "SLAVE" : (pManagementHeader->NodeType == TCNET_TYPE_MASTER ? "MASTER" : "AUTO"));
	printf(" %u [%u] %d\n", pManagementHeader->TimeStamp, pManagementHeader->TimeStamp - s_nTimeStampPrevious, pManagementHeader->SEQ);

	s_nTimeStampPrevious = pManagementHeader->TimeStamp;
}

void TCNet::DumpOptIn() {
	DEBUG_ENTRY

	DumpManagementHeader();

	const auto *pPacketOptIn = reinterpret_cast<struct TTCNetPacketOptIn *>(m_pReceiveBuffer);

	printf(" OptIn\n");
	printf("  %d %d\n", pPacketOptIn->NodeCount, pPacketOptIn->NodeListenerPort);
	printf("  %d %d\n", pPacketOptIn->NodeCount, pPacketOptIn->Uptime);
	printf("  %.16s %.16s %d.%d.%d [%d]\n", pPacketOptIn->VendorName, pPacketOptIn->DeviceName, pPacketOptIn->DeviceMajorVersion, pPacketOptIn->DeviceMinorVersion, pPacketOptIn->DeviceBugVersion, pPacketOptIn->NodeCount);

	DEBUG_EXIT
}
