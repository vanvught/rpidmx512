/**
 * @file tcnetdump.cpp
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

#include <cstdint>
#include <cstdio>

#include "tcnet.h"
 #include "firmware/debug/debug_debug.h"

static uint32_t s_timestamp_previous = 0;

void TCNet::DumpManagementHeader(const uint8_t* buffer)
{
    const auto* management_header = reinterpret_cast<const struct TTCNetPacketManagementHeader*>(buffer);

    printf("ManagementHeader\n");
    printf(" %.3s V%d.%d %.8s\n", management_header->Header, management_header->ProtocolVersionMajor, management_header->ProtocolVersionMinor,
           management_header->NodeName);
    printf(" %s\n", management_header->NodeType == TCNET_TYPE_SLAVE ? "SLAVE" : (management_header->NodeType == TCNET_TYPE_MASTER ? "MASTER" : "AUTO"));
    printf(" %u [%u] %d\n", management_header->TimeStamp, management_header->TimeStamp - s_timestamp_previous, management_header->SEQ);

    s_timestamp_previous = management_header->TimeStamp;
}

void TCNet::DumpOptIn(const uint8_t* buffer)
{
    DEBUG_ENTRY();

    DumpManagementHeader(buffer);

    const auto* opt_in = reinterpret_cast<const struct TTCNetPacketOptIn*>(buffer);

    printf(" OptIn\n");
    printf("  %d %d\n", opt_in->NodeCount, opt_in->NodeListenerPort);
    printf("  %d %d\n", opt_in->NodeCount, opt_in->Uptime);
    printf("  %.16s %.16s %d.%d.%d [%d]\n", opt_in->VendorName, opt_in->DeviceName, opt_in->DeviceMajorVersion,
           opt_in->DeviceMinorVersion, opt_in->DeviceBugVersion, opt_in->NodeCount);

    DEBUG_EXIT();
}
