/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cstring>
#include <uuid/uuid.h>
#include <cassert>

#include "e131.h"
#include "e131bridge.h"
#include "e131const.h"
#include "e117.h"
#if defined(E131_HAVE_DMXIN)
#include "dmx.h"
#endif
#include "dmxnode.h"
#include "dmxnodedata.h"
#include "dmxnode_data.h"
#include "hal_uuid.h"
#include "hal_boardinfo.h"
#include "network.h"
#include "core/ip4/igmp.h"
#include "softwaretimers.h"
#include "hal_millis.h"
#include "hal_statusled.h"
#include "hal_panelled.h"
#include "hal_statusled.h"
#include "hal.h"
#include "firmware/debug/debug_debug.h"

E131Bridge::E131Bridge()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    memset(&bridge_, 0, sizeof(struct e131bridge::Bridge));

    for (auto& port : bridge_.port)
    {
        port.direction = dmxnode::PortDirection::kDisable;
    }

    memset(&state_, 0, sizeof(e131bridge::State));
    state_.priority = e131::priority::kLowest;
    state_.failsafe = dmxnode::FailSafe::kHold;

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        memset(&output_port_[i], 0, sizeof(e131bridge::OutputPort));
        memset(&input_port_[i], 0, sizeof(e131bridge::InputPort));
        input_port_[i].priority = 100;
    }

#if defined(E131_HAVE_DMXIN) || defined(NODE_SHOWFILE)
    char source_name[e131::kSourceNameLength];
    uint8_t length;
    snprintf(source_name, e131::kSourceNameLength, "%.48s %s", network::iface::HostName(), hal::BoardName(length));
    SetSourceName(source_name);

    hal::UuidCopy(cid_);
#endif

    handle_ = network::udp::Begin(e131::kUdpPort, E131Bridge::StaticCallbackFunctionUdp);
    assert(handle_ != -1);

    SetLongName(nullptr); // Set default long name

    DEBUG_EXIT();
}

void E131Bridge::GetLongNameDefault(char* long_name)
{
#if !defined(E131_LONG_NAME)
    uint8_t boardname_length;
    const auto* const kBoardName = hal::BoardName(boardname_length);
    snprintf(long_name, dmxnode::kNodeNameLength - 1, "%s sACN E1.31 %s", kBoardName, hal::kWebsite);
#else
    uint32_t i;

    for (i = 0; i < (sizeof(E131_LONG_NAME) - 1) && i < (dmxnode::kNodeNameLength - 1); i++)
    {
        if (E131_LONG_NAME[i] == '_')
        {
            long_name[i] = ' ';
        }
        else
        {
            long_name[i] = E131_LONG_NAME[i];
        }
    }

    long_name[i] = '\0';
#endif
}

void E131Bridge::Start()
{
#if defined(E131_HAVE_DMXIN)
    const auto kIpMulticast = network::ConvertToUint(239, 255, 0, 0);
    discovery_ip_address_ = kIpMulticast | ((e131::universe::kDiscovery & static_cast<uint32_t>(0xFF)) << 24) | ((e131::universe::kDiscovery & 0xFF00) << 8);
    FillDataPacket();
    FillDiscoveryPacket();

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kInput, true);
        }
    }

    SetLocalMerging();

    timer_handle_send_discovery_packet_ = SoftwareTimerAdd(e131::kUniverseDiscoveryIntervalSeconds * 1000U, StaticCallbackFunctionSendDiscoveryPacket);
    assert(timer_handle_send_discovery_packet_ >= 0);
#endif

#if defined(OUTPUT_HAVE_STYLESWITCH)
    // Make sure that the supported OutputSyle is correctly set
    if (dmxnode_output_type_ != nullptr)
    {
        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput)
            {
                SetOutputStyle(port_index, GetOutputStyle(port_index));
            }
        }
    }
#endif

    state_.status = e131bridge::Status::kOn;
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
}

void E131Bridge::Stop()
{
    state_.is_network_data_loss = true;

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (dmxnode_output_type_ != nullptr)
        {
            dmxnode_output_type_->Stop(port_index);
        }
        dmxnode::Data::ClearLength(port_index);
    }

