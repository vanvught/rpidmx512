/**
 * @file artnetparams.cpp
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

#ifdef DEBUG_ARTNETPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "artnetnode.h"
#include "network.h"
#include "json/artnetparams.h"
#include "json/artnetparamsconst.h"
#include "dmxnode_utils.h"
#include "json/json_parser.h"
#include "ip4/ip4_helpers.h"
#include "dmxnode.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "firmware/debug/debug_dump.h"

using common::store::dmxnode::Flags;

namespace json
{
ArtNetParams::ArtNetParams()
{
    ConfigStore::Instance().Copy(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void ArtNetParams::SetEnableRdm(const char* val, uint32_t len)
{
    DEBUG_ENTRY();

    if (len != 1) return;

    store_dmxnode_.flags = common::SetFlagValue(store_dmxnode_.flags, Flags::Flag::kEnableRdm, val[0] != '0');

    DEBUG_EXIT();
}

void ArtNetParams::SetMapUniverse0(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_dmxnode_.flags = common::SetFlagValue(store_dmxnode_.flags, Flags::Flag::kMapUniverse0, val[0] != '0');
}

void ArtNetParams::SetDestinationIpPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    if (val_len == 0)
    {
        store_dmxnode_.destination_ip[kIndex] = network::GetBroadcastIp();
    }
    else
    {
        store_dmxnode_.destination_ip[kIndex] = net::ParseIpString(val, val_len);
    }
}

void ArtNetParams::SetProtocolPort(const char* key, uint32_t key_len, const char* val, [[maybe_unused]] uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto protocol = store_dmxnode_.protocol;
    if (val_len == 0)
    {
        json::PortSet<artnet::PortProtocol>(kIndex, artnet::PortProtocol::kArtnet, protocol);
    }
    else
    {
        json::PortSet<artnet::PortProtocol>(kIndex, artnet::GetProtocolMode(val), protocol);
    }
    store_dmxnode_.protocol = protocol;
}

void ArtNetParams::SetRdmEnablePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    DEBUG_ENTRY();
    debug::Dump(key, key_len);
    debug::Dump(val, val_len);

    if (val_len != 1) return;

    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto rdm = store_dmxnode_.rdm;
    json::PortSet<dmxnode::Rdm>(kIndex, val[0] != '0' ? dmxnode::Rdm::kEnable : dmxnode::Rdm::kDisable, rdm);
    store_dmxnode_.rdm = rdm;

    DEBUG_EXIT();
}

void ArtNetParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kArtNetKeys);
    ConfigStore::Instance().Store(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void ArtNetParams::Set()
{
    DEBUG_ENTRY();

    auto& artnet = *ArtNetNode::Get();

#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
    artnet.SetRdm(common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kEnableRdm));
#endif
#if (ARTNET_VERSION >= 4)
    artnet.SetMapUniverse0(common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kMapUniverse0));
#endif

    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
        {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= dmxnode::kMaxPorts)
            {
                break;
            }

            artnet.SetDestinationIp(kPortIndex, store_dmxnode_.destination_ip[config_port_index]);
#if (ARTNET_VERSION >= 4)
            artnet.SetPortProtocol4(kPortIndex, json::PortGet<artnet::PortProtocol>(config_port_index, store_dmxnode_.protocol));
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
            const auto kRdm = json::PortGet<dmxnode::Rdm>(config_port_index, store_dmxnode_.rdm);
            artnet.SetRdm(kPortIndex, kRdm == dmxnode::Rdm::kEnable);
#endif
        }
    }

#ifndef NDEBUG
    Dump();
#endif

    DEBUG_EXIT();
}

void ArtNetParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::ArtNetParamsConst::kFileName);

#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
    printf(" %s=%u\n", json::ArtNetParamsConst::kEnableRdm.name, common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kEnableRdm));
#endif
#if (ARTNET_VERSION >= 4)
    printf(" %s=%u\n", json::ArtNetParamsConst::kMapUniverse0.name, common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kMapUniverse0));
#endif

    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        for (uint32_t port_index = 0; port_index < dmxnode::kConfigPortCount; port_index++)
        {
#if (ARTNET_VERSION >= 4)
            const auto kProtocol = json::PortGet<artnet::PortProtocol>(port_index, store_dmxnode_.protocol);
            printf(" %s=%s\n", json::ArtNetParamsConst::kProtocolPort[port_index].name, artnet::GetProtocolMode(kProtocol));
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
            const auto kRdm = json::PortGet<dmxnode::Rdm>(port_index, store_dmxnode_.rdm);
            printf(" %s=%u\n", json::ArtNetParamsConst::kRdmEnablePort[port_index].name, static_cast<uint32_t>(kRdm));
#endif
            printf(" %s=" IPSTR "\n", json::ArtNetParamsConst::kDestinationIpPort[port_index].name, IP2STR(store_dmxnode_.destination_ip[port_index]));
        }
    }
	
    auto& artnet = *ArtNetNode::Get();
    artnet.Print();
}
} // namespace json