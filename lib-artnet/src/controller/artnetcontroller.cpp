/**
 * @file artnetcontroller.cpp
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "artnetcontroller.h"
#include "artnet.h"
#include "artnetconst.h"
#if (ARTNET_VERSION >= 4)
#include "e131.h"
#endif
#include "hal.h"
#include "hal_boardinfo.h"
#include "hal_millis.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

using namespace artnet;

static uint16_t s_active_universes[POLL_TABLE_SIZE_UNIVERSES] __attribute__((aligned(4)));

ArtNetController::ArtNetController()
{
    DEBUG_ENTRY();

    union uip
    {
        uint32_t u32;
        uint8_t u8[4];
    } ip;

    assert(s_this == nullptr);
    s_this = this;

    m_pArtNetPacket = new struct TArtNetPacket;
    assert(m_pArtNetPacket != nullptr);

    memset(&state_, 0, sizeof(struct State));
    state_.reportcode = artnet::ReportCode::kRcpowerok;
    state_.status = artnet::Status::kStandby;

    memset(&m_ArtNetPoll, 0, sizeof(struct ArtPoll));
    memcpy(&m_ArtNetPoll, artnet::kNodeId, 8);
    m_ArtNetPoll.OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpPoll);
    m_ArtNetPoll.ProtVerLo = artnet::kProtocolRevision;
    m_ArtNetPoll.Flags = Flags::kSendArtpOnChange;

    memset(&art_poll_reply_, 0, sizeof(struct ArtPollReply));
    memcpy(&art_poll_reply_, artnet::kNodeId, 8);
    art_poll_reply_.OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpPollreply);
    art_poll_reply_.Port = artnet::kUdpPort;
    art_poll_reply_.VersInfoH = ArtNetConst::kVersion[0];
    art_poll_reply_.VersInfoL = ArtNetConst::kVersion[1];
    art_poll_reply_.OemHi = ArtNetConst::kOemId[0];
    art_poll_reply_.Oem = ArtNetConst::kOemId[1];
    art_poll_reply_.EstaMan[0] = ArtNetConst::kEstaId[1];
    art_poll_reply_.EstaMan[1] = ArtNetConst::kEstaId[0];
    art_poll_reply_.Style = static_cast<uint8_t>(StyleCode::kServer);
    network::iface::CopyMacAddressTo(art_poll_reply_.MAC);
    art_poll_reply_.bind_index = 1;
    ip.u32 = network::GetPrimaryIp();
    memcpy(art_poll_reply_.IPAddress, ip.u8, sizeof(art_poll_reply_.IPAddress));
#if (ARTNET_VERSION >= 4)
    memcpy(art_poll_reply_.BindIp, ip.u8, sizeof(art_poll_reply_.BindIp));
    art_poll_reply_.AcnPriority = e131::priority::kDefault;
#endif
    /*
     * Status 1
     */
    art_poll_reply_.Status1 |= artnet::Status1::kIndicatorNormalMode | artnet::Status1::kPapNetwork;
    /*
     * Status 2
     */
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kSacnAbleToSwitch);
    art_poll_reply_.Status2 |=
        artnet::Status2::kPortAddress15Bit | (artnet::kVersion >= 4 ? artnet::Status2::kSacnAbleToSwitch : artnet::Status2::kSacnNoSwitch);
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kIpDhcp);
    art_poll_reply_.Status2 |= network::iface::Dhcp() ? artnet::Status2::kIpDhcp : artnet::Status2::kIpManualy;
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kDhcpCapable);
    art_poll_reply_.Status2 |= network::iface::IsDhcpCapable() ? artnet::Status2::kDhcpCapable : static_cast<uint8_t>(0);

#if defined(ENABLE_HTTPD) && defined(ENABLE_CONTENT)
    art_poll_reply_.Status2 |= artnet::Status2::kWebBrowserSupport;