#if defined(E131_HAVE_DMXIN)
    SoftwareTimerDelete(timer_handle_send_discovery_packet_);

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kInput, false);
        }
    }
#endif

    state_.status = e131bridge::Status::kOff;
    hal::statusled::SetMode(hal::statusled::Mode::OFF_OFF);
}

void E131Bridge::SetSynchronizationAddress(bool source_a, bool source_b, uint16_t synchronization_address)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("source_a=%d, source_b=%d, synchronization_address=%d", source_a, source_b, synchronization_address);

    assert(synchronization_address != 0);

    uint16_t* synchronization_address_source;

    if (source_a)
    {
        synchronization_address_source = &state_.synchronization_address_source_a;
    }
    else if (source_b)
    {
        synchronization_address_source = &state_.synchronization_address_source_b;
    }
    else
    {
        DEBUG_EXIT();
        return; // Just make the compiler happy
    }

    if (*synchronization_address_source == 0)
    {
        *synchronization_address_source = synchronization_address;
        DEBUG_PUTS("synchronization_address_source == 0");
    }
    else if (*synchronization_address_source != synchronization_address)
    {
        // dmxnode::kMaxPorts forces to check all ports
        LeaveUniverse(dmxnode::kMaxPorts, *synchronization_address_source);
        *synchronization_address_source = synchronization_address;
        DEBUG_PUTS("synchronization_address_source != synchronization_address");
    }
    else
    {
        DEBUG_PUTS("Already received synchronization_address");
        DEBUG_EXIT();
        return;
    }

    network::igmp::JoinGroup(handle_, e131::UniverseToMulticastIp(synchronization_address));

    DEBUG_EXIT();
}

void E131Bridge::JoinUniverse(uint32_t port_index, uint16_t universe)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%d, universe=%d", port_index, universe);

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        DEBUG_PRINTF("\toutput_Port[%d].universe=%d", i, bridge_.port[i].universe);

        if (i == port_index) // Check for other ports only
        {
            DEBUG_PUTS("continue");
            continue;
        }

        if (bridge_.port[i].universe == universe)
        {
            DEBUG_EXIT();
            return;
        }
    }

    DEBUG_PUTS("Join");
    network::igmp::JoinGroup(handle_, e131::UniverseToMulticastIp(universe));

    DEBUG_EXIT();
}

void E131Bridge::LeaveUniverse(uint32_t port_index, uint16_t universe)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%d, universe=%d", port_index, universe);

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        DEBUG_PRINTF("\toutput_Port[%d].universe=%d", i, bridge_.port[i].universe);

        if (i == port_index) // Check for other ports only
        {
            DEBUG_PUTS("continue");
            continue;
        }

        if (bridge_.port[i].universe == universe)
        {
            DEBUG_EXIT();
            return;
        }
    }

    DEBUG_PUTS("Leave");
    network::igmp::LeaveGroup(handle_, e131::UniverseToMulticastIp(universe));

    DEBUG_EXIT();
}

void E131Bridge::SetLocalMerging()
{
    DEBUG_ENTRY();

    for (uint32_t input_port_index = 0; input_port_index < dmxnode::kMaxPorts; input_port_index++)
    {
        if ((bridge_.port[input_port_index].direction == dmxnode::PortDirection::kOutput) || (bridge_.port[input_port_index].universe == 0))
        {
            continue;
        }

        bridge_.port[input_port_index].local_merge = false;

        for (uint32_t output_port_index = 0; output_port_index < dmxnode::kMaxPorts; output_port_index++)
        {
            if (bridge_.port[output_port_index].direction == dmxnode::PortDirection::kInput)
            {
                continue;
            }

            DEBUG_PRINTF("input_port_index=%u %u, output_port_index=%u %u", input_port_index, bridge_.port[input_port_index].universe, output_port_index,
                         bridge_.port[output_port_index].universe);

            if (bridge_.port[input_port_index].universe == bridge_.port[output_port_index].universe)
            {
                if (!bridge_.port[output_port_index].local_merge)
                {
                    output_port_[output_port_index].source_a.ip = network::kIpaddrLoopback;
                    DEBUG_PUTS("Local merge Source A");
                }
                else
                {
                    output_port_[output_port_index].source_b.ip = network::kIpaddrLoopback;
                    DEBUG_PUTS("Local merge Source B");
                }

                DEBUG_PUTS("");
                bridge_.port[input_port_index].local_merge = true;
                bridge_.port[output_port_index].local_merge = true;
            }
        }
    }

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        DEBUG_PRINTF("port_index=%u, local_merge=%c", port_index, bridge_.port[port_index].local_merge ? 'Y' : 'N');
    }

    DEBUG_EXIT();
}

