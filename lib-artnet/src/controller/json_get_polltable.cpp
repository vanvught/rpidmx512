/**
 * @file json_get_polltable.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetcontroller.h"
#include "artnet.h"
#include "ip4/ip4_address.h"

namespace remoteconfig::artnet::controller
{
static uint32_t GetPort(const struct ::artnet::NodeEntryUniverse* art_net_node_entry_universe, char* out_buffer, uint32_t out_buffer_size)
{
    const auto kLength = static_cast<uint32_t>(
        snprintf(out_buffer, out_buffer_size, "{\"name\":\"%s\",\"universe\":%u},", art_net_node_entry_universe->ShortName, art_net_node_entry_universe->universe));

    if (kLength <= out_buffer_size)
    {
        return kLength;
    }

    return 0;
}

static uint32_t GetEntry(uint32_t index, char* out_buffer, uint32_t out_buffer_size)
{
    const auto* poll_table = ArtNetController::Get()->GetPollTable();
    auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, "{\"name\":\"%s\",\"ip\":\"" IPSTR "\",\"mac\":\"" MACSTR "\",\"ports\":[",
                                                  poll_table[index].LongName, IP2STR(poll_table[index].IPAddress), MAC2STR(poll_table[index].Mac)));

    for (uint32_t universe = 0; universe < poll_table[index].universes_count; universe++)
    {
        const auto* art_net_node_entry_universe = &poll_table[index].Universe[universe];
        length += GetPort(art_net_node_entry_universe, &out_buffer[length], length);
    }

    length--;
    length += static_cast<uint32_t>(snprintf(&out_buffer[length], out_buffer_size - length, "]},"));

    if (length <= out_buffer_size)
    {
        return length;
    }

    return 0;
}

uint32_t JsonGetPolltable(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kBufferSize = out_buffer_size - 2U;
    out_buffer[0] = '[';

    auto length = 1U;

    for (uint32_t index = 0; (index < ArtNetController::Get()->GetPollTableEntries()) && (length < out_buffer_size); index++)
    {
        const auto kSize = kBufferSize - length;
        length += GetEntry(index, &out_buffer[length], kSize);
    }

    if (length != 1)
    {
        out_buffer[length - 1] = ']';
    }
    else
    {
        out_buffer[1] = ']';
        length = 2;
    }

    assert(length <= out_buffer_size);
    return length;
}
} // namespace remoteconfig::artnet::controller
