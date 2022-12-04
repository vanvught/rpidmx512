/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"

#include "network.h"

#include "debug.h"

#include <time.h>
#include <sys/time.h>

static uint32_t micros(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return static_cast<uint32_t>((tv.tv_sec * 1000000) + tv.tv_usec);
}

#include "config.h"

using namespace dmxmulti;
using namespace dmx;

static int s_nHandePortDmx[MAX_PORTS];
static int s_nHandePortRdm[MAX_PORTS];
static uint8_t rdmReceiveBuffer[1500];

struct Data dmxDataRx;

static uint8_t dmxSendBuffer[513];

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx() {
	DEBUG_ENTRY
	printf("Dmx: MAX_PORTS=%u\n", MAX_PORTS);

	assert(s_pThis == nullptr);
	s_pThis = this;

	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		s_nHandePortDmx[i] = Network::Get()->Begin(UDP_PORT_DMX_START + i);
		assert(s_nHandePortDmx[i] != -1);

		s_nHandePortRdm[i] = Network::Get()->Begin(UDP_PORT_RDM_START + i);
		assert(s_nHandePortRdm[i] != -1);

		m_tDmxPortDirection[i] = PortDirection::DISABLED;
	}

	DEBUG_EXIT
}

void Dmx::SetPortDirection(uint32_t nPortIndex, PortDirection tPortDirection, bool bEnableData) {
	assert(nPortIndex < MAX_PORTS);

	if (tPortDirection != m_tDmxPortDirection[nPortIndex]) {
		StopData(0, nPortIndex);

		switch (tPortDirection) {
		case PortDirection::OUTP:
			m_tDmxPortDirection[nPortIndex] = PortDirection::OUTP;
			break;
		case PortDirection::INP:
		default:
			m_tDmxPortDirection[nPortIndex] = PortDirection::INP;
			break;
		}
	} else if (!bEnableData) {
		StopData(0, nPortIndex);
	}

	if (bEnableData) {
		StartData(0, nPortIndex);
	}
}

void Dmx::ClearData(__attribute__((unused)) uint32_t nUart) {
}

void Dmx::StartData(__attribute__((unused)) uint32_t nUart, __attribute__((unused)) uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void Dmx::StopData(__attribute__((unused)) uint32_t nUart, __attribute__((unused)) uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

// DMX Send

void Dmx::SetDmxBreakTime(__attribute__((unused)) uint32_t nBreakTime) {
}

void Dmx::SetDmxMabTime(__attribute__((unused)) uint32_t nMabTime) {
}

void Dmx::SetDmxPeriodTime(__attribute__((unused)) uint32_t nPeriod) {
}

void Dmx::SetDmxSlots(__attribute__((unused)) uint16_t nSlots) {
}

void Dmx::SetPortSendDataWithoutSC(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex < MAX_PORTS);
	assert(pData != 0);
	assert(nLength != 0);
	assert(nLength <= 512);

	dmxSendBuffer[0] = 0;
	memcpy(&dmxSendBuffer[1], pData,  nLength);

	Network::Get()->SendTo(s_nHandePortDmx[nPortIndex], dmxSendBuffer, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_DMX_START + nPortIndex);
}

void Dmx::Blackout() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void Dmx::FullOn() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

// DMX Receive

const uint8_t *Dmx::GetDmxAvailable(__attribute__((unused)) uint32_t nPortIndex)  {
	assert(nPortIndex < MAX_PORTS);

	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

//	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortDmx[nPortIndex], &dmxDataRx, sizeof(buffer::SIZE), &fromIp, &fromPort);
		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == (UDP_PORT_DMX_START + nPortIndex))) {
			nPackets++;
		}
//	} while (nBytesReceived == 0);

	if (nPackets == 0) {
		return nullptr;
	}

	dmxDataRx.Statistics.nSlotsInPacket = nBytesReceived;
	return const_cast<const uint8_t *>(dmxDataRx.Data);
}

const uint8_t* Dmx::GetDmxCurrentData(__attribute__((unused)) uint32_t nPortIndex) {
	return const_cast<const uint8_t *>(dmxDataRx.Data);
}

uint32_t Dmx::GetUpdatesPerSecond(__attribute__((unused)) uint32_t nPortIndex) {
	return 0;
}

uint32_t GetDmxReceivedCount(__attribute__((unused)) uint32_t nPortIndex) {
	return 0;
}

// RDM Send

void Dmx::RdmSendRaw(uint32_t nPortIndex, const uint8_t* pRdmData, uint32_t nLength) {
	assert(nPortIndex < MAX_PORTS);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	Network::Get()->SendTo(s_nHandePortRdm[nPortIndex], pRdmData, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_RDM_START + nPortIndex);
}

// RDM Receive

const uint8_t *Dmx::RdmReceive(uint32_t nPortIndex) {
	assert(nPortIndex < MAX_PORTS);

	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortRdm[nPortIndex], &rdmReceiveBuffer, sizeof(rdmReceiveBuffer), &fromIp, &fromPort);
		if (nBytesReceived != 0) {
			debug_dump(rdmReceiveBuffer, nBytesReceived);
		}

		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == (UDP_PORT_RDM_START + nPortIndex))) {
			nPackets++;
		}
	} while (nBytesReceived != 0);

	if (nPackets == 0) {
		return nullptr;
	}

	return rdmReceiveBuffer;
}

const uint8_t *Dmx::RdmReceiveTimeOut(uint32_t nPortIndex, uint16_t nTimeOut) {
	DEBUG_PRINTF("nTimeOut=%u", nTimeOut);
	assert(nPortIndex < MAX_PORTS);

	uint8_t *p = nullptr;
	const auto nMicros = micros();

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPortIndex))) != nullptr) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while (( micros() - nMicros) < (static_cast<uint32_t>(nTimeOut) + 50000U));

	return p;
}