void E131Bridge::SetUniverse(uint32_t port_index, uint16_t universe)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, universe=%u", port_index, universe);

    assert(port_index < dmxnode::kMaxPorts);
    assert((universe >= e131::universe::kDefault) && (universe <= e131::universe::kMax));

    if (bridge_.port[port_index].universe == universe)
    {
        DEBUG_EXIT();
        return;
    }

    if (bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput)
    {
        LeaveUniverse(port_index, bridge_.port[port_index].universe);
        JoinUniverse(port_index, universe);
    }

    bridge_.port[port_index].universe = universe;
    input_port_[port_index].multicast_ip = e131::UniverseToMulticastIp(universe);

#if defined(E131_HAVE_DMXIN)
    if (state_.status == e131bridge::Status::kOn)
    {
        SetLocalMerging();
    }
#endif

    DEBUG_EXIT();
}

void E131Bridge::SetDirection(uint32_t port_index, dmxnode::PortDirection port_direction)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, port_direction=%s", port_index, dmxnode::GetPortDirection(port_direction));

    assert(port_index < dmxnode::kMaxPorts);
    assert(port_direction <= dmxnode::PortDirection::kDisable);

    if (bridge_.port[port_index].direction == port_direction)
    {
        DEBUG_EXIT();
        return;
    }

    if (port_direction == dmxnode::PortDirection::kDisable)
    {
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            assert(state_.enabled_output_ports >= 1);
            state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports - 1);

            LeaveUniverse(port_index, bridge_.port[port_index].universe);
        }
#if defined(E131_HAVE_DMXIN)
        else if (bridge_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            assert(state_.enabled_input_ports > 1);
            state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports - 1);
        }
#endif

        bridge_.port[port_index].direction = dmxnode::PortDirection::kDisable;
    }
#if defined(E131_HAVE_DMXIN)
    else if (port_direction == dmxnode::PortDirection::kInput)
    {
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            assert(state_.enabled_output_ports >= 1);
            state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports - 1);
            LeaveUniverse(port_index, bridge_.port[port_index].universe);
        }

        state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports + 1);
        assert(state_.enabled_input_ports <= dmxnode::kMaxPorts);

        bridge_.port[port_index].direction = dmxnode::PortDirection::kInput;
    }
#endif
    else if (port_direction == dmxnode::PortDirection::kOutput)
    {
#if defined(E131_HAVE_DMXIN)
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            assert(state_.enabled_input_ports >= 1);
            state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports - 1);
        }
#endif

        state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports + 1);
        assert(state_.enabled_output_ports <= dmxnode::kMaxPorts);

        JoinUniverse(port_index, bridge_.port[port_index].universe);

        bridge_.port[port_index].direction = dmxnode::PortDirection::kOutput;
    }

#if defined(E131_HAVE_DMXIN)
    if (state_.status == e131bridge::Status::kOn)
    {
        SetLocalMerging();
    }
#endif
}

#if defined(E131_HAVE_DMXIN)
static constexpr auto kUuidStringLength = 36;
#endif

