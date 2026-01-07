/**
 * @file e131bridge_inline_impl.h
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

#ifndef E131BRIDGE_INLINE_IMPL_H_
#define E131BRIDGE_INLINE_IMPL_H_

#include <cstdint>

#include "dmxnode.h"
#include "e131bridge.h"
#include "dmxnode_outputtype.h"
#include "dmxnode_data.h"
#include "hal_millis.h"
#include "hal_statusled.h"
#include "network.h"

inline void E131Bridge::SetOutput(DmxNodeOutputType* dmx_node_output_type)
{
    dmxnode_output_type_ = dmx_node_output_type;
}

inline DmxNodeOutputType* E131Bridge::GetOutput() const
{
    return dmxnode_output_type_;
}

inline void E131Bridge::SetLongName(const char* long_name)
{
    if (long_name == nullptr)
    {
        GetLongNameDefault(node_name_);
    }
    else
    {
        strncpy(node_name_, long_name, dmxnode::kNodeNameLength - 1);
    }

    node_name_[dmxnode::kNodeNameLength - 1] = '\0';
}

inline const char* E131Bridge::GetLongName()
{
    return node_name_;
}

inline void E131Bridge::SetShortName(uint32_t port_index, const char* name)
{
    DmxNode::Instance().SetShortName(port_index, name);
};

inline const char* E131Bridge::GetShortName(uint32_t port_index) const
{
    return DmxNode::Instance().GetShortName(port_index);
}

inline bool E131Bridge::GetDisableMergeTimeout() const
{
    return state_.disable_merge_timeout;
}

inline dmxnode::FailSafe E131Bridge::GetFailSafe() const
{
    return state_.failsafe;
}

inline uint16_t E131Bridge::GetUniverse(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return bridge_.port[port_index].universe;
}

inline bool E131Bridge::GetUniverse(uint32_t port_index, uint16_t& universe, dmxnode::PortDirection port_direction) const
{
    assert(port_index < dmxnode::kMaxPorts);
    if (port_direction == dmxnode::PortDirection::kDisable)
    {
        return false;
    }
    universe = bridge_.port[port_index].universe;
    return bridge_.port[port_index].direction == port_direction;
}

inline dmxnode::PortDirection E131Bridge::GetDirection(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return bridge_.port[port_index].direction;
}

inline void E131Bridge::SetUniverse(uint32_t port_index, dmxnode::PortDirection port_direction, uint16_t universe)
{
    SetUniverse(port_index, universe);
    SetDirection(port_index, port_direction);
}

inline void E131Bridge::SetMergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode)
{
    assert(port_index < dmxnode::kMaxPorts);
    output_port_[port_index].merge_mode = merge_mode;
}

inline dmxnode::MergeMode E131Bridge::GetMergeMode(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return output_port_[port_index].merge_mode;
}

inline dmxnode::PortDirection E131Bridge::GetPortDirection(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return bridge_.port[port_index].direction;
}

inline bool E131Bridge::GetOutputPort(uint16_t universe, uint32_t& port_index)
{
    for (port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (bridge_.port[port_index].direction != dmxnode::PortDirection::kOutput)
        {
            continue;
        }
        if (bridge_.port[port_index].universe == universe)
        {
            return true;
        }
    }
    return false;
}

#if defined(OUTPUT_HAVE_STYLESWITCH)
inline void E131Bridge::SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style)
{
    assert(port_index < dmxnode::kMaxPorts);

    if (dmxnode_output_type_ != nullptr)
    {
        dmxnode_output_type_->SetOutputStyle(port_index, output_style);
        output_style = dmxnode_output_type_->GetOutputStyle(port_index);
    }

    output_port_[port_index].output_style = output_style;
}

inline dmxnode::OutputStyle E131Bridge::GetOutputStyle(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return output_port_[port_index].output_style;
}
#endif

inline void E131Bridge::SetPriority(uint32_t port_index, uint8_t priority)
{
    assert(port_index < dmxnode::kMaxPorts);
    if ((priority >= e131::priority::kLowest) && (priority <= e131::priority::kHighest))
    {
        input_port_[port_index].priority = priority;
    }
}

inline uint8_t E131Bridge::GetPriority(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return input_port_[port_index].priority;
}

inline uint32_t E131Bridge::GetActiveOutputPorts() const
{
    return state_.enabled_output_ports;
}

inline uint32_t E131Bridge::GetActiveInputPorts() const
{
    return state_.enabled_input_ports;
}

inline bool E131Bridge::IsTransmitting(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return output_port_[port_index].is_transmitting;
}

inline bool E131Bridge::IsMerging(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return output_port_[port_index].is_merging;
}

inline bool E131Bridge::IsStatusChanged()
{
    if (state_.is_changed)
    {
        state_.is_changed = false;
        return true;
    }

    return false;
}

inline void E131Bridge::SetEnableDataIndicator(bool enable)
{
    enable_data_indicator_ = enable;
}

inline bool E131Bridge::GetEnableDataIndicator() const
{
    return enable_data_indicator_;
}

inline void E131Bridge::SetDisableSynchronize(bool disable_synchronize)
{
    state_.disable_synchronize = disable_synchronize;
}

inline bool E131Bridge::GetDisableSynchronize() const
{
    return state_.disable_synchronize;
}

inline void E131Bridge::SetInputDisabled(uint32_t port_index, bool disable)
{
    assert(port_index < dmxnode::kMaxPorts);
    input_port_[port_index].is_disabled = disable;
}

inline bool E131Bridge::GetInputDisabled(uint32_t port_index) const
{
    return input_port_[port_index].is_disabled;
}

inline void E131Bridge::Clear(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);

    dmxnode::Data::Clear(port_index);
    dmxnode::DataOutput(dmxnode_output_type_, port_index);

    if ((bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput) && !output_port_[port_index].is_transmitting)
    {
        dmxnode_output_type_->Start(port_index);
        output_port_[port_index].is_transmitting = true;
    }

    state_.is_network_data_loss = false; // Force timeout
}

#if defined(E131_HAVE_DMXIN) || defined(NODE_SHOWFILE)
inline void E131Bridge::SetSourceName(const char* source_name)
{
    assert(source_name != nullptr);
    strncpy(source_name_, source_name, e131::kSourceNameLength - 1);
    source_name_[e131::kSourceNameLength - 1] = '\0';
}

inline const char* E131Bridge::GetSourceName() const
{
    return source_name_;
}

inline const uint8_t* E131Bridge::GetCid() const
{
    return cid_;
}
#endif

#if defined(NODE_SHOWFILE) && defined(CONFIG_SHOWFILE_PROTOCOL_NODE_E131)
inline void E131Bridge::HandleShowFile(const e131::DataPacket* pE131DataPacket)
{
    packet_millis_ = hal::Millis();
    ip_address_from_ = network::GetPrimaryIp();
    receive_buffer_ = reinterpret_cast<uint8_t*>(const_cast<e131::DataPacket*>(pE131DataPacket));
    HandleDmx();
}

#endif

inline void E131Bridge::Run()
{
#if defined(E131_HAVE_DMXIN)
    HandleDmxIn();
#endif

    current_millis_ = hal::Millis();
    const auto kDeltaMillis = current_millis_ - packet_millis_;

    if (state_.enabled_output_ports != 0)
    {
        if (kDeltaMillis >= static_cast<uint32_t>(e131::kNetworkDataLossTimeoutSeconds * 1000))
        {
            if ((dmxnode_output_type_ != nullptr) && (!state_.is_network_data_loss))
            {
                SetNetworkDataLossCondition();
            }
        }

        if (kDeltaMillis >= 1000U)
        {
            state_.receiving_dmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(dmxnode::PortDirection::kOutput)));
        }
    }

    // The hal::statusled::Mode::FAST is for RDM Identify (Art-Net 4)
    if (enable_data_indicator_ && (hal::statusled::GetMode() != hal::statusled::Mode::FAST))
    {
        if (state_.receiving_dmx != 0)
        {
            hal::statusled::SetMode(hal::statusled::Mode::DATA);
        }
        else
        {
            hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
        }
    }
}

#endif // E131BRIDGE_INLINE_IMPL_H_