#endif

    art_poll_reply_.PortTypes[0] = artnet::PortType::kOutputArtnet;
    art_poll_reply_.PortTypes[1] = artnet::PortType::kInputArtnet;
    art_poll_reply_.GoodOutput[0] = artnet::GoodOutput::kDataIsBeingTransmitted;
    art_poll_reply_.GoodInput[0] = artnet::GoodInput::kDataRecieved;
    art_poll_reply_.NumPortsLo = 2;

    m_pArtDmx = new struct ArtDmx;
    assert(m_pArtDmx != nullptr);

    memset(m_pArtDmx, 0, sizeof(struct ArtDmx));
    memcpy(m_pArtDmx, artnet::kNodeId, 8);
    m_pArtDmx->OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpDmx);
    m_pArtDmx->ProtVerLo = artnet::kProtocolRevision;

    m_pArtSync = new struct ArtSync;
    assert(m_pArtSync != nullptr);

    memset(m_pArtSync, 0, sizeof(struct ArtSync));
    memcpy(m_pArtSync, artnet::kNodeId, 8);
    m_pArtSync->OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpSync);
    m_pArtSync->ProtVerLo = artnet::kProtocolRevision;

    m_ArtNetController.Oem[0] = ArtNetConst::kOemId[0];
    m_ArtNetController.Oem[1] = ArtNetConst::kOemId[1];

    ActiveUniversesClear();

    SetShortName(nullptr);
    SetLongName(nullptr);

    DEBUG_EXIT();
}

ArtNetController::~ArtNetController()
{
    DEBUG_ENTRY();

    delete m_pArtNetPacket;
    m_pArtNetPacket = nullptr;

    DEBUG_EXIT();
}

void ArtNetController::GetShortNameDefault(char* short_name)
{
#if !defined(ARTNET_SHORT_NAME)
    uint8_t nBoardNameLength;
    const auto* const kBoardName = hal::BoardName(nBoardNameLength);
    snprintf(short_name, artnet::kShortNameLength - 1, "%s %s %u", kBoardName, artnet::kNodeId, static_cast<unsigned int>(artnet::kVersion));
    short_name[artnet::kShortNameLength - 1] = '\0';
#else
    uint32_t i;

    for (i = 0; i < (sizeof(ARTNET_SHORT_NAME) - 1) && i < (artnet::kShortNameLength - 1); i++)
    {
        if (ARTNET_SHORT_NAME[i] == '_')
        {
            short_name[i] = ' ';
        }
        else
        {
            short_name[i] = ARTNET_SHORT_NAME[i];
        }
    }

    short_name[i] = '\0';
#endif
}

void ArtNetController::SetShortName(const char* short_name)
{
    DEBUG_ENTRY();

    if (short_name == nullptr)
    {
        GetShortNameDefault(reinterpret_cast<char*>(art_poll_reply_.ShortName));
    }
    else
    {
        strncpy(reinterpret_cast<char*>(art_poll_reply_.ShortName), short_name, artnet::kShortNameLength - 1);
    }

    art_poll_reply_.LongName[artnet::kShortNameLength - 1] = '\0';

    DEBUG_PUTS(reinterpret_cast<char*>(art_poll_reply_.ShortName));
    DEBUG_EXIT();
}

void ArtNetController::GetLongNameDefault(char* long_name)
{
#if !defined(ARTNET_LONG_NAME)
    uint8_t nBoardNameLength;
    const auto* const kBoardName = hal::BoardName(nBoardNameLength);
    snprintf(long_name, artnet::kLongNameLength - 1, "%s %s %u %s", kBoardName, artnet::kNodeId, static_cast<unsigned int>(artnet::kVersion), hal::kWebsite);
#else
    uint32_t i;

    for (i = 0; i < (sizeof(ARTNET_LONG_NAME) - 1) && i < (artnet::kLongNameLength - 1); i++)
    {
        if (ARTNET_LONG_NAME[i] == '_')
        {
            long_name[i] = ' ';
        }
        else
        {
            long_name[i] = ARTNET_LONG_NAME[i];
        }
    }

    long_name[i] = '\0';
#endif
}

