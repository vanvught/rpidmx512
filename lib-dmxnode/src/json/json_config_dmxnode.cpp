/**
 * @file json_config_dmxnode.cpp
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

#include "dmxnode_nodetype.h"
#include "json/dmxnodeparamsconst.h"
#include "json/dmxnodeparams.h"
#include "json/json_helpers.h"

namespace json::config
{
uint32_t GetDmxNode(char* buffer, uint32_t length)
{
    auto* dmx_node = DmxNodeNodeType::Get();
    assert(dmx_node != nullptr);

 	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {	
	    doc[json::DmxNodeParamsConst::kPersonality.name] = "node";
	    doc[json::DmxNodeParamsConst::kNodeName.name] = dmx_node->GetLongName();
	    doc[json::DmxNodeParamsConst::kFailsafe.name] = dmxnode::GetFailsafe(dmx_node->GetFailSafe());
	    doc[json::DmxNodeParamsConst::kDisableMergeTimeout.name] = dmx_node->GetDisableMergeTimeout() ? 1 : 0;
	
	    if constexpr (dmxnode::kConfigPortCount != 0)
	    {
	        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
	        {
	            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;
	
	            if (kPortIndex >= dmxnode::kMaxPorts)
	            {
	                break;
	            }
	            
	            const auto kPortDirection = dmx_node->GetPortDirection(kPortIndex);
	            uint16_t universe = 0;
	            dmx_node->GetUniverse(kPortIndex, universe, kPortDirection);
	
	            doc[json::DmxNodeParamsConst::kLabelPort[config_port_index].name] = dmx_node->GetShortName(kPortIndex);
	            doc[json::DmxNodeParamsConst::kUniversePort[config_port_index].name] = universe;
	            doc[json::DmxNodeParamsConst::kDirectionPort[config_port_index].name] = dmxnode::GetPortDirection(kPortDirection);
	            doc[json::DmxNodeParamsConst::kMergeModePort[config_port_index].name] = dmxnode::GetMergeMode(dmx_node->GetMergeMode(kPortIndex));
#if defined(OUTPUT_HAVE_STYLESWITCH)
	            doc[json::DmxNodeParamsConst::kOutputStylePort[config_port_index].name] = dmxnode::GetOutputStyle(dmx_node->GetOutputStyle(kPortIndex));
#endif
	        }
	    }
    });
}

void SetDmxNode(const char* buffer, uint32_t buffer_size)
{
    ::json::DmxNodeParams dmxnode_params;
    dmxnode_params.Store(buffer, buffer_size);
    dmxnode_params.Set();
}
} // namespace json::config
