/**
 * @file artnetnodehandleaddress.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARTNET_ADDRESS)
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "artnet.h"
#include "artnetnode.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "dmxnodedata.h"
#include "dmxnode_data.h"
#include "hal_statusled.h"
 #include "firmware/debug/debug_debug.h"

void ArtNetNode::SetSwitch(uint32_t port_index, uint8_t sw)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(state_.status == artnet::Status::kOn);

    node_.port[port_index].sw = sw;
    SetPortAddress(port_index);

    artnet::store::SaveSwitch(port_index, sw);
    artnet::display::Universe(port_index, node_.port[port_index].port_address);

#if (ARTNET_VERSION >= 4)
    SetUniverse4(port_index);
#endif

#if defined(ARTNET_HAVE_DMXIN)
    SetLocalMerging();
#endif

    DEBUG_EXIT();
}

void ArtNetNode::HandleAddress()
{
    const auto* const kArtAddress = reinterpret_cast<artnet::ArtAddress*>(receive_buffer_);
    state_.report_code = artnet::ReportCode::kRcpowerok;

    const auto kPortIndex = static_cast<uint32_t>(kArtAddress->bind_index > 0 ? kArtAddress->bind_index - 1 : 0);

    DEBUG_PRINTF("kPortIndex=%u", kPortIndex);

    if (kArtAddress->ShortName[0] != 0)
    {
        SetShortName(kPortIndex, reinterpret_cast<const char*>(kArtAddress->ShortName));
        state_.report_code = artnet::ReportCode::kRcshnameok;
    }

    if (kArtAddress->LongName[0] != 0)
    {
        SetLongName(reinterpret_cast<const char*>(kArtAddress->LongName));
        state_.report_code = artnet::ReportCode::kRclonameok;
    }

    if (kArtAddress->SubSwitch == artnet::Program::kDefaults)
    {
        node_.port[kPortIndex].sub_switch = artnet::defaults::kSubnetSwitch;
    }
    else if (kArtAddress->SubSwitch & artnet::Program::kChangeMask)
    {
        node_.port[kPortIndex].sub_switch = static_cast<uint8_t>(kArtAddress->SubSwitch & ~artnet::Program::kChangeMask);
    }

    if (kArtAddress->NetSwitch == artnet::Program::kDefaults)
    {
        node_.port[kPortIndex].net_switch = artnet::defaults::kNetSwitch;
    }
    else if (kArtAddress->NetSwitch & artnet::Program::kChangeMask)
    {
        node_.port[kPortIndex].net_switch = static_cast<uint8_t>(kArtAddress->NetSwitch & ~artnet::Program::kChangeMask);
    }

    if (kArtAddress->SwOut[0] == artnet::Program::kNoChange)
    {
        SetSwitch(kPortIndex, node_.port[kPortIndex].sw);
    }
    else
    {
        if (node_.port[kPortIndex].direction == dmxnode::PortDirection::kOutput)
        {
            if (kArtAddress->SwOut[0] == artnet::Program::kDefaults)
            {
                SetSwitch(kPortIndex, artnet::defaults::kSwitch);
            }
            else if (kArtAddress->SwOut[0] & artnet::Program::kChangeMask)
            {
                SetSwitch(kPortIndex, static_cast<uint8_t>(kArtAddress->SwOut[0] & ~artnet::Program::kChangeMask));
            }
        }
    }

    if (kArtAddress->SwIn[0] == artnet::Program::kNoChange)
    {
        SetSwitch(kPortIndex, node_.port[kPortIndex].sw);
    }
    else
    {
        if (node_.port[kPortIndex].direction == dmxnode::PortDirection::kInput)
        {
            if (kArtAddress->SwIn[0] == artnet::Program::kDefaults)
            {
                SetSwitch(kPortIndex, artnet::defaults::kSwitch);
            }
            else if (kArtAddress->SwIn[0] & artnet::Program::kChangeMask)
            {
                SetSwitch(kPortIndex, static_cast<uint8_t>(kArtAddress->SwIn[0] & ~artnet::Program::kChangeMask));
            }
        }
    }

    switch (kArtAddress->Command)
    {
        case artnet::PortCommand::kNone:
            DEBUG_PUTS("No action.");
            break;
        case artnet::PortCommand::kCancel:
            state_.is_merge_mode = false;
            for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
            {
                output_port_[port_index].source_a.ip = 0;
                output_port_[port_index].source_b.ip = 0;
                output_port_[port_index].good_output &= static_cast<uint8_t>(~artnet::GoodOutput::kOutputIsMerging);
            }
            break;

        case artnet::PortCommand::kLedNormal:
            hal::statusled::SetModeWithLock(hal::statusled::Mode::NORMAL, false);
            art_poll_reply_.Status1 =
                static_cast<uint8_t>((art_poll_reply_.Status1 & ~artnet::Status1::kIndicatorMask) | artnet::Status1::kIndicatorNormalMode);
#if (ARTNET_VERSION >= 4)
            E131Bridge::SetEnableDataIndicator(true);
#endif
            break;

        case artnet::PortCommand::kLedMute:
            hal::statusled::SetModeWithLock(hal::statusled::Mode::OFF_OFF, true);
            art_poll_reply_.Status1 = static_cast<uint8_t>((art_poll_reply_.Status1 & ~artnet::Status1::kIndicatorMask) | artnet::Status1::kIndicatorMuteMode);
#if (ARTNET_VERSION >= 4)
            E131Bridge::SetEnableDataIndicator(false);
#endif
            break;

        case artnet::PortCommand::kLedLocate:
            hal::statusled::SetModeWithLock(hal::statusled::Mode::FAST, true);
            art_poll_reply_.Status1 =
                static_cast<uint8_t>((art_poll_reply_.Status1 & ~artnet::Status1::kIndicatorMask) | artnet::Status1::kIndicatorLocateMode);
#if (ARTNET_VERSION >= 4)
            E131Bridge::SetEnableDataIndicator(false);
#endif
            break;

#if defined(ARTNET_HAVE_DMXIN)
        case artnet::PortCommand::kReset:
            for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
            {
                const auto kMask =
                    artnet::GoodInput::kIncludesTestPackets | artnet::GoodInput::kIncludesSip | artnet::GoodInput::kIncludesText | artnet::GoodInput::kErrors;
                input_port_[port_index].good_input &= static_cast<uint8_t>(~kMask);
            }
            break;
#endif
        case artnet::PortCommand::kFailHold:
        case artnet::PortCommand::kFailZero:
        case artnet::PortCommand::kFailFull:
        case artnet::PortCommand::kFailScene:
        case artnet::PortCommand::kFailRecord:
            SetFailSafe(static_cast<artnet::FailSafe>(kArtAddress->Command & 0x0f));
            break;

        case artnet::PortCommand::kMergeLtpO:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kMergeLtp1:
        case artnet::PortCommand::kMergeLtp2:
        case artnet::PortCommand::kMergeLtp3:
#endif
            SetMergeMode(kPortIndex, dmxnode::MergeMode::kLtp);
            break;

#if defined(ARTNET_HAVE_DMXIN)
        case artnet::PortCommand::kDirectionTxO:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kDirectionTx1:
        case artnet::PortCommand::kDirectionTx2:
        case artnet::PortCommand::kDirectionTx3:
#endif
            SetDirection(kPortIndex, dmxnode::PortDirection::kOutput);
            break;

        case artnet::PortCommand::kDirectionRxO:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kDirectionRx1:
        case artnet::PortCommand::kDirectionRx2:
        case artnet::PortCommand::kDirectionRx3:
#endif
            SetDirection(kPortIndex, dmxnode::PortDirection::kInput);
            break;
#endif
        case artnet::PortCommand::kMergeHtp0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kMergeHtp1:
        case artnet::PortCommand::kMergeHtp2:
        case artnet::PortCommand::kMergeHtp3:
#endif
            SetMergeMode(kPortIndex, dmxnode::MergeMode::kHtp);
            break;

#if (ARTNET_VERSION >= 4)
        case artnet::PortCommand::kArtnetSel0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kArtnetSel1:
        case artnet::PortCommand::kArtnetSel2:
        case artnet::PortCommand::kArtnetSel3:
#endif
            SetPortProtocol4(kPortIndex, artnet::PortProtocol::kArtnet);
            break;

        case artnet::PortCommand::kAcnSel0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kAcnSel1:
        case artnet::PortCommand::kAcnSel2:
        case artnet::PortCommand::kAcnSel3:
#endif
            SetPortProtocol4(kPortIndex, artnet::PortProtocol::kSacn);
            break;
#endif

        case artnet::PortCommand::kClr0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kClr1:
        case artnet::PortCommand::kClr2:
        case artnet::PortCommand::kClr3:
#endif
            if (node_.port[kPortIndex].protocol == artnet::PortProtocol::kArtnet)
            {
                dmxnode::Data::Clear(kPortIndex);
                dmxnode::DataOutput(dmxnode_output_type_, kPortIndex);
            }
#if (ARTNET_VERSION >= 4)
            if (node_.port[kPortIndex].protocol == artnet::PortProtocol::kSacn)
            {
                E131Bridge::Clear(kPortIndex);
            }
#endif
            break;

#if defined(OUTPUT_HAVE_STYLESWITCH)
        case artnet::PortCommand::kStyleDelta0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kStyleDelta1:
        case artnet::PortCommand::kStyleDelta2:
        case artnet::PortCommand::kStyleDelta3:
#endif
            SetOutputStyle(kPortIndex, dmxnode::OutputStyle::kDelta);
            break;

        case artnet::PortCommand::kStyleConstant0:
#if (ARTNET_VERSION < 4)
        case artnet::PortCommand::kStyleConstant1:
        case artnet::PortCommand::kStyleConstant2:
        case artnet::PortCommand::kStyleConstant3:
#endif
            SetOutputStyle(kPortIndex, dmxnode::OutputStyle::kConstant);
            break;
#endif

#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
        case artnet::PortCommand::kRdmEnable0:
            SetRdm(kPortIndex, true);
            break;

        case artnet::PortCommand::kRdmDisable0:
            SetRdm(kPortIndex, false);
            break;
#endif
        default:
            [[unlikely]] DEBUG_PRINTF("> Not implemented: %u [%x]", kArtAddress->Command, kArtAddress->Command);
            break;
    }

    SendPollReply(kPortIndex, ip_address_from_);
}
