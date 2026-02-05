/**
 * @file artnetnode.cpp
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
#include <ctime>
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"
#include "artnet.h"
#include "artnetdisplay.h"
#include "artnetstore.h"
#include "common/utils/utils_enum.h"
#if defined(ARTNET_HAVE_TRIGGER)
#include "artnettrigger.h"
#endif
#if defined(ARTNET_HAVE_DMXIN)
#include "dmx.h"
#endif
#if (ARTNET_VERSION >= 4)
#include "e131.h"
#endif
#include "dmxnode.h"
#include "dmxnode_data.h"
#include "hal_boardinfo.h"
#include "network.h"
#include "hal.h"
#include "hal_statusled.h"
#include "hal_millis.h"
#include "hal_rtc.h"
#include "firmware/debug/debug_debug.h"

#if defined(ARTNET_SHOWFILE)
namespace showfile
{
void record(const struct artnet::ArtDmx* artdmx, uint32_t millis);
void record(const struct artnet::ArtSync* artsync, uint32_t millis);
} // namespace showfile
#endif

static constexpr auto kArtnetMinHeaderSize = 12;

ArtNetNode::ArtNetNode()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    DEBUG_PRINTF("MAX_PORTS=%u", dmxnode::kMaxPorts);

    memset(&art_poll_reply_, 0, sizeof(struct artnet::ArtPollReply));
    memcpy(art_poll_reply_.Id, artnet::kNodeId, sizeof(art_poll_reply_.Id));
    art_poll_reply_.OpCode = common::ToValue(artnet::OpCodes::kOpPollreply);
    art_poll_reply_.Port = artnet::kUdpPort;
    art_poll_reply_.VersInfoH = ArtNetConst::kVersion[0];
    art_poll_reply_.VersInfoL = ArtNetConst::kVersion[1];
    art_poll_reply_.OemHi = ArtNetConst::kOemId[0];
    art_poll_reply_.Oem = ArtNetConst::kOemId[1];
    art_poll_reply_.EstaMan[0] = ArtNetConst::kEstaId[1];
    art_poll_reply_.EstaMan[1] = ArtNetConst::kEstaId[0];
    network::iface::CopyMacAddressTo(art_poll_reply_.MAC);
#if (ARTNET_VERSION >= 4)
    art_poll_reply_.AcnPriority = e131::priority::kDefault;
#endif

    memset(&state_, 0, sizeof(struct artnetnode::State));
    state_.report_code = artnet::ReportCode::kRcpowerok;
    state_.status = artnet::Status::kStandby;
    // The device should wait for a random delay of up to 1s before sending the reply.
    state_.art.poll_reply_delay_millis = (art_poll_reply_.MAC[5] | (static_cast<uint32_t>(art_poll_reply_.MAC[4]) << 8)) % 1000;

    SetLongName(nullptr); // Set default long name

    memset(&node_, 0, sizeof(struct artnetnode::Node));
    node_.ip_timecode = network::GetBroadcastIp();

    for (auto& port : node_.port)
    {
        port.direction = dmxnode::PortDirection::kDisable;
    }

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        SetShortName(port_index, nullptr); // Set default port label
    }

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        memset(&output_port_[port_index], 0, sizeof(struct artnetnode::OutputPort));
        output_port_[port_index].source_a.physical = 0x100;
        output_port_[port_index].source_b.physical = 0x100;
        output_port_[port_index].good_output_b = artnet::GoodOutputB::kRdmDisabled | artnet::GoodOutputB::kDiscoveryNotRunning;
        memset(&input_port_[port_index], 0, sizeof(struct artnetnode::InputPort));
        input_port_[port_index].destination_ip = network::GetBroadcastIp();
    }

#if defined(ARTNET_HAVE_DMXIN)
    memcpy(art_dmx_.Id, artnet::kNodeId, sizeof(art_poll_reply_.Id));
    art_dmx_.OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpDmx);
    art_dmx_.ProtVerHi = 0;
    art_dmx_.ProtVerLo = artnet::kProtocolRevision;
#endif

#if defined(ARTNET_HAVE_TIMECODE)
    memcpy(art_time_code_.Id, artnet::kNodeId, sizeof(art_poll_reply_.Id));
    art_time_code_.OpCode = common::ToValue(artnet::OpCodes::kOpTimecode);
    art_time_code_.ProtVerHi = 0;
    art_time_code_.ProtVerLo = artnet::kProtocolRevision;
    art_time_code_.filler1 = 0;
    art_time_code_.filler2 = 0;
#endif

#if defined(ARTNET_ENABLE_SENDDIAG)
    memset(&diag_data_, 0, sizeof(struct artnet::ArtDiagData));
    memcpy(diag_data_.Id, artnet::kNodeId, sizeof(diag_data_.Id));
    diag_data_.OpCode = common::ToValue(artnet::OpCodes::kOpDiagdata);
    diag_data_.ProtVerLo = artnet::kProtocolRevision;
#endif

    DEBUG_EXIT();
}

void ArtNetNode::Start()
{
    DEBUG_ENTRY();

#if defined(ARTNET_HAVE_TRIGGER)
    assert(art_trigger_callback_function_ptr_ != nullptr);
#endif

#if defined(ARTNET_HAVE_TIMECODE)
    assert(art_time_code_callback_function_ptr_ != nullptr);
#endif

    // Status 1
    art_poll_reply_.Status1 |= artnet::Status1::kIndicatorNormalMode | artnet::Status1::kPapNetwork;
    //  Status 2
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kSacnAbleToSwitch);
    art_poll_reply_.Status2 |=
        artnet::Status2::kPortAddress15Bit | (artnet::kVersion >= 4 ? artnet::Status2::kSacnAbleToSwitch : artnet::Status2::kSacnNoSwitch);
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kIpDhcp);
    art_poll_reply_.Status2 |= network::iface::Dhcp() ? artnet::Status2::kIpDhcp : artnet::Status2::kIpManualy;
    art_poll_reply_.Status2 &= static_cast<uint8_t>(~artnet::Status2::kDhcpCapable);
    art_poll_reply_.Status2 |= network::iface::IsDhcpCapable() ? artnet::Status2::kDhcpCapable : static_cast<uint8_t>(0);
#if defined(ENABLE_HTTPD) && defined(ENABLE_CONTENT)
    art_poll_reply_.Status2 |= artnet::Status2::kWebBrowserSupport;
#endif
#if defined(OUTPUT_HAVE_STYLESWITCH)
    art_poll_reply_.Status2 |= artnet::Status2::kOutputStyleSwitch;
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
    art_poll_reply_.Status2 |= artnet::Status2::kRdmSwitch;
#endif
    // Status 3
    art_poll_reply_.Status3 |= artnet::Status3::kFailsafeControl | artnet::Status3::kSupportsBackgrounddiscovery;
#if defined(ARTNET_HAVE_DMXIN)
    art_poll_reply_.Status3 |= artnet::Status3::kOutputSwitch;
#endif

    handle_ = network::udp::Begin(artnet::kUdpPort, StaticCallbackFunction);
    assert(handle_ != -1);

#if defined(ARTNET_HAVE_DMXIN)
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if ((node_.port[port_index].protocol == artnet::PortProtocol::kArtnet) && (node_.port[port_index].direction == dmxnode::PortDirection::kInput))
        {
            Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kInput, true);
        }
    }

    SetLocalMerging();
#endif

#if defined(OUTPUT_HAVE_STYLESWITCH)
    // Make sure that the supported LightSet OutputSyle is correctly set
    if (dmxnode_output_type_ != nullptr)
    {
        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
            {
                SetOutputStyle(port_index, GetOutputStyle(port_index));
            }
        }
    }
#endif

#if (ARTNET_VERSION >= 4)
    E131Bridge::Start();
#endif

    state_.status = artnet::Status::kOn;
    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

    DEBUG_EXIT();
}

void ArtNetNode::Stop()
{
    DEBUG_ENTRY();

#if (ARTNET_VERSION >= 4)
    E131Bridge::Stop();
#endif

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet)
        {
            if (dmxnode_output_type_ != nullptr)
            {
                dmxnode_output_type_->Stop(port_index);
            }
            dmxnode::Data::ClearLength(port_index);
            output_port_[port_index].is_transmitting = false;
        }
    }

#if defined(ARTNET_HAVE_DMXIN)
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kInput, false);
        }
    }
#endif

    hal::statusled::SetMode(hal::statusled::Mode::OFF_OFF);
    hal::panelled::Off(hal::panelled::ARTNET);

    art_poll_reply_.Status1 = static_cast<uint8_t>((art_poll_reply_.Status1 & ~artnet::Status1::kIndicatorMask) | artnet::Status1::kIndicatorMuteMode);
    state_.status = artnet::Status::kStandby;

    DEBUG_EXIT();
}

void ArtNetNode::GetLongNameDefault(char* long_name)
{
    DEBUG_ENTRY();
#if !defined(ARTNET_LONG_NAME)
    uint8_t board_name_length;
    const auto* const kBoardName = hal::BoardName(board_name_length);
    snprintf(long_name, artnet::kLongNameLength - 1, "%s %s %u %s", kBoardName, artnet::kNodeId, static_cast<unsigned int>(artnet::kVersion), hal::kWebsite);
#else
    uint32_t i;

    for (i = 0; i < (sizeof(ARTNET_LONG_NAME) - 1) && i < (artnet::kLongNameLength - 1); i++)
    {
        if (ARTNET_LONG_NAME[i] == '_')
        {
            long_name[i] = ' ';
        }
        else
        {
            long_name[i] = ARTNET_LONG_NAME[i];
        }
    }

    long_name[i] = '\0';
#endif
    DEBUG_EXIT();
}

void ArtNetNode::SetLongName(const char* long_name)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("long_name=%p", reinterpret_cast<const void*>(long_name));

    if ((long_name == nullptr) || (long_name[0] == '\0'))
    {
        GetLongNameDefault(reinterpret_cast<char*>(art_poll_reply_.LongName));
    }
    else
    {
        DEBUG_PUTS(long_name);
        strncpy(reinterpret_cast<char*>(art_poll_reply_.LongName), long_name, artnet::kLongNameLength - 1);
    }

    art_poll_reply_.LongName[artnet::kLongNameLength - 1] = '\0';

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveLongName(reinterpret_cast<char*>(art_poll_reply_.LongName));
        artnet::display::Longname(reinterpret_cast<char*>(art_poll_reply_.LongName));
    }

    DEBUG_PUTS(reinterpret_cast<char*>(art_poll_reply_.LongName));
    DEBUG_EXIT();
}

void ArtNetNode::SetShortName(uint32_t port_index, const char* name)
{
    DmxNode::Instance().SetShortName(port_index, name);

    const auto* label = DmxNode::Instance().GetShortName(port_index);

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveShortName(port_index, label);
    }

    DEBUG_PUTS(label);
}

const char* ArtNetNode::GetShortName(uint32_t port_index) const
{
    return DmxNode::Instance().GetShortName(port_index);
}

void ArtNetNode::SetLocalMerging()
{
    DEBUG_ENTRY();

    for (uint32_t input_port_index = 0; input_port_index < dmxnode::kMaxPorts; input_port_index++)
    {
        if (node_.port[input_port_index].direction == dmxnode::PortDirection::kOutput)
        {
            continue;
        }

        node_.port[input_port_index].local_merge = false;

        for (uint32_t output_port_index = 0; output_port_index < dmxnode::kMaxPorts; output_port_index++)
        {
            if (node_.port[output_port_index].direction == dmxnode::PortDirection::kInput)
            {
                continue;
            }

            DEBUG_PRINTF("nInputPortIndex=%u %s %u, nOutputPortIndex=%u %s %u", input_port_index,
                         artnet::GetProtocolMode(node_.port[input_port_index].protocol), node_.port[input_port_index].port_address, output_port_index,
                         artnet::GetProtocolMode(node_.port[output_port_index].protocol), node_.port[output_port_index].port_address);

            if ((node_.port[input_port_index].protocol == node_.port[output_port_index].protocol) &&
                (node_.port[input_port_index].port_address == node_.port[output_port_index].port_address))
            {
                if (!node_.port[output_port_index].local_merge)
                {
                    output_port_[output_port_index].source_a.ip = network::kIpaddrLoopback;
                    DEBUG_PUTS("Local merge Source A");
                }
                else
                {
                    output_port_[output_port_index].source_b.ip = network::kIpaddrLoopback;
                    DEBUG_PUTS("Local merge Source B");
                }

                node_.port[input_port_index].local_merge = true;
                node_.port[output_port_index].local_merge = true;
            }
        }
    }

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        DEBUG_PRINTF("port_index=%u, local_merge=%c", port_index, node_.port[port_index].local_merge ? 'Y' : 'N');
    }

    DEBUG_EXIT();
}

void ArtNetNode::SetUniverse(uint32_t port_index, uint16_t universe)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, universe=%u", port_index, universe);

    assert(port_index < dmxnode::kMaxPorts);

    if (node_.port[port_index].port_address == universe)
    {
        DEBUG_EXIT();
        return;
    }

    node_.port[port_index].sw = universe & 0x0F;
    node_.port[port_index].net_switch = (universe >> 8) & 0x7F;
    node_.port[port_index].sub_switch = (universe >> 4) & 0x0F;
    node_.port[port_index].port_address = universe;

#if (ARTNET_VERSION >= 4)
    SetUniverse4(port_index);
#endif

#if defined(ARTNET_HAVE_DMXIN)
    if (state_.status == artnet::Status::kOn)
    {
        SetLocalMerging();
    }
#endif

    DEBUG_EXIT();
}

void ArtNetNode::SetDirection(uint32_t port_index, dmxnode::PortDirection port_direction)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, port_direction=%s", port_index, dmxnode::GetPortDirection(port_direction));

    assert(port_index < dmxnode::kMaxPorts);
    assert(port_direction <= dmxnode::PortDirection::kDisable);

    if (node_.port[port_index].direction == port_direction)
    {
        DEBUG_EXIT();
        return;
    }

    if (port_direction == dmxnode::PortDirection::kDisable)
    {
        if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            assert(state_.enabled_output_ports >= 1);
            state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports - 1);
        }
#if defined(ARTNET_HAVE_DMXIN)
        else if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            assert(state_.enabled_input_ports > 1);
            state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports - 1);
        }
#endif

        node_.port[port_index].direction = dmxnode::PortDirection::kDisable;
    }
#if defined(ARTNET_HAVE_DMXIN)
    else if (port_direction == dmxnode::PortDirection::kInput)
    {
        if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            assert(state_.enabled_output_ports >= 1);
            state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports - 1);
        }

        state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports + 1);
        assert(state_.enabled_input_ports <= dmxnode::kMaxPorts);

        input_port_[port_index].good_input = 0;
        node_.port[port_index].direction = dmxnode::PortDirection::kInput;
    }
#endif
    else if (port_direction == dmxnode::PortDirection::kOutput)
    {
#if defined(ARTNET_HAVE_DMXIN)
        if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            assert(state_.enabled_input_ports >= 1);
            state_.enabled_input_ports = static_cast<uint8_t>(state_.enabled_input_ports - 1);
        }
#endif

        state_.enabled_output_ports = static_cast<uint8_t>(state_.enabled_output_ports + 1);
        assert(state_.enabled_output_ports <= dmxnode::kMaxPorts);

        node_.port[port_index].direction = dmxnode::PortDirection::kOutput;
    }

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveDirection(port_index, port_direction);
#if defined(ARTNET_HAVE_DMXIN)
        SetLocalMerging();
#endif
    }

#if (ARTNET_VERSION >= 4)
    SetDirection4(port_index);
#endif

    DEBUG_EXIT();
}

void ArtNetNode::SetMergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode)
{
    assert(port_index < dmxnode::kMaxPorts);

    if (merge_mode == dmxnode::MergeMode::kLtp)
    {
        output_port_[port_index].good_output |= artnet::GoodOutput::kMergeModeLtp;
    }
    else
    {
        output_port_[port_index].good_output &= static_cast<uint8_t>(~artnet::GoodOutput::kMergeModeLtp);
    }

#if (ARTNET_VERSION >= 4)
    E131Bridge::SetMergeMode(port_index, merge_mode);
#endif

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveMergeMode(port_index, merge_mode);
        artnet::display::MergeMode(port_index, merge_mode);
    }
}

void ArtNetNode::SetFailSafe(artnet::FailSafe fail_safe)
{
    DEBUG_PRINTF("fail_safe=%u", static_cast<uint32_t>(fail_safe));

#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
    if ((state_.status == artnet::Status::kOn) && (fail_safe == artnet::FailSafe::kRecord))
    {
        FailSafeRecord();
        return;
    }
#endif

    art_poll_reply_.Status3 &= static_cast<uint8_t>(~artnet::Status3::kNetworklossMask);

    switch (fail_safe)
    {
        case artnet::FailSafe::kLast:
            art_poll_reply_.Status3 |= artnet::Status3::kNetworklossLastState;
            break;

        case artnet::FailSafe::kOff:
            art_poll_reply_.Status3 |= artnet::Status3::kNetworklossOffState;
            break;

        case artnet::FailSafe::kOn:
            art_poll_reply_.Status3 |= artnet::Status3::kNetworklossOnState;
            break;

        case artnet::FailSafe::kPlayback:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
            art_poll_reply_.Status3 |= artnet::Status3::kNetworklossPlayback;
#endif
            break;

        case artnet::FailSafe::kRecord:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
            assert(false && "case artnet::FailSafe::kRecord");
            __builtin_unreachable();
#endif
            break;

        default:
            [[unlikely]] assert(false && "Invalid fail_safe");
            __builtin_unreachable();
            break;
    }

#if (ARTNET_VERSION >= 4)
    E131Bridge::SetFailSafe(static_cast<dmxnode::FailSafe>(static_cast<uint8_t>(fail_safe) & 0x3));
#endif

    if (state_.status == artnet::Status::kOn)
    {
        const auto kFailSafe = static_cast<uint8_t>(static_cast<uint8_t>(fail_safe) & 0x3);
        artnet::store::SaveFailSafe(kFailSafe);
        artnet::display::Failsafe(kFailSafe);
    }

    DEBUG_EXIT();
}

#if defined(OUTPUT_HAVE_STYLESWITCH)
void ArtNetNode::SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style)
{
    assert(port_index < dmxnode::kMaxPorts);

    if ((output_style == GetOutputStyle(port_index)) && (state_.status == artnet::Status::kOn))
    {
        return;
    }

    if (dmxnode_output_type_ != nullptr)
    {
        dmxnode_output_type_->SetOutputStyle(port_index, output_style);
        output_style = dmxnode_output_type_->GetOutputStyle(port_index);
    }

    if (output_style == dmxnode::OutputStyle::kConstant)
    {
        output_port_[port_index].good_output_b |= artnet::GoodOutputB::kStyleConstant;
    }
    else
    {
        output_port_[port_index].good_output_b &= static_cast<uint8_t>(~artnet::GoodOutputB::kStyleConstant);
    }

    state_.is_synchronous_mode = false;

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveOutputStyle(port_index, output_style);
        artnet::display::Outputstyle(port_index, output_style);
    }
}

dmxnode::OutputStyle ArtNetNode::GetOutputStyle(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);

    const auto kIsStyleConstant = (output_port_[port_index].good_output_b & artnet::GoodOutputB::kStyleConstant) == artnet::GoodOutputB::kStyleConstant;
    return kIsStyleConstant ? dmxnode::OutputStyle::kConstant : dmxnode::OutputStyle::kDelta;
}
#endif

void ArtNetNode::SetNetworkDataLossCondition()
{
    state_.is_merge_mode = false;
    state_.is_synchronous_mode = false;

    uint32_t ip_count = 0;

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
#if defined(ARTNET_HAVE_DMXIN)
        if (node_.port[port_index].local_merge)
        {
            continue;
        }
#endif
        ip_count += (output_port_[port_index].source_a.ip + output_port_[port_index].source_b.ip);
        if (ip_count != 0)
        {
            break;
        }
    }

    if (ip_count == 0)
    {
        return;
    }

    const auto kNetworkloss = (art_poll_reply_.Status3 & artnet::Status3::kNetworklossMask);

    DEBUG_PRINTF("kNetworkloss=%x", kNetworkloss);

    switch (kNetworkloss)
    {
        case artnet::Status3::kNetworklossLastState:
            break;
        case artnet::Status3::kNetworklossOffState:
            dmxnode_output_type_->Blackout(true);
            break;
        case artnet::Status3::kNetworklossOnState:
            dmxnode_output_type_->FullOn();
            break;
        case artnet::Status3::kNetworklossPlayback:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
            FailSafePlayback();
#endif
            break;
        default:
            assert(false && "Invalid kNetworkloss");
            break;
    }

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        output_port_[i].source_a.ip = 0;
        output_port_[i].source_b.ip = 0;
        dmxnode::Data::ClearLength(i);
    }

    hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
    hal::panelled::Off(hal::panelled::ARTNET);

#if defined(ARTNET_HAVE_DMXIN)
    SetLocalMerging();
#endif
}

void ArtNetNode::Print()
{
    printf("Art-Net %u V%u.%u\n", static_cast<unsigned int>(artnet::kVersion), static_cast<unsigned int>(ArtNetConst::kVersion[0]),
           static_cast<unsigned int>(ArtNetConst::kVersion[1]));
    printf(" Long name  : %s\n", reinterpret_cast<char*>(art_poll_reply_.LongName));
#if defined(ARTNET_HAVE_TIMECODE)
    printf(" TimeCode IP: " IPSTR "\n", IP2STR(node_.ip_timecode));
#endif

    if (state_.enabled_output_ports != 0)
    {
        puts(" Output");

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (GetDirection(port_index) == dmxnode::PortDirection::kOutput)
            {
                const auto kUniverse = GetUniverse(port_index);
                const auto kMergeMode = ((output_port_[port_index].good_output & artnet::GoodOutput::kMergeModeLtp) == artnet::GoodOutput::kMergeModeLtp)
                                            ? dmxnode::MergeMode::kLtp
                                            : dmxnode::MergeMode::kHtp;
                printf("  Port %-2u %-4u %s", static_cast<unsigned int>(port_index), static_cast<unsigned int>(kUniverse),
                       dmxnode::GetMergeMode(kMergeMode, true));
#if defined(OUTPUT_HAVE_STYLESWITCH)
                printf(" %s", dmxnode::GetOutputStyle(GetOutputStyle(port_index), true));
#endif
#if (ARTNET_VERSION >= 4)
                printf(" %s", artnet::GetProtocolMode(node_.port[port_index].protocol, true));
#endif
                printf(" %s\n", GetRdm(port_index) ? "RDM" : "   ");
            }
        }
    }

#if defined(ARTNET_HAVE_DMXIN)
    if (state_.enabled_input_ports != 0)
    {
        puts(" Input");

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if (GetDirection(port_index) == dmxnode::PortDirection::kInput)
            {
                const auto kUniverse = GetUniverse(port_index);
                printf("  Port %-2u %-4u", static_cast<unsigned int>(port_index), static_cast<unsigned int>(kUniverse));
                if (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet)
                {
                    const auto kDestinationIp =
                        (input_port_[port_index].destination_ip == 0 ? network::GetBroadcastIp() : input_port_[port_index].destination_ip);
                    printf(" -> " IPSTR, IP2STR(kDestinationIp));
                }
#if (ARTNET_VERSION >= 4)
                printf(" %s\n", artnet::GetProtocolMode(node_.port[port_index].protocol, true));
#else
                puts("");
#endif
            }
        }
    }
#endif

#if (ARTNET_VERSION >= 4)
    if (ArtNetNode::GetActiveOutputPorts() != 0)
    {
        if (IsMapUniverse0())
        {
            puts("  Universes are mapped +1");
        }
    }

    E131Bridge::Print();
#endif
}

void ArtNetNode::HandleTimeSync()
{
    const auto* const kArtTimeSync = reinterpret_cast<artnet::ArtTimeSync*>(receive_buffer_);
    struct tm tm_time;

    tm_time.tm_sec = kArtTimeSync->tm_sec;
    tm_time.tm_min = kArtTimeSync->tm_min;
    tm_time.tm_hour = kArtTimeSync->tm_hour;
    tm_time.tm_mday = kArtTimeSync->tm_mday;
    tm_time.tm_mon = kArtTimeSync->tm_mon;
    tm_time.tm_year = ((kArtTimeSync->tm_year_hi) << 8) + kArtTimeSync->tm_year_lo;

    hal::rtc::Set(&tm_time);

    DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", 1900 + tm_time.tm_year, 1 + tm_time.tm_mon, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

static artnet::OpCodes GetOpCode(uint32_t bytes_received, const uint8_t* buffer)
{
    if (__builtin_expect((bytes_received < kArtnetMinHeaderSize), 0))
    {
        return artnet::OpCodes::kOpNotDefined;
    }

    if (__builtin_expect(((buffer[10] != 0) || (buffer[11] != artnet::kProtocolRevision)), 0))
    {
        return artnet::OpCodes::kOpNotDefined;
    }

    if (__builtin_expect((memcmp(buffer, artnet::kNodeId, 8) == 0), 1))
    {
        return static_cast<artnet::OpCodes>((static_cast<uint16_t>(buffer[9] << 8)) + buffer[8]);
    }

    return artnet::OpCodes::kOpNotDefined;
}

void ArtNetNode::InputUdp(const uint8_t* buffer, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    const auto kOpCode = GetOpCode(size, buffer);

    if (__builtin_expect((kOpCode == artnet::OpCodes::kOpNotDefined), 0)) return;

    receive_buffer_ = const_cast<uint8_t*>(buffer);
    ip_address_from_ = from_ip;

    current_millis_ = hal::Millis();
    packet_millis_ = current_millis_;

    if (state_.is_synchronous_mode)
    {
        if (current_millis_ - state_.art.sync_millis >= (4 * 1000))
        {
            state_.is_synchronous_mode = false;
        }
    }

    switch (kOpCode)
    {
#if (DMXNODE_PORTS > 0)
        case artnet::OpCodes::kOpDmx:
            if (dmxnode_output_type_ != nullptr)
            {
                HandleDmx();
                state_.art.dmx_ip = ip_address_from_;
#if defined(ARTNET_SHOWFILE)
                if (state_.do_record)
                {
                    showfile::record(reinterpret_cast<const artnet::ArtDmx*>(buffer), current_millis_);
                }
#endif
            }
            break;
        case artnet::OpCodes::kOpSync:
            if (dmxnode_output_type_ != nullptr)
            {
                /*
                 * In order to allow for multiple controllers on a network,
                 * a node shall compare the source IP of the ArtSync to the source IP
                 * of the most recent ArtDmx packet.
                 * The ArtSync shall be ignored if the IP addresses do not match.
                 */
                /*
                 * When a port is merging multiple streams of ArtDmx from different IP addresses,
                 * ArtSync packets shall be ignored.
                 */
                if ((state_.art.dmx_ip == ip_address_from_) && (!state_.is_merge_mode))
                {
                    state_.art.sync_millis = current_millis_;
                    HandleSync();
                }
