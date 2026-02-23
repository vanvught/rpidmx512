/**
 * @file dmxnodenode.h
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

#ifndef DMXNODENODE_H_
#define DMXNODENODE_H_

#include "dmxnode_nodetype.h"
#include "json/dmxnodeparams.h"
#if defined(DMXNODE_TYPE_ARTNET)
#include "json/artnetparams.h"
#endif
#if defined(DMXNODE_TYPE_E131) || (defined(DMXNODE_TYPE_ARTNET) && (ARTNET_VERSION >= 4))
#include "json/e131params.h"
#endif

class DmxNodeNode final : public DmxNodeNodeType
{
   public:
    DmxNodeNode()
    {
        {
            json::DmxNodeParams dmxnode_params;
            dmxnode_params.Load();
            dmxnode_params.Set();
        }
#if defined(DMXNODE_TYPE_ARTNET)
        {
            json::ArtNetParams artnet_params;
            artnet_params.Load();
            artnet_params.Set();
        }
#endif
#if defined(DMXNODE_TYPE_E131) || (defined(DMXNODE_TYPE_ARTNET) && (ARTNET_VERSION >= 4))
        {
            json::E131Params e131_params;
            e131_params.Load();
            e131_params.Set();
        }
#endif
    }
};

#endif  // DMXNODENODE_H_