void E131Bridge::Print()
{
#if defined(E131_HAVE_DMXIN)
    char uuid_str[kUuidStringLength + 1];
    uuid_str[kUuidStringLength] = '\0';
    uuid_unparse(cid_, uuid_str);
#endif
    printf("sACN E1.31 V%d.%d\n", E131Const::kVersion[0], E131Const::kVersion[1]);
#if defined(E131_HAVE_DMXIN)
    printf(" CID      : %s\n", uuid_str);
#endif
    if (state_.enabled_output_ports != 0)
    {
        printf(" Output\n");

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (GetPortDirection(port_index) == dmxnode::PortDirection::kOutput)
            {
                const auto kUniverse = GetUniverse(port_index);
                printf("  Port %-2u %-4u %s\n", static_cast<unsigned int>(port_index), static_cast<unsigned int>(kUniverse),
                       dmxnode::GetMergeMode(output_port_[port_index].merge_mode, true));
            }
        }
    }

#if defined(E131_HAVE_DMXIN)
    if (state_.enabled_input_ports != 0)
    {
        printf(" Input\n");

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (GetPortDirection(port_index) == dmxnode::PortDirection::kInput)
            {
                const auto kUniverse = GetUniverse(port_index);
                printf("  Port %-2u %-4u %-3u\n", static_cast<unsigned int>(port_index), static_cast<unsigned int>(kUniverse), GetPriority(port_index));
            }
        }
    }
#endif

    if (state_.disable_synchronize)
    {
        printf(" Synchronize is disabled\n");
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

static bool IsValidRoot(const uint8_t* buffer)
{
    const auto* const kRaw = reinterpret_cast<const e131::RawPacket*>(buffer);
    // 5 E1.31 use of the ACN Root Layer Protocol
    // Receivers shall discard the packet if the ACN Packet Identifier is not valid.
    if (memcmp(kRaw->root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength) != 0)
    {
        return false;
    }

    if (kRaw->root_layer.vector != __builtin_bswap32(e131::vector::root::kData) &&
        (kRaw->root_layer.vector != __builtin_bswap32(e131::vector::root::kExtended)))
    {
        return false;
    }

    return true;
}

static bool IsValidDataPacket(const uint8_t* buffer)
{
    const auto& data = *reinterpret_cast<const e131::DataPacket*>(buffer);
    // The DMP Layer's vector shall be set to 0x02, which indicates a DMP Set Property message by
    // transmitters. Receivers shall discard the packet if the received value is not 0x02.
    if (data.dmp_layer.vector != e131::vector::dmp::kSetProperty)
    {
        return false;
    }
    // Transmitters shall set the DMP Layer's Address Type and Data Type to 0xa1. Receivers shall discard the
    // packet if the received value is not 0xa1.
    if (data.dmp_layer.type != 0xa1)
    {
        return false;
    }
    // Transmitters shall set the DMP Layer's First Property Address to 0x0000. Receivers shall discard the
    // packet if the received value is not 0x0000.
    if (data.dmp_layer.first_address_property != __builtin_bswap16(0x0000))
    {
        return false;
    }
    // Transmitters shall set the DMP Layer's Address Increment to 0x0001. Receivers shall discard the packet if
    // the received value is not 0x0001.
    if (data.dmp_layer.address_increment != __builtin_bswap16(0x0001))
    {
        return false;
    }

    return true;
}

void E131Bridge::HandleSynchronization()
{
    // 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
    // Receivers may ignore Synchronization Packets sent to multicast addresses
    // which do not correspond to their Synchronization Address.
    //
    // NOTE: There is no multicast addresses (To Ip) available
    // We just check if synchronization_address is published by a Source

    const auto& synchronization_packet = *reinterpret_cast<const e131::SynchronizationPacket*>(receive_buffer_);
    const auto kSynchronizationAddress = __builtin_bswap16(synchronization_packet.frame_layer.universe_number);

    if ((kSynchronizationAddress != state_.synchronization_address_source_a) && (kSynchronizationAddress != state_.synchronization_address_source_b))
    {
        hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
        DEBUG_PUTS("");
        return;
    }

    state_.synchronization_time = packet_millis_;

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (output_port_[port_index].is_data_pending)
        {
            dmxnode_output_type_->Sync(port_index);
        }
    }

    dmxnode_output_type_->Sync();

    for (auto& output_port : output_port_)
    {
        if (output_port.is_data_pending)
        {
            output_port.is_data_pending = false;
            if (!output_port.is_transmitting)
            {
                output_port.is_transmitting = true;
                state_.is_changed = true;
            }
        }
    }

    if (sync_callback_function_pointer_ != nullptr)
    {
        sync_callback_function_pointer_();
    }
}

