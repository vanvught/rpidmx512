/**
 * @file json_config_oscclient.cpp
 *
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

#include "ip4/ip4_helpers.h"
#include "json/oscclientparams.h"
#include "json/json_helpers.h"
#include "json/oscclientparamsconst.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "configstore.h"
#include "oscclient.h"

using common::store::osc::client::Flags;

namespace json::config
{
uint32_t GetOscClient(char* buffer, uint32_t length)
{
    auto& oscclient = OscClient::Instance();
    const auto kFlags = ConfigStore::Instance().OscClientGet(&common::store::OscClient::flags);

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
		char ip[net::kIpBufferSize];	
	    doc[OscClientParamsConst::kServerIp.name] = net::FormatIp(oscclient.GetServerIP(), ip);
	    doc[OscParamsConst::kIncomingPort.name] = oscclient.GetPortIncoming();
	    doc[OscParamsConst::kOutgoingPort.name] = oscclient.GetPortOutgoing();
	    doc[OscClientParamsConst::kPingDisable.name] = common::IsFlagSet(kFlags, Flags::Flag::kPingDisable);
	    doc[OscClientParamsConst::kPingDelay.name] = oscclient.GetPingDelaySeconds();
	
	    for (uint32_t i = 0; i < common::store::osc::client::kCmdCount; i++)
	    {
	        doc[OscClientParamsConst::kCmd[i].name] = oscclient.GetCmd(i);
	    }
	
	    for (uint32_t i = 0; i < common::store::osc::client::kLedCount; i++)
	    {
	        doc[OscClientParamsConst::kLed[i].name] = oscclient.GetLed(i);
	    }
    });
}

void SetOscClient(const char* buffer, uint32_t buffer_size)
{
    ::json::OscClientParams oscclient_params;
    oscclient_params.Store(buffer, buffer_size);
    oscclient_params.Set();
}
} // namespace json::config
