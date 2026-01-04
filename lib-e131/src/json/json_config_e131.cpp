#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION >= 4)
/**
 * @file json_config_e131.cpp
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


#include "dmxnode_nodetype.h"
#include "e131bridge.h"
#include "json/e131params.h"
#include "json/e131paramsconst.h"
#include "json/json_helpers.h"

namespace json::config
{
uint32_t GetE131(char* buffer, uint32_t length)
{
	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    if constexpr (dmxnode::kConfigPortCount != 0)
	    {

	        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
	        {
	            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;
	
	            if (kPortIndex >= dmxnode::kMaxPorts)
	            {
	                break;
	            }
	            doc[json::E131ParamsConst::kPriorityPort[config_port_index].name] = E131Bridge::Get()->GetPriority(kPortIndex);
	        }
	    }
    });
}

void SetE131(const char* buffer, uint32_t buffer_size)
{
    ::json::E131Params e131_params;
    e131_params.Store(buffer, buffer_size);
    e131_params.Set();
}
} // namespace json::config
#endif