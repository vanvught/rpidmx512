#if defined(RDM_CONTROLLER)
/**
 * @file json_status_rdm.cpp
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetnode.h"

namespace json::status
{
static uint32_t PortStatus(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index)
{
    const auto kDirection = ArtNetNode::Get()->GetPortDirection(port_index);
    const char* status;

    if (kDirection == dmxnode::PortDirection::kOutput)
    {
        if (ArtNetNode::Get()->Rdm(port_index))
        {
            if (ArtNetNode::Get()->RdmBgDiscovery(port_index))
            {
                bool is_incremental;
                if (ArtNetNode::Get()->RdmIsRunning(port_index, is_incremental))
                {
                    status = is_incremental ? "Incremental" : "Full";
                }
                else
                {
                    status = "Idle";
                }
            }
            else
            {
                status = "Disabled";
            }
        }
        else
        {
            return 0;
        }
    }
    else if (kDirection == dmxnode::PortDirection::kInput)
    {
        if (ArtNetNode::Get()->RdmTodUidCount(port_index) != 0)
        {
            status = "TOD";
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, "{\"port\":\"%c\",\"direction\":\"%s\",\"status\":\"%s\"},",
                                                 static_cast<char>('A' + port_index), dmxnode::GetPortDirection(kDirection), status));

    return length;
}

uint32_t Rdm(char* out_buffer, uint32_t out_buffer_size)
{
    out_buffer[0] = '[';
    uint32_t length = 1;

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        length += PortStatus(&out_buffer[length], out_buffer_size - length, port_index);
    }

    if (length == 1)
    {
        length++;
    }

    out_buffer[length - 1] = ']';

    return length;
}

uint32_t RdmQueue(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kBufferSize = out_buffer_size - 2U;
    auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, "{\"uid\":["));

    length += ArtNetNode::Get()->RdmCopyWorkingQueue(&out_buffer[length], kBufferSize - length);

    out_buffer[length++] = ']';
    out_buffer[length++] = '}';

    assert(length <= out_buffer_size);
    return length;
}

uint32_t RdmTod(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index)
{
    if (port_index < dmxnode::kMaxPorts)
    {
        const auto kBufferSize = out_buffer_size - 2U;
        auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, "{\"port\":\"%c\",\"tod\":[", static_cast<char>(port_index + 'A')));

        length += ArtNetNode::Get()->RdmCopyTod(port_index, &out_buffer[length], kBufferSize - length);

        out_buffer[length++] = ']';
        out_buffer[length++] = '}';

        assert(length <= out_buffer_size);
        return length;
    }

    return 0;
}
} // namespace json::status
#endif
