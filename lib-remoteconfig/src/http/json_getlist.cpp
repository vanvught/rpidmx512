/**
 * @file json_getlist.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "configstore.h"
#include "configurationstore.h"
#include "dmxnode_nodetype.h"
#include "dmxnode_outputtype.h"
#include "network.h"
#include "ip4/ip4_address.h"

namespace json
{
uint32_t GetList(char* out_buffer, uint32_t out_buffer_size)
{
    uint8_t display_name[common::store::remoteconfig::kDisplayNameLength];
    ConfigStore::Instance().RemoteConfigCopyArray(display_name, &common::store::RemoteConfig::display_name);

#if defined(DMXNODE_NODETYPE_DEFINED)
    if (display_name[0] == '\0')
    {
		const auto* name = DmxNodeNodeType::Get()->GetLongName();
        strncpy(reinterpret_cast<char *>(display_name), name, common::store::remoteconfig::kDisplayNameLength - 1);
        display_name[common::store::remoteconfig::kDisplayNameLength - 1] = '\0';
    }
#endif
#if defined(DMXNODE_PORTS)
    const uint32_t kPortCount = DMXNODE_PORTS;
#else
    const uint32_t kPortCount = 0;
#endif

    const auto kLength = static_cast<uint32_t>(snprintf(
        out_buffer, out_buffer_size, 
        "{\"list\":{\"ip\":\"" IPSTR "\",\"name\":\"%s\",\"node\":{\"type\":\"%s\",\"output\":{\"type\":\"%s\",\"count\":%d}}}}",
        IP2STR(network::GetPrimaryIp()), 
        display_name, 
        dmxnode::GetNodeType(dmxnode::kNodeType),
        dmxnode::GetOutputType(dmxnode::kOutputType),
        kPortCount));

    return kLength;
}
} // namespace json