void E131Bridge::InputUdp(const uint8_t* buffer, [[maybe_unused]] uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((!IsValidRoot(buffer)), 0)) return;

    current_millis_ = hal::Millis();
    packet_millis_ = current_millis_;

    state_.is_network_data_loss = false;

    if (state_.is_synchronized && !state_.is_forced_synchronized)
    {
        if ((current_millis_ - state_.synchronization_time) >= static_cast<uint32_t>(e131::kNetworkDataLossTimeoutSeconds * 1000))
        {
            state_.is_synchronized = false;
        }
    }

    if (dmxnode_output_type_ != nullptr)
    {
        receive_buffer_ = const_cast<uint8_t*>(buffer);
        ip_address_from_ = from_ip;

        const auto* const kRaw = reinterpret_cast<const e131::RawPacket*>(buffer);
        const auto kRootVector = __builtin_bswap32(kRaw->root_layer.vector);

        if (kRootVector == e131::vector::root::kData)
        {
            if (__builtin_expect((IsValidDataPacket(buffer)), 1))
            {
                HandleDmx();
            }
        }
        else if (kRootVector == e131::vector::root::kExtended)
        {
            const auto kFramingVector = __builtin_bswap32(kRaw->frame_layer.vector);
            if (kFramingVector == e131::vector::extended::kSynchronization)
            {
                HandleSynchronization();
            }
        }
        else
        {
            DEBUG_PRINTF("Not supported Root vector : 0x%x", kRootVector);
        }
    }

    hal::panelled::On(hal::panelled::SACN);
}

void E131Bridge::UpdateMergeStatus(uint32_t port_index)
{
    if (!state_.is_merge_mode)
    {
        state_.is_merge_mode = true;
        state_.is_changed = true;
    }

    output_port_[port_index].is_merging = true;
}

void E131Bridge::CheckMergeTimeouts(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);

    const auto kTimeOutA = current_millis_ - output_port_[port_index].source_a.millis;

    if (kTimeOutA > (e131::kMergeTimeoutSeconds * 1000U))
    {
        output_port_[port_index].source_a.ip = 0;
        memset(output_port_[port_index].source_a.cid, 0, e117::kCidLength);
        output_port_[port_index].is_merging = false;
    }

    const auto kTimeOutB = current_millis_ - output_port_[port_index].source_b.millis;

    if (kTimeOutB > (e131::kMergeTimeoutSeconds * 1000U))
    {
        output_port_[port_index].source_b.ip = 0;
        memset(output_port_[port_index].source_b.cid, 0, e117::kCidLength);
        output_port_[port_index].is_merging = false;
    }

    auto is_merging = false;

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        is_merging |= output_port_[i].is_merging;
    }

    if (!is_merging)
    {
        state_.is_changed = true;
        state_.is_merge_mode = false;
    }
}

bool E131Bridge::IsPriorityTimeOut(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);

    const auto kTimeOutA = current_millis_ - output_port_[port_index].source_a.millis;
    const auto kTimeOutB = current_millis_ - output_port_[port_index].source_b.millis;

    if ((output_port_[port_index].source_a.ip != 0) && (output_port_[port_index].source_b.ip != 0))
    {
        if ((kTimeOutA < (e131::kPriorityTimeoutSeconds * 1000U)) || (kTimeOutB < (e131::kPriorityTimeoutSeconds * 1000U)))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else if ((output_port_[port_index].source_a.ip != 0) && (output_port_[port_index].source_b.ip == 0))
    {
        if (kTimeOutA > (e131::kPriorityTimeoutSeconds * 1000U))
        {
            return true;
        }
    }
    else if ((output_port_[port_index].source_a.ip == 0) && (output_port_[port_index].source_b.ip != 0))
    {
        if (kTimeOutB > (e131::kPriorityTimeoutSeconds * 1000U))
        {
            return true;
        }
    }

    return false;
}

