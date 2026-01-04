/**
 * @file dmxnode_nodetype.h
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

#ifndef DMXNODE_NODETYPE_H_
#define DMXNODE_NODETYPE_H_

#include <cstdint>

namespace dmxnode
{
enum class NodeType
{
    kArtnet,
    kE131,
    kDdp,
    kPp,
    kDmx,
    kLtc,
    kOscClient,
    kOscServer,
    kRdmResponder,
    kRdmNet,
    kShowfile,
    kUndefined
};

inline constexpr const char* kNodeTypeNames[static_cast<uint32_t>(NodeType::kUndefined)] = 
{
	"Artnet",    
	"sACN E1.31", 
	"DDP Display", 
	"PixelPusher",
	"DMX",        
	"LTC SMPTE", 
	"OSC Client", 
	"OSC Server",  
	"RDM Responder",
	"RDMNet LLRP Only"
};

inline const char* GetNodeType(NodeType type)
{
    if (type < NodeType::kUndefined)
    {
        return kNodeTypeNames[static_cast<uint32_t>(type)];
    }

    return "Undefined";
}
} // namespace dmxnode

#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#define DMXNODE_TYPE_ARTNET
#define DMXNODE_NODETYPE_DEFINED
#include "artnetnode.h"
using DmxNodeNodeType = ArtNetNode;
#endif

#if defined(NODE_E131) || defined(NODE_E131_MULTI)
#define DMXNODE_TYPE_E131
#define DMXNODE_NODETYPE_DEFINED
#include "e131bridge.h"
using DmxNodeNodeType = E131Bridge;
#endif

#if defined(NODE_DDP_DISPLAY)
#define DMXNODE_TYPE_DDP
#define DMXNODE_NODETYPE_DEFINED
#include "ddpdisplay.h"
using DmxNodeNodeType = DdpDisplay;
#endif

#if defined (NODE_PP)
#define DMXNODE_TYPE_PP
#endif

#if defined(NODE_DMX)
#define DMXNODE_TYPE_DMX
#endif

#if defined(NODE_LTC_SMPTE)
#define DMXNODE_TYPE_LTC
#endif

#if defined(NODE_OSC_CLIENT)
#define DMXNODE_TYPE_OSCCLIENT
#endif

#if defined(NODE_OSC_SERVER)
#define DMXNODE_TYPE_OSCSERVER
#endif

#if defined (RDM_RESPONDER)
#define DMXNODE_TYPE_RDM_RESPONDER
#endif

#if defined(NODE_RDMNET_LLRP_ONLY)
#define DMXNODE_TYPE_RDMNET_LLRP_ONLY
#endif

#if defined(NODE_SHOWFILE) && !defined(DMXNODE_NODETYPE_DEFINED)
#define DMXNODE_TYPE_SHOWFILE
#endif

namespace dmxnode
{
#if defined (DMXNODE_TYPE_LTC)
inline constexpr auto kNodeType = NodeType::kLtc;
#elif defined (DMXNODE_TYPE_ARTNET)
inline constexpr auto kNodeType = NodeType::kArtnet;
#elif defined (DMXNODE_TYPE_E131)
inline constexpr auto kNodeType = NodeType::kE131;
#elif defined (DMXNODE_TYPE_DDP)
inline constexpr auto kNodeType = NodeType::kDdp;
#elif defined (DMXNODE_TYPE_PP)
inline constexpr auto kNodeType = NodeType::kPp;
#elif defined (DMXNODE_TYPE_DMX)
inline constexpr auto kNodeType = NodeType::kDmx;
#elif defined (DMXNODE_TYPE_OSCCLIENT)
inline constexpr auto kNodeType = NodeType::kOscClient;
#elif defined (DMXNODE_TYPE_OSCSERVER)
inline constexpr auto kNodeType = NodeType::kOscServer;
#elif defined (DMXNODE_TYPE_RDM_RESPONDER)
inline constexpr auto kNodeType = NodeType::kRdmResponder;
#elif defined (DMXNODE_TYPE_RDMNET_LLRP_ONLY)
inline constexpr auto kNodeType = NodeType::kRdmNet;
#elif defined (DMXNODE_TYPE_SHOWFILE)
inline constexpr auto kNodeType = NodeType::kShowfile;
#else
inline constexpr auto kNodeType = NodeType::kUndefined;
#endif
} // namespace dmxnode

#endif  // DMXNODE_NODETYPE_H_
