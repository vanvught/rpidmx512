/**
 * @file dmx.cpp
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
	return (uint32_t)((tv.tv_sec * 1000000) + tv.tv_usec);
}

#include "../config.h"

using namespace dmxsingle;
using namespace dmx;

static PortDirection s_nPortDirection = dmx::PortDirection::INP;

static int s_nHandePortDmx;
static int s_nHandePortRdm;

static uint8_t rdmReceiveBuffer[1500];

struct Data dmxDataRx;

static uint8_t dmxSendBuffer[513];

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx(bool DoInit) {
	DEBUG_PRINTF("m_IsInitDone=%d", DoInit);

	assert(s_pThis == nullptr);
	s_pThis = this;

	if (DoInit) {
		Init();
	}

	s_nHandePortDmx = Network::Get()->Begin(UDP_PORT_DMX_START);
	assert(s_nHandePortDmx != -1);

	s_nHandePortRdm = Network::Get()->Begin(UDP_PORT_RDM_START);
	assert(s_nHandePortRdm != -1);

	DEBUG_EXIT
}

void Dmx::Init() {
	assert(!m_IsInitDone);

	if (m_IsInitDone) {
		return;
	}

	m_IsInitDone = true;


}

void Dmx::StartData() {
}

void Dmx::StopData() {
}

void Dmx::SetPortDirection(__attribute__((unused)) uint32_t nPort, PortDirection tPortDirection, bool bEnableData) {
	assert(nPort == 0);

	if (tPortDirection != s_nPortDirection) {
		StopData();

		switch (tPortDirection) {
		case PortDirection::OUTP:
			s_nPortDirection = PortDirection::OUTP;
			break;
		case PortDirection::INP:
		default:
			s_nPortDirection = PortDirection::INP;
			break;
		}
	} else if (!bEnableData) {
		StopData();
	}

	if (bEnableData) {
		StartData();
	}
}

PortDirection Dmx::GetPortDirection() {
	return s_nPortDirection;
}

// DMX

void Dmx::SetDmxBreakTime(__attribute__((unused)) uint32_t nBreakTime) {
}

void Dmx::SetDmxMabTime(__attribute__((unused)) uint32_t nMabTime) {
}

void Dmx::SetDmxPeriodTime(__attribute__((unused)) uint32_t nPeriodTime) {
}

void Dmx::SetDmxSlots(__attribute__((unused)) uint16_t nSlots) {
}

const uint8_t* Dmx::GetDmxCurrentData() {
	return const_cast<const uint8_t *>(dmxDataRx.Data);
}

const uint8_t* Dmx::GetDmxAvailable() {
	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortDmx, &dmxDataRx, sizeof(buffer::SIZE), &fromIp, &fromPort);
		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == UDP_PORT_DMX_START)) {
			nPackets++;
		}
	} while (nBytesReceived == 0);

	if (nPackets == 0) {
		return nullptr;
	}

	dmxDataRx.Statistics.nSlotsInPacket = nBytesReceived;
	return const_cast<const uint8_t *>(dmxDataRx.Data);
}

const uint8_t* Dmx::GetDmxChanged() {
	const auto *p = GetDmxAvailable();
	return p;
}

void Dmx::SetSendDataLength(__attribute__((unused)) uint32_t nLength) {

}

void Dmx::SetSendData(__attribute__((unused)) const uint8_t *pData, __attribute__((unused)) uint32_t nLength) {

}

void Dmx::SetSendDataWithoutSC(const uint8_t *pData, uint32_t nLength) {
	assert(pData != 0);
	assert(nLength != 0);
	assert(nLength <= 512);

	dmxSendBuffer[0] = 0;
	memcpy(&dmxSendBuffer[1], pData,  nLength);

	Network::Get()->SendTo(s_nHandePortDmx, dmxSendBuffer, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_DMX_START);
}

uint32_t Dmx::GetUpdatesPerSecond() {
	return 0;
}

void Dmx::ClearData() {
}

// RDM

uint32_t Dmx::RdmGetDateReceivedEnd() {
	return 0;
}

const uint8_t *Dmx::RdmReceive(uint32_t nPort) {
	assert(nPort < MAX_PORTS);

	uint32_t fromIp;
	uint16_t fromPort;

	uint32_t nPackets = 0;
	uint16_t nBytesReceived;

	do {
		nBytesReceived = Network::Get()->RecvFrom(s_nHandePortRdm, &rdmReceiveBuffer, sizeof(rdmReceiveBuffer), &fromIp, &fromPort);
		if ((nBytesReceived != 0) && (fromIp != Network::Get()->GetIp()) && (fromPort == UDP_PORT_RDM_START)) {
			if (rdmReceiveBuffer[0] == 0xCC) {
				return rdmReceiveBuffer;
			}
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

const uint8_t* Dmx::RdmReceiveTimeOut(__attribute__((unused)) uint32_t nPort, uint16_t nTimeOut) {
	uint8_t *p = nullptr;
	const auto nMicros = micros();

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != nullptr) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while (( micros() - nMicros) < nTimeOut);

	return p;
}

void Dmx::RdmSendRaw(__attribute__((unused)) uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength) {
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	Network::Get()->SendTo(s_nHandePortRdm, pRdmData, nLength, Network::Get()->GetBroadcastIp(), UDP_PORT_RDM_START);
}
