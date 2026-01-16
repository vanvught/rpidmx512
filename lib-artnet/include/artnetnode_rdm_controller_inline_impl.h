/**
 * @file artnetnode_rdm_controller_inline_impl.h
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

#ifndef ARTNETNODE_RDM_CONTROLLER_INLINE_IMPL_H_
#define ARTNETNODE_RDM_CONTROLLER_INLINE_IMPL_H_

#include <cstdint>

#include "artnetnode.h"

inline uint32_t ArtNetNode::RdmCopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size)
{
    
    return rdm_controller_.CopyWorkingQueue(out_buffer, out_buffer_size);
}

inline uint32_t ArtNetNode::RdmGetUidCount(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);
    
    return rdm_controller_.GetUidCount(port_index);
}

inline uint32_t ArtNetNode::RdmCopyTod(uint32_t port_index, char* out_buffer, uint32_t out_buffer_size)
{
    assert(port_index < dmxnode::kMaxPorts);
    
    return rdm_controller_.CopyTod(port_index, out_buffer, out_buffer_size);
}

inline bool ArtNetNode::RdmIsRunning(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);
    
    return rdm_controller_.IsRunning(port_index);
}

inline bool ArtNetNode::RdmIsRunning(uint32_t port_index, bool& is_incremental)
{
    assert(port_index < dmxnode::kMaxPorts);
    
    return rdm_controller_.IsRunning(port_index, is_incremental);
}

inline bool ArtNetNode::GetRdmDiscovery(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);
    
    return rdm_controller_.IsEnabledBackground(port_index);
}

#endif  // ARTNETNODE_RDM_CONTROLLER_INLINE_IMPL_H_
