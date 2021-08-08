/**
 * @file dmxmulti.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef NDEBUG
# undef NDEBUG
#endif

#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmxmulti.h"

#include "network.h"

#include "debug.h"

#include <time.h>
#include <sys/time.h>

static uint32_t micros(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint32_t)((tv.tv_sec * 1000000) + tv.tv_usec);
}

#include "config.h"

using namespace dmxmulti;
using namespace dmx;

static int s_nHandePortDmx[MAX_PORTS];
static int s_nHandePortRdm[MAX_PORTS];
static uint8_t rdmReceiveBuffer[1500];

struct Data dmxDataRx;

static uint8_t dmxSendBuffer[513];

DmxMulti::DmxMulti() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		s_nHandePortDmx[i] = Network::Get()->Begin(UDP_PORT_DMX_START + i);
		assert(s_nHandePortDmx[i] != -1);

		s_nHandePortRdm[i] = Network::Get()->Begin(UDP_PORT_RDM_START + i);
		assert(s_nHandePortRdm[i] != -1);
	}

	DEBUG_EXIT
}

void DmxMulti::SetPortDirection(__attribute__((unused)) uint32_t nPort, __attribute__((unused)) PortDirection tPortDirection, __attribute__((unused)) bool bEnableData) {
}

void DmxMulti::ClearData(__attribute__((unused)) uint32_t nUart) {
}

void DmxMulti::StartData(__attribute__((unused)) uint32_t nUart) {
}

void DmxMulti::StopData(__attribute__((unused)) uint32_t nUart) {
}

// DMX Send

void DmxMulti::SetDmxBreakTime(__attribute__((unused)) uint32_t nBreakTime) {
}

void DmxMulti::SetDmxMabTime(__attribute__((unused)) uint32_t nMabTime) {
}

void DmxMulti::SetDmxPeriodTime(__attribute__((unused)) uint32_t nPeriod) {
}

void DmxMulti::SetPortSendDataWithoutSC(uint32_t nPort, const uint8_t *pData, uint32_t nLength) {
	assert(nPort < MAX_PORTS);
	assert(pData != 0);
	assert(nLength != 0);
	assert(nLength <= 512);

	dmxSendBuffer[0] = 0;
	memcpy(&dmxSendBuffer[1], pData,  nLength);

	Network::Get()->SendTo(s_nHandePortDmx[nPort], dmxSendBuffer, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_DMX_START + nPort);
}

// DMX Receive

const uint8_t *DmxMulti::GetDmxAvailable(__attribute__((unused)) uint32_t nPort)  {
	assert(nPort < MAX_PORTS);

	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortDmx[nPort], &dmxDataRx, sizeof(buffer::SIZE), &fromIp, &fromPort);
		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == (UDP_PORT_DMX_START + nPort))) {
			nPackets++;
		}
	} while (nBytesReceived == 0);

	if (nPackets == 0) {
		return nullptr;
	}

	dmxDataRx.nSlotsInPacket = nBytesReceived;
	return const_cast<const uint8_t *>(dmxDataRx.data);
}

uint32_t DmxMulti::GetUpdatesPerSeconde(__attribute__((unused)) uint32_t nPort) {
	return 0;
}

// RDM Send

void DmxMulti::RdmSendRaw(uint32_t nPort, const uint8_t* pRdmData, uint32_t nLength) {
	assert(nPort < MAX_PORTS);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	Network::Get()->SendTo(s_nHandePortRdm[nPort], pRdmData, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_RDM_START + nPort);
}

// RDM Receive

const uint8_t *DmxMulti::RdmReceive(uint32_t nPort) {
	assert(nPort < MAX_PORTS);

	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortRdm[nPort], &rdmReceiveBuffer, sizeof(rdmReceiveBuffer), &fromIp, &fromPort);
		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == (UDP_PORT_RDM_START + nPort))) {
			nPackets++;
		}
	} while (nBytesReceived != 0);

	if (nPackets == 0) {
		return nullptr;
	} else if  (nPackets != 1)  {
		rdmReceiveBuffer[0] = 0xFF; // Invalidate
	}

	return rdmReceiveBuffer;
}

const uint8_t *DmxMulti::RdmReceiveTimeOut(uint32_t nPort, uint16_t nTimeOut) {
	assert(nPort < MAX_PORTS);

	uint8_t *p = nullptr;
	const auto nMicros = micros();

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != nullptr) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while (( micros() - nMicros) < nTimeOut);

	return p;
}
