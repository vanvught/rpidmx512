/**
 * @file json_config_artnet.cpp
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
#include <cassert>

#include "dmxnode.h"
#include "dmxnode_nodetype.h"
#include "json/artnetparams.h"
#include "json/artnetparamsconst.h"
#include "json/json_helpers.h"
#include "ip4/ip4_helpers.h"

namespace json::config
{
uint32_t GetArtNet(char* buffer, uint32_t length)
{
    auto* dmx_node = DmxNodeNodeType::Get();
    assert(dmx_node != nullptr);

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
	    doc[json::ArtNetParamsConst::kEnableRdm.name] = dmx_node->GetRdm();
#endif
#if (ARTNET_VERSION >= 4)    
	    doc[json::ArtNetParamsConst::kMapUniverse0.name] = dmx_node->IsMapUniverse0();
#endif
	
	    if constexpr (dmxnode::kConfigPortCount != 0)
	    {
	        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
	        {
	            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;
	
	            if (kPortIndex >= dmxnode::kMaxPorts)
	            {
	                break;
	            }
	
#if (ARTNET_VERSION >= 4)
	            doc[json::ArtNetParamsConst::kProtocolPort[config_port_index].name] = artnet::GetProtocolMode(dmx_node->GetPortProtocol4(kPortIndex));
#endif            
	            doc[json::ArtNetParamsConst::kRdmEnablePort[config_port_index].name] = dmx_node->GetRdm(kPortIndex);
	
	            char ip[net::kIpBufferSize];
	            doc[json::ArtNetParamsConst::kDestinationIpPort[config_port_index].name] = net::FormatIp(dmx_node->GetDestinationIp(kPortIndex), ip);
	        }
	    }
	});
}

void SetArtNet(const char* buffer, uint32_t buffer_size)
{
    ::json::ArtNetParams artnet_params;
    artnet_params.Store(buffer, buffer_size);
    artnet_params.Set();
}
} // namespace json::config