bool E131Bridge::IsIpCidMatch(const e131bridge::Source* const kSource) const
{
    if (kSource->ip != ip_address_from_)
    {
        return false;
    }

    const auto& raw = *reinterpret_cast<const e131::RawPacket*>(receive_buffer_);

    if (memcmp(kSource->cid, raw.root_layer.cid, e117::kCidLength) != 0)
    {
        return false;
    }

    return true;
}

void E131Bridge::HandleDmx()
{
    const auto& data = *reinterpret_cast<const e131::DataPacket*>(receive_buffer_);
    const auto* const kDmxData = &data.dmp_layer.property_values[1];
    const auto kDmxSlots = __builtin_bswap16(data.dmp_layer.property_value_count) - 1U;

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (bridge_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            // Frame layer
            // 8.2 Association of Multicast Addresses and Universe
            // Note: The identity of the universe shall be determined by the universe number in the
            // packet and not assumed from the multicast address.
            if (data.frame_layer.universe != __builtin_bswap16(bridge_.port[port_index].universe))
            {
                continue;
            }

            auto& source_a = output_port_[port_index].source_a;
            auto& source_b = output_port_[port_index].source_b;

            const auto kIpA = source_a.ip;
            const auto kIpB = source_b.ip;

            const auto kIsSourceA = IsIpCidMatch(&source_a);
            const auto kIsSourceB = IsIpCidMatch(&source_b);

            // 6.9.2 Sequence Numbering
            // Having first received a packet with sequence number A, a second packet with sequence number B
            // arrives. If, using signed 8-bit binary arithmetic, B – A is less than or equal to 0, but greater than -20 then
            // the packet containing sequence number B shall be deemed out of sequence and discarded
            if (kIsSourceA)
            {
                const auto kDiff = static_cast<int8_t>(data.frame_layer.sequence_number - source_a.sequence_number_data);
                source_a.sequence_number_data = data.frame_layer.sequence_number;
                if ((kDiff <= 0) && (kDiff > -20))
                {
                    continue;
                }
            }
            else if (kIsSourceB)
            {
                const auto kDiff = static_cast<int8_t>(data.frame_layer.sequence_number - source_b.sequence_number_data);
                source_b.sequence_number_data = data.frame_layer.sequence_number;
                if ((kDiff <= 0) && (kDiff > -20))
                {
                    continue;
                }
            }

            // This bit, when set to 1, indicates that the data in this packet is intended for use in visualization or media
            // server preview applications and shall not be used to generate live output.
            if (e131::OptionsMask::Has(data.frame_layer.options, e131::OptionsMask::Mask::kPreviewData))
            {
                continue;
            }

            // Upon receipt of a packet containing this bit set to a value of 1, receiver shall enter network data loss condition.
            // Any property values in these packets shall be ignored.
            if (e131::OptionsMask::Has(data.frame_layer.options, e131::OptionsMask::Mask::kStreamTerminated))
            {
                if (kIsSourceA || kIsSourceB)
                {
                    SetNetworkDataLossCondition(kIsSourceA, kIsSourceB);
                }
                continue;
            }

            if (state_.is_merge_mode)
            {
                if (__builtin_expect((!state_.disable_merge_timeout), 1))
                {
                    CheckMergeTimeouts(port_index);
                }
            }

            if (data.frame_layer.priority < state_.priority)
            {
                if (!IsPriorityTimeOut(port_index))
                {
                    continue;
                }
                state_.priority = data.frame_layer.priority;
            }
            else if (data.frame_layer.priority > state_.priority)
            {
                output_port_[port_index].source_a.ip = 0;
                output_port_[port_index].source_b.ip = 0;
                state_.is_merge_mode = false;
                state_.priority = data.frame_layer.priority;
            }

            if ((kIpA == 0) && (kIpB == 0))
            {
                // printf("1. First package from Source\n");
                source_a.ip = ip_address_from_;
                source_a.sequence_number_data = data.frame_layer.sequence_number;
                memcpy(source_a.cid, data.root_layer.cid, 16);
                source_a.millis = packet_millis_;
                dmxnode::Data::SetSourceA(port_index, kDmxData, kDmxSlots);
            }
            else if (kIsSourceA && (kIpB == 0))
            {
                // printf("2. Continue package from SourceA\n");
                source_a.sequence_number_data = data.frame_layer.sequence_number;
                source_a.millis = packet_millis_;
                dmxnode::Data::SetSourceA(port_index, kDmxData, kDmxSlots);
            }
            else if ((kIpA == 0) && kIsSourceB)
            {
                // printf("3. Continue package from source_b\n");
                source_b.sequence_number_data = data.frame_layer.sequence_number;
                source_b.millis = packet_millis_;
                dmxnode::Data::SetSourceB(port_index, kDmxData, kDmxSlots);
            }
            else if (!kIsSourceA && (kIpB == 0))
            {
                // printf("4. New ip, start merging\n");
                source_b.ip = ip_address_from_;
                source_b.sequence_number_data = data.frame_layer.sequence_number;
                memcpy(source_b.cid, data.root_layer.cid, 16);
                source_b.millis = packet_millis_;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceB(port_index, kDmxData, kDmxSlots, output_port_[port_index].merge_mode);
            }
            else if ((kIpA == 0) && !kIsSourceB)
            {
                // printf("5. New ip, start merging\n");
                source_a.ip = ip_address_from_;
                source_a.sequence_number_data = data.frame_layer.sequence_number;
                memcpy(source_a.cid, data.root_layer.cid, 16);
                source_a.millis = packet_millis_;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceA(port_index, kDmxData, kDmxSlots, output_port_[port_index].merge_mode);
            }
            else if (kIsSourceA && !kIsSourceB)
            {
                // printf("6. Continue merging\n");
                source_a.sequence_number_data = data.frame_layer.sequence_number;
                source_a.millis = packet_millis_;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceA(port_index, kDmxData, kDmxSlots, output_port_[port_index].merge_mode);
            }
            else if (!kIsSourceA && kIsSourceB)
            {
                // printf("7. Continue merging\n");
                source_b.sequence_number_data = data.frame_layer.sequence_number;
                source_b.millis = packet_millis_;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceB(port_index, kDmxData, kDmxSlots, output_port_[port_index].merge_mode);
            }
#ifndef NDEBUG
            else if (kIsSourceA && kIsSourceB)
            {
                puts("WARN: 8. Source matches both ip, discarding data");
                return;
            }
            else if (!kIsSourceA && !kIsSourceB)
            {
                puts("WARN: 9. More than two sources, discarding data");
                return;
            }
            else
            {
                puts("ERROR: 0. No cases matched, this shouldn't happen!");
                return;
            }
#endif
            // This bit indicates whether to lock or revert to an unsynchronized state when synchronization is lost
            // (See Section 11 on Universe Synchronization and 11.1 for discussion on synchronization states).
            // When set to 0, components that had been operating in a synchronized state shall not update with any
            // new packets until synchronization resumes. When set to 1, once synchronization has been lost,
            // components that had been operating in a synchronized state need not wait for a new
            // E1.31 Synchronization Packet in order to update to the next E1.31 Data Packet.

            // If the FORCE_SYNCHRONIZATION bit is 0, the receiver MUST wait for synchronization packets.
            // If it is 1, the receiver MAY update without waiting for synchronization packets.
            if (!e131::OptionsMask::Has(data.frame_layer.options, e131::OptionsMask::Mask::kForceSynchronization))
            {
                // 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
                // An E1.31 Synchronization Packet is sent to synchronize the E1.31 data on a specific universe number.
                // A Synchronization Address of 0 is thus meaningless, and shall not be transmitted.
                // Receivers shall ignore E1.31 Synchronization Packets containing a Synchronization Address of 0.

                // Synchronization is required: enter synchronized state (until sync is lost or overridden)
                if (data.frame_layer.synchronization_address != 0)
                {
                    if (!state_.is_forced_synchronized)
                    {
                        // Decide which source triggered the sync request
                        if (!(kIsSourceA || kIsSourceB))
                        {
                            SetSynchronizationAddress((source_a.ip != 0), (source_b.ip != 0), __builtin_bswap16(data.frame_layer.synchronization_address));
                        }
                        else
                        {
                            SetSynchronizationAddress(kIsSourceA, kIsSourceB, __builtin_bswap16(data.frame_layer.synchronization_address));
                        }
                        state_.is_forced_synchronized = true;
                        state_.is_synchronized = true;
                    }
                }
            }
            else
            {
                // Synchronization not required — allow unsynchronized updates
                state_.is_forced_synchronized = false;
            }

            const auto kDoUpdate = ((!state_.is_synchronized) || (state_.disable_synchronize));

            if (kDoUpdate)
            {
                dmxnode::DataOutput(dmxnode_output_type_, port_index);

                if (!output_port_[port_index].is_transmitting)
                {
                    dmxnode_output_type_->Start(port_index);
                    output_port_[port_index].is_transmitting = true;
                    state_.is_changed = true;
                }
            }
            else
            {
                dmxnode::DataSet(dmxnode_output_type_, port_index);
                output_port_[port_index].is_data_pending = true;
            }

            state_.receiving_dmx |= (1U << static_cast<uint8_t>(dmxnode::PortDirection::kOutput));
        }
    }
}

