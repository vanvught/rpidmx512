/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#undef NDEBUG

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <cassert>

#include "dmx.h"
#include "rdm.h"
#include "network.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

static uint32_t Micros()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint32_t>((tv.tv_sec * 1000000) + tv.tv_usec);
}

#include "config.h"

using namespace dmx;

static int s_nHandePortDmx[dmx::config::max::PORTS];
static int s_nHandePortRdm[dmx::config::max::PORTS];
static uint8_t rdmReceiveBuffer[1500];

static dmx::TotalStatistics sv_TotalStatistics[dmx::config::max::PORTS];

struct Data dmxDataRx;

static uint8_t dmxSendBuffer[513];

// RDM

volatile uint32_t gsv_RdmDataReceiveEnd;

Dmx* Dmx::s_this = nullptr;

Dmx::Dmx()
{
    DEBUG_ENTRY();
    printf("Dmx: dmx::config::max::PORTS=%u\n", dmx::config::max::PORTS);

    assert(s_this == nullptr);
    s_this = this;

    for (uint32_t i = 0; i < dmx::config::max::PORTS; i++)
    {
        s_nHandePortDmx[i] = network::udp::Begin(UDP_PORT_DMX_START + i, nullptr);
        assert(s_nHandePortDmx[i] != -1);

        s_nHandePortRdm[i] = network::udp::Begin(UDP_PORT_RDM_START + i, nullptr);
        assert(s_nHandePortRdm[i] != -1);

        SetPortDirection(i, PortDirection::kInput, false);
    }

    DEBUG_EXIT();
}

void Dmx::SetPortDirection(uint32_t port_index, PortDirection tPortDirection, bool bEnableData)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, tPortDirection=%u", port_index, static_cast<uint32_t>(tPortDirection));
    assert(port_index < dmx::config::max::PORTS);

    if (tPortDirection != m_tDmxPortDirection[port_index])
    {
        StopData(0, port_index);

        switch (tPortDirection)
        {
            case PortDirection::kOutput:
                m_tDmxPortDirection[port_index] = PortDirection::kOutput;
                break;
            case PortDirection::kInput:
            default:
                m_tDmxPortDirection[port_index] = PortDirection::kInput;
                break;
        }
    }
    else if (!bEnableData)
    {
        StopData(0, port_index);
    }

    if (bEnableData)
    {
        StartData(0, port_index);
    }

    DEBUG_EXIT();
}

void Dmx::ClearData([[maybe_unused]] uint32_t uart) {}

volatile dmx::TotalStatistics& Dmx::GetTotalStatistics(uint32_t port_index)
{
    return sv_TotalStatistics[port_index];
}

