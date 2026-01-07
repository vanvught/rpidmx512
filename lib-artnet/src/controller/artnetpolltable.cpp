/**
 * @file artnetpolltable.cpp
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

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 9
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast" // FIXME GCC 8.0.3 Raspbian GNU/Linux 10 (buster)
#endif
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "artnetpolltable.h"
#include "hal.h"
#include "hal_millis.h"
#include "ip4/ip4_address.h"
 #include "firmware/debug/debug_debug.h"

union uip
{
    uint32_t u32;
    uint8_t u8[4];
} static ip;

ArtNetPollTable::ArtNetPollTable()
{
    DEBUG_ENTRY();

    table_ = new artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES];
    assert(table_ != nullptr);

    memset(table_, 0, sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES]));

    table_universes_ = new artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES];
    assert(table_universes_ != nullptr);

    memset(table_universes_, 0, sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES]));

    for (uint32_t nIndex = 0; nIndex < artnet::POLL_TABLE_SIZE_UNIVERSES; nIndex++)
    {
        table_universes_[nIndex].pIpAddresses = new uint32_t[artnet::POLL_TABLE_SIZE_ENRIES];
        assert(table_universes_[nIndex].pIpAddresses != nullptr);
    }

    table_clean_.nTableIndex = 0;
    table_clean_.universe_index = 0;
    table_clean_.bOffLine = true;

    DEBUG_PRINTF("NodeEntry[%d] = %u bytes [%u Kb]", artnet::POLL_TABLE_SIZE_ENRIES,
                 static_cast<unsigned int>(sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES])),
                 static_cast<unsigned int>(sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES])) / 1024U);
    DEBUG_PRINTF("PollTableUniverses[%d] = %u bytes [%u Kb]", artnet::POLL_TABLE_SIZE_UNIVERSES,
                 static_cast<unsigned int>(sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES])),
                 static_cast<unsigned int>(sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES])) / 1024U);
    DEBUG_EXIT();
}

ArtNetPollTable::~ArtNetPollTable()
{
    DEBUG_ENTRY();

    for (uint32_t nIndex = 0; nIndex < artnet::POLL_TABLE_SIZE_UNIVERSES; nIndex++)
    {
        delete[] table_universes_[nIndex].pIpAddresses;
        table_universes_[nIndex].pIpAddresses = nullptr;
    }

    delete[] table_universes_;
    table_universes_ = nullptr;

    delete[] table_;
    table_ = nullptr;

    DEBUG_EXIT();
}

const struct artnet::PollTableUniverses* ArtNetPollTable::GetIpAddress(uint16_t universe) const
{
    if (universes_entries_ == 0)
    {
        return nullptr;
    }

    // FIXME Universe lookup
    for (uint32_t nEntry = 0; nEntry < universes_entries_; nEntry++)
    {
        artnet::PollTableUniverses* pTableUniverses = &table_universes_[nEntry];
        assert(pTableUniverses != nullptr);

        if (pTableUniverses->universe == universe)
        {
            return &table_universes_[nEntry];
        }
    }

    return nullptr;
}

void ArtNetPollTable::RemoveIpAddress(const uint16_t universe, const uint32_t nIpAddress)
{
    if (universes_entries_ == 0)
    {
        return;
    }

    uint32_t nEntry = 0;

    // FIXME Universe lookup
    for (nEntry = 0; nEntry < universes_entries_; nEntry++)
    {
        artnet::PollTableUniverses* pTableUniverses = &table_universes_[nEntry];
        assert(pTableUniverses != nullptr);

        if (pTableUniverses->universe == universe)
        {
            break;
        }
    }

    if (nEntry == universes_entries_)
    {
        // Universe not found
        return;
    }

    artnet::PollTableUniverses* pTableUniverses = &table_universes_[nEntry];
    assert(pTableUniverses->nCount > 0);

    uint32_t nIpAddressIndex = 0;

    // FIXME IP lookup
    for (nIpAddressIndex = 0; nIpAddressIndex < pTableUniverses->nCount; nIpAddressIndex++)
    {
        if (pTableUniverses->pIpAddresses[nIpAddressIndex] == nIpAddress)
        {
            break;
        }
    }

    auto* p32 = pTableUniverses->pIpAddresses;
    int32_t i;

    for (i = static_cast<int32_t>(nIpAddressIndex); i < static_cast<int32_t>(pTableUniverses->nCount) - 1; i++)
    {
        p32[i] = p32[i + 1];
    }

    p32[i] = 0;

    pTableUniverses->nCount--;

    if (pTableUniverses->nCount == 0)
    {
        DEBUG_PRINTF("Delete Universe -> universes_entries_=%u, nEntry=%u", universes_entries_, nEntry);

        artnet::PollTableUniverses* p = table_universes_;

        for (i = static_cast<int32_t>(nEntry); i < static_cast<int32_t>(universes_entries_) - 1; i++)
        {
            p[i].universe = p[i + 1].universe;
            p[i].nCount = p[i + 1].nCount;
            p[i].pIpAddresses = p[i + 1].pIpAddresses;
        }

        p[i].universe = 0;
        p[i].nCount = 0;

        universes_entries_--;
    }
}

void ArtNetPollTable::ProcessUniverse(const uint32_t nIpAddress, const uint16_t universe)
{
    DEBUG_ENTRY();

    if (artnet::POLL_TABLE_SIZE_UNIVERSES == universes_entries_)
    {
        DEBUG_PUTS("table_universes_ is full");
        DEBUG_EXIT();
        return;
    }

    // FIXME Universe lookup
    auto bFoundUniverse = false;
    uint32_t nEntry = 0;

    for (nEntry = 0; nEntry < universes_entries_; nEntry++)
    {
        auto* pTableUniverses = &table_universes_[nEntry];
        assert(pTableUniverses != nullptr);

        if (pTableUniverses->universe == universe)
        {
            bFoundUniverse = true;
            DEBUG_PRINTF("Universe found %u", universe);
            break;
        }
    }

    auto* pTableUniverses = &table_universes_[nEntry];
    auto bFoundIp = false;
    uint32_t nCount = 0;

    if (bFoundUniverse)
    {
        // FIXME IP lookup
        for (nCount = 0; nCount < pTableUniverses->nCount; nCount++)
        {
            if (pTableUniverses->pIpAddresses[nCount] == nIpAddress)
            {
                bFoundIp = true;
                DEBUG_PUTS("IP found");
                break;
            }
        }
    }
    else
    {
        // New universe
        pTableUniverses->universe = universe;
        universes_entries_++;
        DEBUG_PRINTF("New Universe %d", static_cast<int>(universe));
    }

    if (!bFoundIp)
    {
        if (pTableUniverses->nCount < artnet::POLL_TABLE_SIZE_ENRIES)
        {
            pTableUniverses->pIpAddresses[pTableUniverses->nCount] = nIpAddress;
            pTableUniverses->nCount++;
            DEBUG_PUTS("It is a new IP for the Universe");
        }
        else
        {
            DEBUG_PUTS("New IP does not fit");
        }
    }

    DEBUG_EXIT();
}

void ArtNetPollTable::Add(const struct artnet::ArtPollReply* poll_reply)
{
    DEBUG_ENTRY();

    auto bFound = false;

    memcpy(ip.u8, poll_reply->IPAddress, 4);

    const auto kIpSwap = __builtin_bswap32(ip.u32);

    int32_t i;
    int32_t nLow = 0;
    int32_t nMid;
    auto nHigh = static_cast<int32_t>(table_entries_);

    while (nLow <= nHigh)
    {
        nMid = nLow + ((nHigh - nLow) / 2);
        const auto kMidValue = __builtin_bswap32(table_[nMid].IPAddress);

        if (kMidValue < kIpSwap)
        {
            nLow = nMid + 1;
        }
        else if (kMidValue > kIpSwap)
        {
            nHigh = nMid - 1;
        }
        else
        {
            i = nMid;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        if (table_entries_ == artnet::POLL_TABLE_SIZE_ENRIES)
        {
            DEBUG_PUTS("Full");
            return;
        }

        if (table_entries_ != static_cast<uint32_t>(nHigh))
        {
            DEBUG_PUTS("Move");

            auto* pArtNetNodeEntry = table_; // TODO

            assert(table_entries_ >= 1);
            assert(nLow >= 0);

            for (int32_t entry = static_cast<int32_t>(table_entries_) - 1; entry >= nLow; entry--)
            {
                const struct artnet::NodeEntry* pSrc = &pArtNetNodeEntry[entry];
                struct artnet::NodeEntry* pDst = &pArtNetNodeEntry[entry + 1];
                memcpy(pDst, pSrc, sizeof(struct artnet::NodeEntry));
            }

            auto* pDst = &pArtNetNodeEntry[nLow];
            memset(pDst, 0, sizeof(struct artnet::NodeEntry));

            i = nLow;
        }
        else
        {
            i = static_cast<int32_t>(table_entries_);
            DEBUG_PRINTF("Add -> i=%d", i);
        }

        table_[i].IPAddress = ip.u32;
        table_entries_++;
    }

    if (poll_reply->bind_index <= 1)
    {
        memcpy(table_[i].Mac, poll_reply->MAC, artnet::kMacSize);
        const uint8_t* pSrc = poll_reply->LongName;
        uint8_t* pDst = table_[i].LongName;
        memcpy(pDst, pSrc, artnet::kLongNameLength);
    }

    const auto kMillis = hal::Millis();

    for (uint32_t port_index = 0; port_index < artnet::kPorts; port_index++)
    {
        const auto kPortAddress = poll_reply->SwOut[port_index];

        if (poll_reply->PortTypes[port_index] == static_cast<uint8_t>(artnet::PortType::kOutputArtnet))
        {
            const auto kUniverse = artnet::MakePortAddress(poll_reply->NetSwitch, poll_reply->SubSwitch, kPortAddress);

            uint32_t nIndexUniverse;

            for (nIndexUniverse = 0; nIndexUniverse < table_[i].universes_count; nIndexUniverse++)
            {
                if (table_[i].Universe[nIndexUniverse].universe == kUniverse)
                {
                    break;
                }
            }

            if (nIndexUniverse == table_[i].universes_count)
            {
                // Not found
                if (table_[i].universes_count < artnet::POLL_TABLE_SIZE_NODE_UNIVERSES)
                {
                    table_[i].universes_count++;
                    table_[i].Universe[nIndexUniverse].universe = kUniverse;
                    const auto* src = poll_reply->ShortName;
                    auto* dst = table_[i].Universe[nIndexUniverse].ShortName;
                    memcpy(dst, src, artnet::kShortNameLength);
                    ProcessUniverse(ip.u32, kUniverse);
                }
                else
                {
                    // No room
                    continue;
                }
            }

            table_[i].Universe[nIndexUniverse].nLastUpdateMillis = kMillis;
        }
    }

    DEBUG_EXIT();;
}

void ArtNetPollTable::Clean()
{
    if (table_entries_ == 0)
    {
        return;
    }

    assert(table_clean_.nTableIndex < table_entries_);
    assert(table_clean_.universe_index < artnet::POLL_TABLE_SIZE_NODE_UNIVERSES);

    if (table_clean_.universe_index == 0)
    {
        table_clean_.bOffLine = true;
    }

    auto* pArtNetNodeEntryBind = &table_[table_clean_.nTableIndex].Universe[table_clean_.universe_index];

    if (pArtNetNodeEntryBind->nLastUpdateMillis != 0)
    {
        if ((hal::Millis() - pArtNetNodeEntryBind->nLastUpdateMillis) > (1.5 * artnet::POLL_INTERVAL_MILLIS))
        {
            pArtNetNodeEntryBind->nLastUpdateMillis = 0;
            RemoveIpAddress(pArtNetNodeEntryBind->universe, table_[table_clean_.nTableIndex].IPAddress);
        }
        else
        {
            table_clean_.bOffLine = false;
        }
    }

    table_clean_.universe_index++;

    if (table_clean_.universe_index == artnet::POLL_TABLE_SIZE_NODE_UNIVERSES)
    {
        if (table_clean_.bOffLine)
        {
            DEBUG_PUTS("Node is off-line");

            auto* pArtNetNodeEntry = table_;
            // Move
            for (uint32_t i = table_clean_.nTableIndex; i < (table_entries_ - 1); i++)
            {
                const auto* pSrc = &pArtNetNodeEntry[i + 1];
                auto* pDst = &pArtNetNodeEntry[i];
                memcpy(pDst, pSrc, sizeof(struct artnet::NodeEntry));
            }

            table_entries_--;

            auto* dst = &pArtNetNodeEntry[table_entries_];
            dst->IPAddress = 0;
            dst->universes_count = 0;
            memset(dst->Universe, 0, sizeof(struct artnet::NodeEntryUniverse[artnet::POLL_TABLE_SIZE_NODE_UNIVERSES]));
#ifndef NDEBUG
            memset(dst->Mac, 0, artnet::kMacSize + artnet::kLongNameLength);
#endif
        }

        table_clean_.universe_index = 0;
        table_clean_.bOffLine = true;
        table_clean_.nTableIndex++;

        if (table_clean_.nTableIndex >= table_entries_)
        {
            table_clean_.nTableIndex = 0;
        }
    }
}

void ArtNetPollTable::Dump()
{
#ifndef NDEBUG
    printf("Entries : %d\n", table_entries_);

    for (uint32_t i = 0; i < table_entries_; i++)
    {
        printf("\t" IPSTR " [" MACSTR "] |%-64s|\n", IP2STR(table_[i].IPAddress), MAC2STR(table_[i].Mac), table_[i].LongName);

        for (uint32_t universe = 0; universe < table_[i].universes_count; universe++)
        {
            auto* pArtNetNodeEntryUniverse = &table_[i].Universe[universe];
            printf("\t %u [%u] |%-18s|\n", pArtNetNodeEntryUniverse->universe, (hal::Millis() - pArtNetNodeEntryUniverse->nLastUpdateMillis) / 1000U,
                   pArtNetNodeEntryUniverse->ShortName);
        }
        puts("");
    }
#endif
}

void ArtNetPollTable::DumpTableUniverses()
{
#ifndef NDEBUG
    printf("Entries : %d\n", universes_entries_);

    for (uint32_t entry = 0; entry < universes_entries_; entry++)
    {
        const auto* const kTableUniverses = &table_universes_[entry];
        assert(kTableUniverses != nullptr);

        printf("%3d |%4u | %d ", entry, kTableUniverses->universe, kTableUniverses->nCount);

        const auto* ip_addresses = kTableUniverses->pIpAddresses;
        assert(ip_addresses != nullptr);

        for (uint32_t count = 0; count < kTableUniverses->nCount; count++)
        {
            printf(" " IPSTR, IP2STR(ip_addresses[count]));
        }

        puts("");
    }

    puts("");
#endif
}