void ArtNetController::SetLongName(const char* long_name)
{
    DEBUG_ENTRY();

    if (long_name == nullptr)
    {
        GetLongNameDefault(reinterpret_cast<char*>(art_poll_reply_.LongName));
    }
    else
    {
        strncpy(reinterpret_cast<char*>(art_poll_reply_.LongName), long_name, artnet::kLongNameLength - 1);
    }

    art_poll_reply_.LongName[artnet::kLongNameLength - 1] = '\0';

    DEBUG_PUTS(reinterpret_cast<char*>(art_poll_reply_.LongName));
    DEBUG_EXIT();
}

void ArtNetController::Start()
{
    DEBUG_ENTRY();

    assert(handle_ == -1);
    handle_ = network::udp::Begin(artnet::kUdpPort, StaticCallbackFunction);
    assert(handle_ != -1);

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&m_ArtNetPoll), sizeof(struct ArtPoll), network::GetBroadcastIp(), artnet::kUdpPort);

    state_.status = artnet::Status::kOn;

    DEBUG_EXIT();
}

void ArtNetController::Stop()
{
    DEBUG_ENTRY();

    //  FIXME ArtNetController::Stop
    //
    //	network::udp::End(artnet::UDP_PORT);
    //	handle_ = -1;
    //
    //	state_.status = artnet::Status::OFF;

    DEBUG_EXIT();
}

void ArtNetController::HandleDmxOut(uint16_t nUniverse, const uint8_t* pDmxData, uint32_t nLength, uint8_t nPortIndex)
{
    DEBUG_ENTRY();

    ActiveUniversesAdd(nUniverse);

    m_pArtDmx->Physical = nPortIndex & 0xFF;
    m_pArtDmx->PortAddress = nUniverse;
    m_pArtDmx->LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
    m_pArtDmx->Length = static_cast<uint8_t>(nLength & 0xFF);

    // The sequence number is used to ensure that ArtDmx packets are used in the correct order.
    // This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
    m_pArtDmx->Sequence++;

    if (m_pArtDmx->Sequence == 0)
    {
        m_pArtDmx->Sequence = 1;
    }

#if defined(CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER)
    if (__builtin_expect((master_ == dmxnode::kDmxMaxValue), 1))
    {
#endif
        memcpy(m_pArtDmx->data, pDmxData, nLength);
#if defined(CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER)
    }
    else if (master_ == 0)
    {
        memset(m_pArtDmx->data, 0, nLength);
    }
    else
    {
        for (uint32_t i = 0; i < nLength; i++)
        {
            m_pArtDmx->data[i] = ((master_ * static_cast<uint32_t>(pDmxData[i])) / dmxnode::kDmxMaxValue) & 0xFF;
        }
    }
#endif

    uint32_t count = 0;
    auto IpAddresses = const_cast<struct artnet::PollTableUniverses*>(GetIpAddress(nUniverse));

    if (m_bUnicast && !m_bForceBroadcast)
    {
        if (IpAddresses != nullptr)
        {
            count = IpAddresses->nCount;
        }
        else
        {
            DEBUG_EXIT();
            return;
        }
    }

    // If the number of universe subscribers exceeds 40 for a given universe, the transmitting device may broadcast.

    if (m_bUnicast && (count <= 40) && !m_bForceBroadcast)
    {
        for (uint32_t index = 0; index < count; index++)
        {
            network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pArtDmx), sizeof(struct ArtDmx), IpAddresses->pIpAddresses[index], artnet::kUdpPort);
        }

        m_bDmxHandled = true;

        DEBUG_EXIT();
        return;
    }

    if (!m_bUnicast || (count > 40) || !m_bForceBroadcast)
    {
        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pArtDmx), sizeof(struct ArtDmx), network::GetBroadcastIp(), artnet::kUdpPort);

        m_bDmxHandled = true;
    }

    DEBUG_EXIT();
}

