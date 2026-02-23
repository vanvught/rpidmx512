/**
 * @file displayudfparamsconst.h
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

#ifndef JSON_DISPLAYUDFPARAMSCONST_H_
#define JSON_DISPLAYUDFPARAMSCONST_H_

#include "json/json_key.h"
#if defined (DMXNODE_PORTS) && (DMXNODE_PORTS > 0)
#include "json/dmxnodeparamsconst.h"
#endif
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)
#include "json/artnetparamsconst.h"
#endif
#if defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)
#include "dmx.h"
#endif

namespace json
{
struct DisplayUdfParamsConst
{
	static constexpr char kFileName[] = "display.json";
   
	static constexpr json::SimpleKey kIntensity {
	    "intensity",
	    9,
	    Fnv1a32("intensity", 9)
	};
	
	static constexpr json::SimpleKey kSleepTimeout {
	    "sleep_timeout",
	    13,
	    Fnv1a32("sleep_timeout", 13)
	};
	
	static constexpr json::SimpleKey kFlipVertically {
	    "flip_vertically",
	    15,
	    Fnv1a32("flip_vertically", 15)
	};
	
	static constexpr json::PortKey kTitle {
	    "title",
	    5,
	    Fnv1a32("title", 5)
	};
	
	static constexpr json::PortKey kBoardName {
	    "board_name",
	    10,
	    Fnv1a32("board_name", 10)
	};
	
	static constexpr json::PortKey kVersion {
	    "version",
	    7,
	    Fnv1a32("version", 7)
	};

	static constexpr json::PortKey kActivePorts {
	    "active_ports",
	    12,
	    Fnv1a32("active_ports", 12)
	};
	
	static constexpr json::PortKey kHostname {
	    "hostname",
	    8,
	    Fnv1a32("hostname", 8)
	};
		
	static constexpr json::PortKey kIpAddress {
	    "ip_address",
	    10,
	    Fnv1a32("ip_address", 10)
	};
	
	static constexpr json::PortKey kNetMask {
	    "net_mask",
	    8,
	    Fnv1a32("net_mask", 8)
	};
	
	static constexpr json::PortKey kDefaultGateway {
	    "default_gateway",
	    15,
	    Fnv1a32("default_gateway", 15)
	};
	
	static constexpr json::PortKey kDmxStartAddress {
	    "dmx_start_address",
	    17,
	    Fnv1a32("dmx_start_address", 17)
	};
		
	static constexpr json::PortKey kLabels[] = 
    {
		kTitle,
		kBoardName,
		kVersion,
		kHostname,
		kIpAddress,
		kNetMask,
		kDefaultGateway,
		kActivePorts,	
		kDmxStartAddress,
#if defined (DMX_MAX_PORTS)
#if (DMX_MAX_PORTS == 1)	
		DmxNodeParamsConst::kUniversePortA,
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)			
		ArtNetParamsConst::kDestinationIpPortA
#endif		
#endif		
#if (DMX_MAX_PORTS == 2)		
		DmxNodeParamsConst::kUniversePortA,
		DmxNodeParamsConst::kUniversePortB,
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
		ArtNetParamsConst::kDestinationIpPortA,
		ArtNetParamsConst::kDestinationIpPortB	
#endif			
#endif
#if (DMX_MAX_PORTS == 3)		
		DmxNodeParamsConst::kUniversePortA,
		DmxNodeParamsConst::kUniversePortB,
		DmxNodeParamsConst::kUniversePortC,
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
		ArtNetParamsConst::kDestinationIpPortA,
		ArtNetParamsConst::kDestinationIpPortB,
		ArtNetParamsConst::kDestinationIpPortC	
#endif			
#endif
#if (DMX_MAX_PORTS == 4)		
		DmxNodeParamsConst::kUniversePortA,
		DmxNodeParamsConst::kUniversePortB,
		DmxNodeParamsConst::kUniversePortC,
		DmxNodeParamsConst::kUniversePortD,
#if defined(NODE_ARTNET) || defined (NODE_ARTNET_MULTI)		
		ArtNetParamsConst::kDestinationIpPortA,
		ArtNetParamsConst::kDestinationIpPortB,
		ArtNetParamsConst::kDestinationIpPortC,	
		ArtNetParamsConst::kDestinationIpPortD	
#endif			
#endif
#endif   
	};
};
} // namespace json

#endif  // JSON_DISPLAYUDFPARAMSCONST_H_