void Dmx::StartData([[maybe_unused]] uint32_t uart, [[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void Dmx::StopData([[maybe_unused]] uint32_t uart, [[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

// DMX Send

void Dmx::SetDmxBreakTime([[maybe_unused]] uint32_t break_time) {}

void Dmx::SetDmxMabTime([[maybe_unused]] uint32_t mab_time) {}

void Dmx::SetDmxPeriodTime([[maybe_unused]] uint32_t nPeriod) {}

void Dmx::SetDmxSlots([[maybe_unused]] uint16_t nSlots) {}

void Dmx::SetSendDataWithoutSC(uint32_t port_index, const uint8_t* pData, uint32_t nLength, [[maybe_unused]] const dmx::SendStyle dmxSendStyle)
{
    assert(port_index < dmx::config::max::PORTS);
    assert(pData != nullptr);
    assert(nLength != 0);
    assert(nLength <= 512);

    dmxSendBuffer[0] = 0;
    memcpy(&dmxSendBuffer[1], pData, nLength);

    network::udp::Send(s_nHandePortDmx[port_index], dmxSendBuffer, nLength, network::GetBroadcastIp(), UDP_PORT_DMX_START + port_index);
}

void Dmx::Blackout()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void Dmx::FullOn()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

// DMX Receive

const uint8_t* Dmx::GetDmxAvailable([[maybe_unused]] uint32_t port_index)
{
    assert(port_index < dmx::config::max::PORTS);

    uint32_t fromIp;
    uint16_t fromPort;

    uint32_t nPackets = 0;
    uint16_t nBytesReceived;

    //	do {
    nBytesReceived = network::udp::Recv(s_nHandePortDmx[port_index], const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(&dmxDataRx)), &fromIp, &fromPort);
    if ((nBytesReceived != 0) && (fromIp != network::GetPrimaryIp()) && (fromPort == (UDP_PORT_DMX_START + port_index)))
    {
        nPackets++;
    }
    //	} while (nBytesReceived == 0);

    if (nPackets == 0)
    {
        return nullptr;
    }

    dmxDataRx.Statistics.nSlotsInPacket = nBytesReceived;
    return const_cast<const uint8_t*>(dmxDataRx.Data);
}

const uint8_t* Dmx::GetDmxChanged(uint32_t port_index)
{
    const auto* p = GetDmxAvailable(port_index);
    // This function is not implemented
    return p;
}

const uint8_t* Dmx::GetDmxCurrentData([[maybe_unused]] uint32_t port_index)
{
    return const_cast<const uint8_t*>(dmxDataRx.Data);
}

uint32_t Dmx::GetDmxUpdatesPerSecond([[maybe_unused]] uint32_t port_index)
{
    return 0;
}

uint32_t GetDmxReceivedCount([[maybe_unused]] uint32_t port_index)
{
    return 0;
}

// RDM Send

void Dmx::RdmSendRaw(uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength)
{
    assert(port_index < dmx::config::max::PORTS);
    assert(pRdmData != nullptr);
    assert(nLength != 0);

    network::udp::Send(s_nHandePortRdm[port_index], pRdmData, nLength, network::GetBroadcastIp(), UDP_PORT_RDM_START + port_index);

    sv_TotalStatistics[port_index].rdm.sent.classes++;
}

void Dmx::RdmSendDiscoveryRespondMessage(uint32_t port_index, const uint8_t* pRdmData, uint32_t nLength)
{
    DEBUG_ENTRY();

    assert(port_index < dmx::config::max::PORTS);
    assert(pRdmData != nullptr);
    assert(nLength != 0);

    SetPortDirection(port_index, dmx::PortDirection::kOutput, false);

    RdmSendRaw(port_index, pRdmData, nLength);

    udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

    SetPortDirection(port_index, dmx::PortDirection::kInput, true);

    sv_TotalStatistics[port_index].rdm.sent.discovery_response++;

    DEBUG_EXIT();
}

// RDM Receive

const uint8_t* Dmx::RdmReceive(uint32_t port_index)
{
    assert(port_index < dmx::config::max::PORTS);

    uint32_t fromIp;
    uint16_t fromPort;

    uint32_t nPackets = 0;
    uint16_t nBytesReceived;

    const auto micros = Micros();

    do
    {
        nBytesReceived =
            network::udp::Recv(s_nHandePortRdm[port_index], const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(&rdmReceiveBuffer)), &fromIp, &fromPort);
        if (nBytesReceived != 0)
        {
            debug::Dump(rdmReceiveBuffer, nBytesReceived);
        }

        if ((nBytesReceived != 0) && (fromIp != network::GetPrimaryIp()) && (fromPort == (UDP_PORT_RDM_START + port_index)))
        {
            nPackets++;
        }
    } while ((Micros() - micros) < 1000);

    if (nPackets == 0)
    {
        return nullptr;
    }

    if (nPackets != 1)
    {
        printf("RDM => collision:%u\n", nPackets);
        rdmReceiveBuffer[0] = 0;
    }

    return rdmReceiveBuffer;
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t port_index, uint16_t nTimeOut)
{
    DEBUG_PRINTF("nTimeOut=%u", nTimeOut);
    assert(port_index < dmx::config::max::PORTS);

    uint8_t* p = nullptr;
    const auto micros = Micros();

    do
    {
        if ((p = const_cast<uint8_t*>(RdmReceive(port_index))) != nullptr)
        {
            return reinterpret_cast<const uint8_t*>(p);
        }
    } while ((Micros() - micros) < (static_cast<uint32_t>(nTimeOut) + 100000U));

    return p;
}

void Dmx::StartOutput([[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void Dmx::SetOutput([[maybe_unused]] const bool doForce)
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}