void ArtNetController::HandleSync()
{
    if (m_bSynchronization && m_bDmxHandled)
    {
        m_bDmxHandled = false;
        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pArtSync), sizeof(struct ArtSync), network::GetBroadcastIp(), artnet::kUdpPort);
    }
}

void ArtNetController::HandleBlackout()
{
    m_pArtDmx->LengthHi = (512 & 0xFF00) >> 8;
    m_pArtDmx->Length = (512 & 0xFF);

    memset(m_pArtDmx->data, 0, 512);

    for (uint32_t active_universe_index = 0; active_universe_index < m_nActiveUniverses; active_universe_index++)
    {
        m_pArtDmx->PortAddress = s_active_universes[active_universe_index];

        uint32_t count = 0;
        const auto* const kIpAddresses = GetIpAddress(s_active_universes[active_universe_index]);

        if (m_bUnicast && !m_bForceBroadcast)
        {
            if (kIpAddresses != nullptr)
            {
                count = kIpAddresses->nCount;
            }
            else
            {
                continue;
            }
        }

        if (m_bUnicast && (count <= 40) && !m_bForceBroadcast)
        {
            // The sequence number is used to ensure that ArtDmx packets are used in the correct order.
            // This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
            m_pArtDmx->Sequence++;

            if (m_pArtDmx->Sequence == 0)
            {
                m_pArtDmx->Sequence = 1;
            }

            for (uint32_t index = 0; index < count; index++)
            {
                network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pArtDmx), sizeof(struct ArtDmx), kIpAddresses->pIpAddresses[index],
                               artnet::kUdpPort);
            }

            continue;
        }

        if (!m_bUnicast || (count > 40) || !m_bForceBroadcast)
        {
            // The sequence number is used to ensure that ArtDmx packets are used in the correct order.
            // This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
            m_pArtDmx->Sequence++;

            if (m_pArtDmx->Sequence == 0)
            {
                m_pArtDmx->Sequence = 1;
            }

            network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(m_pArtDmx), sizeof(struct ArtDmx), network::GetBroadcastIp(), artnet::kUdpPort);
        }
    }

    m_bDmxHandled = true;
    HandleSync();
}

void ArtNetController::HandleTrigger()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void ArtNetController::ProcessPoll()
{
    const auto kCurrentMillis = hal::Millis();

    if (__builtin_expect((kCurrentMillis - m_nLastPollMillis > POLL_INTERVAL_MILLIS), 0))
    {
        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&m_ArtNetPoll), sizeof(struct ArtPoll), network::GetBroadcastIp(), artnet::kUdpPort);
        m_nLastPollMillis = kCurrentMillis;

#ifndef NDEBUG
        Dump();
        DumpTableUniverses();
#endif
    }

    if (m_bDoTableCleanup && (__builtin_expect((kCurrentMillis - m_nLastPollMillis > POLL_INTERVAL_MILLIS / 4), 0)))
    {
        Clean();
    }
}

void ArtNetController::HandlePoll(const uint8_t* buffer, uint32_t from_ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(from_ip));

    auto art_poll_reply = reinterpret_cast<artnet::ArtPollReply*>(const_cast<uint8_t*>(buffer));

    snprintf(reinterpret_cast<char*>(art_poll_reply->NodeReport), artnet::kReportLength, "#%04x [%u]", static_cast<int>(state_.reportcode),
             static_cast<unsigned>(state_.art.poll_reply_count++));

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&art_poll_reply), sizeof(artnet::ArtPollReply), from_ip, artnet::kUdpPort);

    DEBUG_EXIT();
}

