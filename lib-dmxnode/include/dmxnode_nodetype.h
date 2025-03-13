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

#if defined (NODE_ARTNET) || defined (NODE_ARTNET_MULTI)
# define DMXNODE_TYPE_ARTNETNODE
# include "artnetnode.h"
using DmxNodeNodeType = ArtNetNode;
#endif

#if defined (NODE_E131) || defined (NODE_E131_MULTI)
# define DMXNODE_TYPE_E131BRIDGE
# include "e131bridge.h"
using DmxNodeNodeType = E131Bridge;
#endif

#if defined (NODE_OSC_SERVER)
# define DMXNODE_TYPE_OSC_SERVER
#endif

#if defined (NODE_RDMNET_LLRP_ONLY)
# define DMXNODE_TYPE_RDMNET_LLRP_ONLY
#endif

#endif /* DMXNODE_NODETYPE_H_ */