#if defined(ARTNET_SHOWFILE)
                if (state_.do_record)
                {
                    showfile::record(reinterpret_cast<const artnet::ArtSync*>(buffer), current_millis_);
                }
#endif
            }
            break;
#endif
        case artnet::OpCodes::kOpAddress:
            HandleAddress();
            break;
#if defined(ARTNET_HAVE_TIMECODE)
        case artnet::OpCodes::kOpTimecode:
        {
            const auto* const kArtTimeCode = reinterpret_cast<const artnet::ArtTimeCode*>(buffer);
            art_time_code_callback_function_ptr_(reinterpret_cast<const struct artnet::TimeCode*>(&kArtTimeCode->Frames));
        }
        break;
#endif
        case artnet::OpCodes::kOpTimesync:
            HandleTimeSync();
            break;
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
        case artnet::OpCodes::kOpTodrequest:
            if (state_.is_rdm_enabled)
            {
                HandleTodRequest();
            }
            break;
        case artnet::OpCodes::kOpToddata:
            if (state_.is_rdm_enabled)
            {
                HandleTodData();
            }
            break;
        case artnet::OpCodes::kOpTodcontrol:
            if (state_.is_rdm_enabled)
            {
                HandleTodControl();
            }
            break;
        case artnet::OpCodes::kOpRdm:
            if (state_.is_rdm_enabled)
            {
                HandleRdm();
            }
            break;
        case artnet::OpCodes::kOpRdmsub:
            if (state_.is_rdm_enabled)
            {
                HandleRdmSub();
            }
            break;
