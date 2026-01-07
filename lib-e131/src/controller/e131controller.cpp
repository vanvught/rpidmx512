/**
 * @file e131controller.cpp
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cassert>

#include "e131controller.h"
#include "e131.h"
#include "e117.h"
#include "hal_uuid.h"
#include "hal_boardinfo.h"
#include "network.h"
#include "softwaretimers.h"
#include "firmware/debug/debug_debug.h"

using namespace e131;

static constexpr uint8_t kDeviceSoftwareVersion[] = {1, 0};

struct TSequenceNumbers
{
    uint16_t universe;
    uint8_t sequence_number;
    uint32_t ip_address;
};

static struct TSequenceNumbers s_SequenceNumbers[512] __attribute__((aligned(8)));

E131Controller::E131Controller()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    memset(&state_, 0, sizeof(struct TE131ControllerState));
    state_.priority = 100;

    char aSourceName[e131::kSourceNameLength];
    uint8_t nLength;
    snprintf(aSourceName, e131::kSourceNameLength, "%.48s %s", network::iface::HostName(), hal::BoardName(nLength));
    SetSourceName(aSourceName);

    hal::UuidCopy(cid_);

    for (uint32_t nIndex = 0; nIndex < sizeof(s_SequenceNumbers) / sizeof(s_SequenceNumbers[0]); nIndex++)
    {
        memset(&s_SequenceNumbers[nIndex], 0, sizeof(s_SequenceNumbers[0]));
    }

    SetSynchronizationAddress();

    const auto kIpMulticast = network::ConvertToUint(239, 255, 0, 0);
    m_DiscoveryIpAddress = kIpMulticast | ((universe::kDiscovery & static_cast<uint32_t>(0xFF)) << 24) | ((universe::kDiscovery & 0xFF00) << 8);

    // e131::DataPacket
    m_pE131DataPacket = new struct e131::DataPacket;
    assert(m_pE131DataPacket != nullptr);

    // e131::DiscoveryPacket
    m_pE131DiscoveryPacket = new struct e131::DiscoveryPacket;
    assert(m_pE131DiscoveryPacket != nullptr);

    // e131::SynchronizationPacket
    m_pE131SynchronizationPacket = new struct e131::SynchronizationPacket;
    assert(m_pE131SynchronizationPacket != nullptr);

    handle_ = network::udp::Begin(e131::kUdpPort, nullptr);
    assert(handle_ != -1);

    DEBUG_EXIT();
}

E131Controller::~E131Controller()
{
    DEBUG_ENTRY();

    network::udp::End(e131::kUdpPort);

    if (m_pE131SynchronizationPacket != nullptr)
    {
        delete m_pE131SynchronizationPacket;
        m_pE131SynchronizationPacket = nullptr;
    }

    if (m_pE131DiscoveryPacket != nullptr)
    {
        delete m_pE131DiscoveryPacket;
        m_pE131DiscoveryPacket = nullptr;
    }

    if (m_pE131DataPacket != nullptr)
    {
        delete m_pE131DataPacket;
        m_pE131DataPacket = nullptr;
    }

    DEBUG_EXIT();
}

void E131Controller::Start()
{
    DEBUG_ENTRY();

    FillDataPacket();
    FillDiscoveryPacket();
    FillSynchronizationPacket();

    timer_handle_send_discovery_packet_ = SoftwareTimerAdd(e131::kUniverseDiscoveryIntervalSeconds * 1000U, StaticCallbackFunctionSendDiscoveryPacket);
    assert(timer_handle_send_discovery_packet_ >= 0);

    DEBUG_EXIT();
}

void E131Controller::Stop()
{
    SoftwareTimerDelete(timer_handle_send_discovery_packet_);
}

void E131Controller::FillDataPacket()
{
    // Root Layer (See Section 5)
    m_pE131DataPacket->root_layer.pre_amble_size = __builtin_bswap16(0x0010);
    m_pE131DataPacket->root_layer.post_amble_size = __builtin_bswap16(0x0000);
    memcpy(m_pE131DataPacket->root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
    m_pE131DataPacket->root_layer.vector = __builtin_bswap32(vector::root::kData);
    memcpy(m_pE131DataPacket->root_layer.cid, cid_, e117::kCidLength);

    // E1.31 Framing Layer (See Section 6)
    m_pE131DataPacket->frame_layer.vector = __builtin_bswap32(vector::data::kPacket);
    memcpy(m_pE131DataPacket->frame_layer.source_name, source_name_, e131::kSourceNameLength);
    m_pE131DataPacket->frame_layer.priority = state_.priority;
    m_pE131DataPacket->frame_layer.synchronization_address = __builtin_bswap16(state_.SynchronizationPacket.nUniverseNumber);
    m_pE131DataPacket->frame_layer.options = 0;

    // Data Layer
    m_pE131DataPacket->dmp_layer.vector = e131::vector::dmp::kSetProperty;
    m_pE131DataPacket->dmp_layer.type = 0xa1;
    m_pE131DataPacket->dmp_layer.first_address_property = __builtin_bswap16(0x0000);
    m_pE131DataPacket->dmp_layer.address_increment = __builtin_bswap16(0x0001);
    m_pE131DataPacket->dmp_layer.property_values[0] = 0;
}

void E131Controller::FillDiscoveryPacket()
{
    memset(m_pE131DiscoveryPacket, 0, sizeof(struct e131::DiscoveryPacket));

    // Root Layer (See Section 5)
    m_pE131DiscoveryPacket->root_layer.pre_amble_size = __builtin_bswap16(0x10);
    memcpy(m_pE131DiscoveryPacket->root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
    m_pE131DiscoveryPacket->root_layer.vector = __builtin_bswap32(vector::root::kExtended);
    memcpy(m_pE131DiscoveryPacket->root_layer.cid, cid_, e117::kCidLength);

    // E1.31 Framing Layer (See Section 6)
    m_pE131DiscoveryPacket->frame_layer.vector = __builtin_bswap32(vector::extended::kDiscovery);
    memcpy(m_pE131DiscoveryPacket->frame_layer.source_name, source_name_, e131::kSourceNameLength);

    // Universe Discovery Layer (See Section 8)
    m_pE131DiscoveryPacket->universe_discovery_layer.vector = __builtin_bswap32(vector::universe::kDiscoveryUniverseList);
}

void E131Controller::FillSynchronizationPacket()
{
    memset(m_pE131SynchronizationPacket, 0, sizeof(struct e131::SynchronizationPacket));

    // Root Layer (See Section 4.2)
    m_pE131SynchronizationPacket->root_layer.pre_amble_size = __builtin_bswap16(0x10);
    memcpy(m_pE131SynchronizationPacket->root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
    m_pE131SynchronizationPacket->root_layer.flags_length = __builtin_bswap16((0x07 << 12) | (e131::kSynchronizationRootLayerSize));
    m_pE131SynchronizationPacket->root_layer.vector = __builtin_bswap32(vector::root::kExtended);
    memcpy(m_pE131SynchronizationPacket->root_layer.cid, cid_, e117::kCidLength);

    // E1.31 Framing Layer (See Section 6)
    m_pE131SynchronizationPacket->frame_layer.flags_length = __builtin_bswap16((0x07 << 12) | (e131::kSynchronizationFrameLayerSize));
    m_pE131SynchronizationPacket->frame_layer.vector = __builtin_bswap32(vector::extended::kSynchronization);
    m_pE131SynchronizationPacket->frame_layer.universe_number = __builtin_bswap16(state_.SynchronizationPacket.nUniverseNumber);
}

void E131Controller::HandleDmxOut(uint16_t nUniverse, const uint8_t* pDmxData, uint32_t nLength)
{
    uint32_t ip;

    // Root Layer (See Section 5)
    m_pE131DataPacket->root_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataRootLayerLength(1U + nLength))));

    // E1.31 Framing Layer (See Section 6)
    m_pE131DataPacket->frame_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataFrameLayerLength(1U + nLength))));
    m_pE131DataPacket->frame_layer.sequence_number = GetSequenceNumber(nUniverse, ip);
    m_pE131DataPacket->frame_layer.universe = __builtin_bswap16(nUniverse);

    // Data Layer
    m_pE131DataPacket->dmp_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataLayerLength(1U + nLength))));

    if (__builtin_expect((master_ == dmxnode::kDmxMaxValue), 1))
    {
        memcpy(&m_pE131DataPacket->dmp_layer.property_values[1], pDmxData, nLength);
    }
    else if (master_ == 0)
    {
        memset(&m_pE131DataPacket->dmp_layer.property_values[1], 0, nLength);
    }
    else
    {
        for (uint32_t i = 0; i < nLength; i++)
        {
            m_pE131DataPacket->dmp_layer.property_values[1 + i] = static_cast<uint8_t>((master_ * pDmxData[i]) / dmxnode::kDmxMaxValue);
        }
    }

    m_pE131DataPacket->dmp_layer.property_value_count = __builtin_bswap16(static_cast<uint16_t>(1 + nLength));

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pE131DataPacket), static_cast<uint16_t>(e131::DataPacketSize(1U + nLength)), ip, e131::kUdpPort);
}

void E131Controller::HandleSync()
{
    if (state_.SynchronizationPacket.nUniverseNumber != 0)
    {
        m_pE131SynchronizationPacket->frame_layer.sequence_number = state_.SynchronizationPacket.sequence_number++;
        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pE131SynchronizationPacket), e131::kSynchronizationPacketSize,
                       state_.SynchronizationPacket.nIpAddress, e131::kUdpPort);
    }
}

void E131Controller::HandleBlackout()
{
    // Root Layer (See Section 5)
    m_pE131DataPacket->root_layer.flags_length = __builtin_bswap16((0x07 << 12) | (e131::DataRootLayerLength(513)));

    // E1.31 Framing Layer (See Section 6)
    m_pE131DataPacket->frame_layer.flags_length = __builtin_bswap16((0x07 << 12) | (e131::DataFrameLayerLength(513)));

    // Data Layer
    m_pE131DataPacket->dmp_layer.flags_length = __builtin_bswap16((0x07 << 12) | (e131::DataLayerLength(513)));
    m_pE131DataPacket->dmp_layer.property_value_count = __builtin_bswap16(513);
    memset(&m_pE131DataPacket->dmp_layer.property_values[1], 0, 512);

    for (uint32_t nIndex = 0; nIndex < state_.nActiveUniverses; nIndex++)
    {
        uint32_t ip;
        uint16_t nUniverse = s_SequenceNumbers[nIndex].universe;

        m_pE131DataPacket->frame_layer.sequence_number = GetSequenceNumber(nUniverse, ip);
        m_pE131DataPacket->frame_layer.universe = __builtin_bswap16(nUniverse);

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pE131DataPacket), e131::DataPacketSize(513), ip, e131::kUdpPort);
    }

    if (state_.SynchronizationPacket.nUniverseNumber != 0)
    {
        HandleSync();
    }
}

const uint8_t* E131Controller::GetSoftwareVersion()
{
    return kDeviceSoftwareVersion;
}

void E131Controller::SetSourceName(const char* pSourceName)
{
    assert(pSourceName != nullptr);
    strncpy(source_name_, pSourceName, e131::kSourceNameLength - 1);
    source_name_[e131::kSourceNameLength - 1] = '\0';
}

void E131Controller::SetPriority(uint8_t priority)
{ // TODO (a) SetPriority
    state_.priority = priority;
}

void E131Controller::SendDiscoveryPacket()
{
    assert(m_DiscoveryIpAddress != 0);

    m_pE131DiscoveryPacket->root_layer.flags_length =
        __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DiscoveryRootLayerLength(state_.nActiveUniverses))));
    m_pE131DiscoveryPacket->frame_layer.flags_length =
        __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DiscoveryFrameLayerLength(state_.nActiveUniverses))));
    m_pE131DiscoveryPacket->universe_discovery_layer.flags_length =
        __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | e131::DiscoveryLayerLength(state_.nActiveUniverses)));

    for (uint32_t i = 0; i < state_.nActiveUniverses; i++)
    {
        m_pE131DiscoveryPacket->universe_discovery_layer.list_of_universes[i] = __builtin_bswap16(s_SequenceNumbers[i].universe);
    }

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pE131DiscoveryPacket), static_cast<uint16_t>(e131::DiscoveryPacketSize(state_.nActiveUniverses)),
                   m_DiscoveryIpAddress, e131::kUdpPort);

    DEBUG_PUTS("Discovery sent");
}

uint8_t E131Controller::GetSequenceNumber(uint16_t nUniverse, uint32_t& nMulticastIpAddress)
{
    assert(sizeof(struct TSequenceNumbers) == sizeof(uint64_t));

    int32_t nLow = 0;
    int32_t nMid = 0;
    int32_t nHigh = state_.nActiveUniverses;

    while (nLow <= nHigh)
    {
        nMid = nLow + ((nHigh - nLow) / 2);

        const uint32_t nMidValue = s_SequenceNumbers[nMid].universe;

        if (nMidValue < nUniverse)
        {
            nLow = nMid + 1;
        }
        else if (nMidValue > nUniverse)
        {
            nHigh = nMid - 1;
        }
        else
        {
            DEBUG_PRINTF("Found nUniverse=%u", nUniverse);
            nMulticastIpAddress = s_SequenceNumbers[nMid].ip_address;
            s_SequenceNumbers[nMid].sequence_number++;
            return s_SequenceNumbers[nMid].sequence_number;
        }
    }

    DEBUG_PRINTF("nActiveUniverses=%u -> %u : nLow=%d, nMid=%d, nHigh=%d", state_.nActiveUniverses, nUniverse, nLow, nMid, nHigh);

    if ((nHigh != -1) && (state_.nActiveUniverses != static_cast<uint32_t>(nHigh)))
    {
        auto p64 = reinterpret_cast<uint64_t*>(s_SequenceNumbers);

        for (int32_t i = state_.nActiveUniverses - 1; i >= nLow; i--)
        {
            p64[i + 1] = p64[i];
        }

        s_SequenceNumbers[nLow].ip_address = UniverseToMulticastIp(nUniverse);
        s_SequenceNumbers[nLow].universe = nUniverse;
        s_SequenceNumbers[nLow].sequence_number = 0;

        nMulticastIpAddress = s_SequenceNumbers[nLow].ip_address;

        DEBUG_PRINTF(">m< nUniverse=%u, nLow=%d", nUniverse, nLow);
    }
    else
    {
        s_SequenceNumbers[nMid].ip_address = UniverseToMulticastIp(nUniverse);
        s_SequenceNumbers[nMid].universe = nUniverse;

        nMulticastIpAddress = s_SequenceNumbers[nMid].ip_address;

        DEBUG_PRINTF(">a< nUniverse=%u, nMid=%d", nUniverse, nMid);
    }

    state_.nActiveUniverses++;

    return 0;
}

void E131Controller::Print()
{
    puts("sACN E1.31 Controller");
    printf(" Max Universes : %d\n", static_cast<int>(sizeof(s_SequenceNumbers) / sizeof(s_SequenceNumbers[0])));
    if (state_.SynchronizationPacket.nUniverseNumber != 0)
    {
        printf(" Synchronization Universe : %u\n", state_.SynchronizationPacket.nUniverseNumber);
    }
    else
    {
        puts(" Synchronization is disabled");
    }
}
