/**
 * @file dmxnode_scenes.cpp
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

#include "dmxnode.h"
#include "dmxnodedata.h"
#include "dmxnode_nodetype.h"

void DmxNode::SceneStore()
{
    dmxnode::scenes::WriteStart();

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& port = port_[port_index];

        if (port.port_direction == dmxnode::PortDirection::kOutput)
        {
            dmxnode::scenes::Write(port_index, dmxnode::Data::Backup(port_index));
        }
    }

    dmxnode::scenes::WriteEnd();
}

void DmxNode::ScenePlayback()
{
    dmxnode::scenes::ReadStart();

    auto dmxnode_output_type = DmxNodeNodeType::Get()->GetOutput();

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& port = port_[port_index];

        if (port.port_direction == dmxnode::PortDirection::kOutput)
        {
            dmxnode::scenes::Read(port_index, const_cast<uint8_t*>(dmxnode::Data::Backup(port_index)));
            dmxnode::DataOutput(dmxnode_output_type, port_index);

            if (!port.is_transmitting)
            {
                dmxnode_output_type->Start(port_index);
                port.is_transmitting = true;
            }

            dmxnode::Data::ClearLength(port_index);
        }
    }

    dmxnode::scenes::ReadEnd();
}
