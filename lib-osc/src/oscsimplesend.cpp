/**
 * @file oscsimplesend.cpp
 *
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

#include <cstring>
#include <cassert>

#include "oscsimplesend.h"
#include "oscstring.h"
#include "network.h"
#include "firmware/debug/debug_dump.h"

// Support for path only
OscSimpleSend::OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type)
{
    if (type == nullptr)
    {
        const auto kPathLength = osc::StringSize(path);
        const auto kMessageLength = kPathLength + 4U;

        assert(kMessageLength < sizeof(s_message));

        UpdateMessage(path, kPathLength, 0);
        Send(kMessageLength, handle, ip_address, port);
    }
}

// Support for 's'
OscSimpleSend::OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, const char* string)
{
    if ((type != nullptr) && (*type == 's'))
    {
        const auto kPathLength = osc::StringSize(path);
        const auto kMessageLength = kPathLength + 4U + osc::StringSize(string);

        assert(kMessageLength < sizeof(s_message));

        UpdateMessage(path, kPathLength, 's');

        memset(s_message + kMessageLength - 4, 0, 4);

        assert((kPathLength + 4) < sizeof(s_message));
        strncpy(reinterpret_cast<char*>(&s_message[kPathLength + 4]), string, sizeof(s_message) - (kPathLength + 4));

        Send(kMessageLength, handle, ip_address, port);
    }
}

// Support for type 'i'
OscSimpleSend::OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, int value)
{
    if ((type != nullptr) && (*type == 'i'))
    {
        const auto kPathLength = osc::StringSize(path);
        const auto kMessageLength = kPathLength + 4U + 4U;

        assert(kMessageLength < sizeof(s_message));

        UpdateMessage(path, kPathLength, 'i');

        *reinterpret_cast<int32_t*>(&s_message[kMessageLength - 4]) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(value)));

        Send(kMessageLength, handle, ip_address, port);
    }
}

// Support for type 'f'
OscSimpleSend::OscSimpleSend(int32_t handle, uint32_t ip_address, uint16_t port, const char* path, const char* type, float value)
{
    if ((type != nullptr) && (*type == 'f'))
    {
        const auto kPathLength = osc::StringSize(path);
        const auto kMessageLength = kPathLength + 4U + 4U;

        assert(kMessageLength < sizeof(s_message));

        UpdateMessage(path, kPathLength, 'f');

        union pcast32
        {
            uint32_t u;
            float f;
        } osc_pcast32;

        osc_pcast32.f = value;

        *reinterpret_cast<int32_t*>(&s_message[kMessageLength - 4]) = static_cast<int32_t>(__builtin_bswap32(osc_pcast32.u));

        Send(kMessageLength, handle, ip_address, port);
    }
}

void OscSimpleSend::UpdateMessage(const char* path, uint32_t path_length, char type)
{
    memset(s_message + path_length - 4, 0, 4);
    strncpy(reinterpret_cast<char*>(s_message), path, sizeof(s_message));

    assert(path_length < sizeof(s_message));

    s_message[path_length++] = ',';
    s_message[path_length++] = type;
    s_message[path_length++] = '\0';
    s_message[path_length++] = '\0';
}

void OscSimpleSend::Send(uint32_t message_length, int32_t handle, uint32_t ip_address, uint16_t port)
{
    debug::Dump(s_message, message_length);

    network::udp::Send(handle, s_message, message_length, ip_address, port);
}