#endif
        case artnet::OpCodes::kOpIpprog:
            HandleIpProg();
            break;
#if defined(ARTNET_HAVE_TRIGGER)
        case artnet::OpCodes::kOpTrigger:
        {
            const auto* const kArtTrigger = reinterpret_cast<const artnet::ArtTrigger*>(buffer);
            if ((kArtTrigger->oem_code_hi == 0xFF && kArtTrigger->oem_code_lo == 0xFF) ||
                (kArtTrigger->oem_code_hi == ArtNetConst::kOemId[0] && kArtTrigger->oem_code_lo == ArtNetConst::kOemId[1]))
            {
                DEBUG_PRINTF("Key=%d, SubKey=%d, Data[0]=%d", kArtTrigger->key, kArtTrigger->sub_key, kArtTrigger->data[0]);
                art_trigger_callback_function_ptr_(reinterpret_cast<const struct ArtNetTrigger*>(&kArtTrigger->key));
            }
        }
        break;
#endif
#if defined(ARTNET_HAVE_DMXIN)
        case artnet::OpCodes::kOpInput:
            HandleInput();
            break;
#endif
        case artnet::OpCodes::kOpPoll:
            HandlePoll();
            break;
        default:
            // ArtNet but OpCode is not implemented
            // Just skip ... no error
            break;
    }

    hal::panelled::On(hal::panelled::ARTNET);
}

