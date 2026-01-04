/**
 * @file dmxserialhandleudp.cpp
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
#include <cstring>
#include <errno.h>

#include "dmxserial.h"
#include "dmxserial_internal.h"
#include "hal.h"
#include "net/protocol/udp.h"
#include "net/udp.h"
#include "firmware/debug/debug_dump.h"

namespace cmd
{
inline constexpr char kRequestFiles[] = "?files#";
inline constexpr char kGetTftp[] = "?tftp#";
inline constexpr char kSetTftp[] = "!tftp#";
inline constexpr char kRequestReload[] = "?reload##";
inline constexpr char kRequestDelete[] = "!delete#";
} // namespace cmd

namespace length
{
inline constexpr auto kRequestFiles = sizeof(cmd::kRequestFiles) - 1;
inline constexpr auto kGetTftp = sizeof(cmd::kGetTftp) - 1;
inline constexpr auto kSetTftp = sizeof(cmd::kSetTftp) - 1;
inline constexpr auto kRequestReload = sizeof(cmd::kRequestReload) - 1;
inline constexpr auto kRequestDelete = sizeof(cmd::kRequestDelete) - 1;
} // namespace length

void DmxSerial::Input(const uint8_t* p, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((size < 6), 0))
    {
        return;
    }

    auto* buffer = const_cast<uint8_t*>(p);

    if (buffer[size - 1] == '\n')
    {
        size--;
    }

#ifndef NDEBUG
    debug::Dump(buffer, size);
#endif

    if (memcmp(buffer, cmd::kRequestFiles, length::kRequestFiles) == 0)
    {
        for (uint32_t i = 0; i < m_nFilesCount; i++)
        {
            const auto kLength =
                snprintf(reinterpret_cast<char*>(buffer), UDP_DATA_SIZE - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX "\n", m_aFileIndex[i]);
            net::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
        }
        return;
    }

    if ((size >= length::kGetTftp) && (memcmp(buffer, cmd::kGetTftp, length::kGetTftp) == 0))
    {
        if (size == length::kGetTftp)
        {
            const auto kLength = snprintf(reinterpret_cast<char*>(buffer), UDP_DATA_SIZE - 1, "tftp:%s\n", enable_tftp_ ? "On" : "Off");
            net::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
            return;
        }

        if (size == length::kGetTftp + 3)
        {
            if (memcmp(&buffer[length::kGetTftp], "bin", 3) == 0)
            {
                net::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&enable_tftp_), sizeof(bool), from_ip, UDP::PORT);
                return;
            }
        }
    }

    if ((size == length::kSetTftp + 1) && (memcmp(buffer, cmd::kSetTftp, length::kSetTftp) == 0))
    {
        EnableTFTP(buffer[length::kSetTftp] != '0');
        return;
    }

    if (memcmp(buffer, cmd::kRequestReload, length::kRequestReload) == 0)
    {
        hal::Reboot();
        return;
    }

    if ((size == length::kRequestDelete + 3) && (memcmp(buffer, cmd::kRequestDelete, length::kRequestDelete) == 0))
    {
        buffer[length::kRequestDelete + 3] = '\0';

        if (DmxSerial::Get()->DeleteFile(reinterpret_cast<char*>(&buffer[length::kRequestDelete])))
        {
            net::udp::Send(handle_, reinterpret_cast<const uint8_t*>("Success\n"), 8, from_ip, UDP::PORT);
        }
        else	
        {
            const auto* error = strerror(errno);
            const auto kLength = snprintf(reinterpret_cast<char*>(buffer), UDP_DATA_SIZE - 1, "%s\n", error);
            net::udp::Send(handle_, buffer, kLength, from_ip, UDP::PORT);
        }
        return;
    }
}