void E131Bridge::SetNetworkDataLossCondition(bool source_a, bool source_b)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("%d %d", source_a, source_b);

    state_.is_changed = true;
    auto do_failsafe = false;

    if (source_a && source_b)
    {
        state_.is_network_data_loss = true;
        state_.is_merge_mode = false;
        state_.is_synchronized = false;
        state_.is_forced_synchronized = false;
        state_.priority = e131::priority::kLowest;

        for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
        {
            if (output_port_[i].is_transmitting)
            {
                do_failsafe = true;
                output_port_[i].source_a.ip = 0;
                memset(output_port_[i].source_a.cid, 0, e117::kCidLength);
                output_port_[i].source_b.ip = 0;
                memset(output_port_[i].source_b.cid, 0, e117::kCidLength);
                dmxnode::Data::ClearLength(i);
                output_port_[i].is_transmitting = false;
                output_port_[i].is_merging = false;
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
        {
            if (output_port_[i].is_transmitting)
            {
                if ((source_a) && (output_port_[i].source_a.ip != 0))
                {
                    output_port_[i].source_a.ip = 0;
                    memset(output_port_[i].source_a.cid, 0, e117::kCidLength);
                    output_port_[i].is_merging = false;
                }

                if ((source_b) && (output_port_[i].source_b.ip != 0))
                {
                    output_port_[i].source_b.ip = 0;
                    memset(output_port_[i].source_b.cid, 0, e117::kCidLength);
                    output_port_[i].is_merging = false;
                }

                if (!state_.is_merge_mode)
                {
                    do_failsafe = true;
                    dmxnode::Data::ClearLength(i);
                    output_port_[i].is_transmitting = false;
                }
            }
        }
    }

    if (do_failsafe)
    {
        switch (state_.failsafe)
        {
            case dmxnode::FailSafe::kHold:
                break;
            case dmxnode::FailSafe::kOff:
                dmxnode_output_type_->Blackout(true);
                break;
            case dmxnode::FailSafe::kOn:
                dmxnode_output_type_->FullOn();
                break;
            default:
                DEBUG_PRINTF("state_.failsafe=%u", static_cast<uint32_t>(state_.failsafe));
                assert(false && "Invalid state_.failsafe");
                break;
        }
    }

    state_.receiving_dmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(dmxnode::PortDirection::kOutput)));

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    hal::panelled::Off(hal::panelled::SACN);

#if defined(E131_HAVE_DMXIN)
    SetLocalMerging();
#endif

    DEBUG_EXIT();
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif