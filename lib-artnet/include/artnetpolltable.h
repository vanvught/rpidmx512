/**
 * @file artnetpolltable.h
 *
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

#ifndef ARTNETPOLLTABLE_H_
#define ARTNETPOLLTABLE_H_

#include <cstdint>

#include "artnet.h"

namespace artnet
{
inline constexpr uint32_t POLL_INTERVAL_SECONDS = 8;
inline constexpr uint32_t POLL_INTERVAL_MILLIS = (POLL_INTERVAL_SECONDS * 1000U);
inline constexpr uint32_t POLL_TABLE_SIZE_ENRIES = 255;
inline constexpr uint32_t POLL_TABLE_SIZE_NODE_UNIVERSES = 64;
inline constexpr uint32_t POLL_TABLE_SIZE_UNIVERSES = 512;

struct NodeEntryUniverse
{
    uint8_t ShortName[artnet::kShortNameLength];
    uint32_t nLastUpdateMillis;
    uint16_t universe;
};

struct NodeEntry
{
    uint32_t IPAddress;
    uint8_t Mac[artnet::kMacSize];
    uint8_t LongName[artnet::kLongNameLength];
    uint16_t universes_count;
    struct NodeEntryUniverse Universe[artnet::POLL_TABLE_SIZE_NODE_UNIVERSES];
};

struct PollTableUniverses
{
    uint16_t universe;
    uint16_t nCount;
    uint32_t* pIpAddresses;
};

struct PollTableClean
{
    uint32_t nTableIndex;
    uint16_t universe_index;
    bool bOffLine;
};
} // namespace artnet

class ArtNetPollTable
{
   public:
    ArtNetPollTable();
    ~ArtNetPollTable();

    const artnet::NodeEntry* GetPollTable() const { return table_; }

    uint32_t GetPollTableEntries() const { return table_entries_; }

    void Add(const struct artnet::ArtPollReply* reply);
    void Clean();

    const struct artnet::PollTableUniverses* GetIpAddress(uint16_t universe) const;

    void Dump();
    void DumpTableUniverses();

   private:
    void ProcessUniverse(uint32_t ip_address, uint16_t universe);
    void RemoveIpAddress(uint16_t universe, uint32_t ip_address);

   private:
    artnet::NodeEntry* table_;
    artnet::PollTableUniverses* table_universes_;
    uint32_t table_entries_{0};
    uint32_t universes_entries_{0};
    artnet::PollTableClean table_clean_;
};

#endif  // ARTNETPOLLTABLE_H_