void ArtNetNode::UpdateMergeStatus(uint32_t port_index)
{
    if (!state_.is_merge_mode)
    {
        state_.is_merge_mode = true;
        state_.is_changed = true;
    }

    output_port_[port_index].good_output |= artnet::GoodOutput::kOutputIsMerging;
}

void ArtNetNode::CheckMergeTimeouts(uint32_t port_index)
{
    const auto kTimeOutAMillis = current_millis_ - output_port_[port_index].source_a.millis;

    if (kTimeOutAMillis > (artnet::kMergeTimeoutSeconds * 1000U))
    {
        output_port_[port_index].source_a.ip = 0;
        output_port_[port_index].good_output &= static_cast<uint8_t>(~artnet::GoodOutput::kOutputIsMerging);
    }

    const auto kTimeOutBMillis = current_millis_ - output_port_[port_index].source_b.millis;

    if (kTimeOutBMillis > (artnet::kMergeTimeoutSeconds * 1000U))
    {
        output_port_[port_index].source_b.ip = 0;
        output_port_[port_index].good_output &= static_cast<uint8_t>(~artnet::GoodOutput::kOutputIsMerging);
    }

    auto is_merging = false;

    for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
    {
        is_merging |= ((output_port_[i].good_output & artnet::GoodOutput::kOutputIsMerging) != 0);
    }

    if (!is_merging)
    {
        state_.is_changed = true;
        state_.is_merge_mode = false;
        SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Leaving Merging Mode", port_index);
    }
}