void ArtNetController::HandlePollReply(const uint8_t* buffer, uint32_t from_ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(from_ip));

    if (from_ip != network::GetPrimaryIp())
    {
        Add(reinterpret_cast<const artnet::ArtPollReply*>(buffer));

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void ArtNetController::Input(const uint8_t* buffer, [[maybe_unused]] uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (memcmp(buffer, "Art-Net\0", 8) != 0)
    {
        return;
    }

    const auto kOpCode = static_cast<artnet::OpCodes>(((buffer[9] << 8)) + buffer[8]);

    switch (kOpCode)
    {
        case artnet::OpCodes::kOpPollreply:
            HandlePollReply(buffer, from_ip);
            break;
        case artnet::OpCodes::kOpPoll:
            HandlePoll(buffer, from_ip);
            break;
#if defined(ARTNET_HAVE_TRIGGER)
        case artnet::OpCodes::kOpTrigger:
        {
            auto* art_trigger = reinterpret_cast<artnet::ArtTrigger*>(const_cast<uint8_t*>(buffer));
            if ((art_trigger->oem_code_hi == 0xFF && art_trigger->oem_code_lo == 0xFF) ||
                (art_trigger->oem_code_hi == m_ArtNetController.Oem[0] && art_trigger->oem_code_lo == m_ArtNetController.Oem[1]))
            {
                DEBUG_PRINTF("Key=%d, SubKey=%d, Data[0]=%d", art_trigger->key, art_trigger->sub_key, art_trigger->data[0]);
                m_ArtTriggerCallbackFunctionPtr(reinterpret_cast<const struct ArtNetTrigger*>(&art_trigger->key));
            }
        }
        break;
#endif
        default:
            break;
    }
}

void ArtNetController::ActiveUniversesClear()
{
    memset(s_active_universes, 0, sizeof(s_active_universes));
    m_nActiveUniverses = 0;
}

void ArtNetController::ActiveUniversesAdd(uint16_t universe)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("nUniverse=%d", static_cast<int>(universe));

    int32_t low = 0;
    int32_t mid = 0;
    auto high = static_cast<int32_t>(m_nActiveUniverses);

    if (m_nActiveUniverses == (sizeof(s_active_universes) / sizeof(s_active_universes[0])))
    {
        assert(0);
        return;
    }

    while (low <= high)
    {
        mid = low + ((high - low) / 2);
        const uint32_t kNMidValue = s_active_universes[mid];

        if (kNMidValue < universe)
        {
            low = mid + 1;
        }
        else if (kNMidValue > universe)
        {
            high = mid - 1;
        }
        else
        {
            DEBUG_EXIT();
            return;
        }
    }

    DEBUG_PRINTF("nLow=%d, nMid=%d, nHigh=%d", low, mid, high);

    if ((high != -1) && (m_nActiveUniverses != static_cast<uint32_t>(high)))
    {
        auto p16 = reinterpret_cast<uint16_t*>(s_active_universes);

        assert(low >= 0);

        for (uint32_t i = m_nActiveUniverses; i >= static_cast<uint32_t>(low); i--)
        {
            p16[i + 1] = p16[i];
        }

        s_active_universes[low] = universe;

        DEBUG_PRINTF(">m< nUniverse=%u, nLow=%d", universe, low);
    }
    else
    {
        s_active_universes[m_nActiveUniverses] = universe;

        DEBUG_PRINTF(">a< nUniverse=%u, nMid=%d", universe, mid);
    }

    m_nActiveUniverses++;

    DEBUG_EXIT();
}

void ArtNetController::Print()
{
    puts("Art-Net Controller");
    printf(" Max Node's    : %u\n", POLL_TABLE_SIZE_ENRIES);
    printf(" Max Universes : %u\n", POLL_TABLE_SIZE_UNIVERSES);
    if (!m_bUnicast)
    {
        puts(" Unicast is disabled");
    }
    if (!m_bForceBroadcast)
    {
        puts(" Force broadcast is enabled");
    }
    if (!m_bSynchronization)
    {
        puts(" Synchronization is disabled");
    }
}