void ArtNetNode::HandleDmx()
{
    const auto* const kArtDmx = reinterpret_cast<artnet::ArtDmx*>(receive_buffer_);
    const auto kDmxSlots = std::min(static_cast<uint32_t>(((kArtDmx->LengthHi << 8) & 0xff00) | kArtDmx->Length), artnet::kDmxLength);

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if ((node_.port[port_index].direction == dmxnode::PortDirection::kOutput) && (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet) &&
            (node_.port[port_index].port_address == kArtDmx->PortAddress))
        {
            output_port_[port_index].good_output |= artnet::GoodOutput::kDataIsBeingTransmitted;

            if (state_.is_merge_mode)
            {
                if (__builtin_expect((!state_.disable_merge_timeout), 1))
                {
                    CheckMergeTimeouts(port_index);
                }
            }

            const auto kIpA = output_port_[port_index].source_a.ip;
            const auto kIpB = output_port_[port_index].source_b.ip;
            const auto kMergeMode = ((output_port_[port_index].good_output & artnet::GoodOutput::kMergeModeLtp) == artnet::GoodOutput::kMergeModeLtp)
                                        ? dmxnode::MergeMode::kLtp
                                        : dmxnode::MergeMode::kHtp;

            if (__builtin_expect((kIpA == 0 && kIpB == 0), 0))
            { // Case 1.
                output_port_[port_index].source_a.ip = ip_address_from_;
                output_port_[port_index].source_a.millis = current_millis_;
                output_port_[port_index].source_a.physical = kArtDmx->Physical;
                dmxnode::Data::SetSourceA(port_index, kArtDmx->data, kDmxSlots);
                SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 1. First packet", port_index, kArtDmx->Physical);
            }
            else if (kIpA == ip_address_from_ && kIpB == 0)
            { // Case 2.
                if (output_port_[port_index].source_a.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_a.millis = current_millis_;
                    dmxnode::Data::SetSourceA(port_index, kArtDmx->data, kDmxSlots);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 2. continued transmission from the same ip (source A)", port_index, kArtDmx->Physical);
                }
                else if (output_port_[port_index].source_b.physical != kArtDmx->Physical)
                {
                    output_port_[port_index].source_b.ip = ip_address_from_;
                    output_port_[port_index].source_b.millis = current_millis_;
                    output_port_[port_index].source_b.physical = kArtDmx->Physical;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceB(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 2. New source from same ip (source B), start the merge", port_index, kArtDmx->Physical);
                }
                else
                {
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 2. More than two sources, discarding data", port_index, kArtDmx->Physical);
                    return;
                }
            }
            else if (kIpA == 0 && kIpB == ip_address_from_)
            { // Case 3.
                if (output_port_[port_index].source_b.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_b.millis = current_millis_;
                    dmxnode::Data::SetSourceB(port_index, kArtDmx->data, kDmxSlots);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 3. continued transmission from the same ip (source B)", port_index, kArtDmx->Physical);
                }
                else if (output_port_[port_index].source_a.physical != kArtDmx->Physical)
                {
                    output_port_[port_index].source_a.ip = ip_address_from_;
                    output_port_[port_index].source_a.millis = current_millis_;
                    output_port_[port_index].source_a.physical = kArtDmx->Physical;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceA(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 3. New source from same ip (source A), start the merge", port_index, kArtDmx->Physical);
                }
                else
                {
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 3. More than two sources, discarding data", port_index, kArtDmx->Physical);
                    return;
                }
            }
            else if (kIpA != ip_address_from_ && kIpB == 0)
            { // Case 4.
                output_port_[port_index].source_b.ip = ip_address_from_;
                output_port_[port_index].source_b.millis = current_millis_;
                output_port_[port_index].source_b.physical = kArtDmx->Physical;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceB(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 4. new source, start the merge", port_index, kArtDmx->Physical);
            }
            else if (kIpA == 0 && kIpB != ip_address_from_)
            { // Case 5.
                output_port_[port_index].source_a.ip = ip_address_from_;
                output_port_[port_index].source_a.millis = current_millis_;
                output_port_[port_index].source_a.physical = kArtDmx->Physical;
                UpdateMergeStatus(port_index);
                dmxnode::Data::MergeSourceA(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 5. new source, start the merge", port_index, kArtDmx->Physical);
            }
            else if (kIpA == ip_address_from_ && kIpB != ip_address_from_)
            { // Case 6.
                if (output_port_[port_index].source_a.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_a.millis = current_millis_;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceA(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 6. continue merge (Source A)", port_index, kArtDmx->Physical);
                }
                else
                {
                    SendDiag(artnet::PriorityCodes::kDiagMed, "%u:%u 6. More than two sources, discarding data", port_index, kArtDmx->Physical);
                    return;
                }
            }
            else if (kIpA != ip_address_from_ && kIpB == ip_address_from_)
            { // Case 7.
                if (output_port_[port_index].source_b.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_b.millis = current_millis_;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceB(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 7. continue merge (Source B)", port_index, kArtDmx->Physical);
                }
                else
                {
                    SendDiag(artnet::PriorityCodes::kDiagMed, "%u:%u 7. More than two sources, discarding data", port_index, kArtDmx->Physical);
                    puts("WARN: 7. More than two sources, discarding data");
                    return;
                }
            }
            else if (kIpA == ip_address_from_ && kIpB == ip_address_from_)
            { // Case 8.
                if (output_port_[port_index].source_a.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_a.millis = current_millis_;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceA(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 8. Source matches both ip, merging Physical (source_a)", port_index, kArtDmx->Physical);
                }
                else if (output_port_[port_index].source_b.physical == kArtDmx->Physical)
                {
                    output_port_[port_index].source_b.millis = current_millis_;
                    UpdateMergeStatus(port_index);
                    dmxnode::Data::MergeSourceB(port_index, kArtDmx->data, kDmxSlots, kMergeMode);
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 8. Source matches both ip, merging Physical (source_b)", port_index, kArtDmx->Physical);
                }
                else
                {
                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u:%u 8. Source matches both ip, more than two sources, discarding data", port_index,
                             kArtDmx->Physical);
                    puts("WARN: 8. Source matches both ip, discarding data");
                    return;
                }
            }
#ifndef NDEBUG
            else if (kIpA != ip_address_from_ && kIpB != ip_address_from_)
            { // Case 9.
                SendDiag(artnet::PriorityCodes::kDiagLow, "%u: 9. More than two sources, discarding data", port_index);
                puts("WARN: 9. More than two sources, discarding data");
                return;
            }
#endif
            else [[unlikely]]
            { // Case 0.
                SendDiag(artnet::PriorityCodes::kDiagHigh, "%u: 0. No cases matched, this shouldn't happen!", port_index);
#ifndef NDEBUG
                puts("ERROR: 0. No cases matched, this shouldn't happen!");
#endif
                return;
            }

            if ((state_.is_synchronous_mode) &&
                ((output_port_[port_index].good_output & artnet::GoodOutput::kOutputIsMerging) != artnet::GoodOutput::kOutputIsMerging))
            {
                dmxnode::DataSet(dmxnode_output_type_, port_index);
                output_port_[port_index].is_data_pending = true;
                SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Buffering data", port_index);
            }
            else
            {
                dmxnode::DataOutput(dmxnode_output_type_, port_index);

                if (!output_port_[port_index].is_transmitting)
                {
                    dmxnode_output_type_->Start(port_index);
                    state_.is_changed = true;
                    output_port_[port_index].is_transmitting = true;
                }

                SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Send data", port_index);
            }

            state_.receiving_dmx |= (1U << static_cast<uint8_t>(dmxnode::PortDirection::kOutput));
        }
    }
}

/**
 * When a node receives an ArtSync packet it should transfer to synchronous operation.
 * This means that received ArtDmx packets will be buffered
 * and output when the next ArtSync is received.
 */
void ArtNetNode::HandleSync()
{
    if (!state_.is_synchronous_mode)
    {
        state_.is_synchronous_mode = true;
        return;
    }

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (output_port_[port_index].is_data_pending)
        {
            dmxnode_output_type_->Sync(port_index);
            SendDiag(artnet::PriorityCodes::kDiagLow, "Sync individual %u", port_index);
        }
    }

    dmxnode_output_type_->Sync();

    SendDiag(artnet::PriorityCodes::kDiagLow, "Sync all");

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
}
