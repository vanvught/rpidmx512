/**
 * @file rdmhandlere1372.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_RDM_LLRP)
#undef NDEBUG
#endif

#include <cstring>
#include <algorithm>

#include "rdmhandler.h"
#include "e120.h"
#include "rdm_e120.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

namespace dhcp
{
enum class Mode : uint8_t
{
    kInactive = 0x00, ///< The IP address was not obtained via DHCP
    kActive = 0x01,   ///< The IP address was obtained via DHCP
    kUnknown = 0x02   ///< The system cannot determine if the address was obtained via DHCP
};
} // namespace dhcp

static dhcp::Mode GetDhcpMode()
{
    if (network::iface::Dhcp())
    {
        return dhcp::Mode::kActive;
    }

    return dhcp::Mode::kInactive;
}

struct QueuedConfig
{
    static constexpr uint32_t kNone = 0;
    static constexpr uint32_t kStaticIp = (1U << 0);
    static constexpr uint32_t kNetmask = (1U << 1);
    static constexpr uint32_t kGw = (1U << 2);
    static constexpr uint32_t kDhcp = (1U << 3);
    static constexpr uint32_t kZeroconf = (1U << 4);
    uint32_t mask = QueuedConfig::kNone;
    uint32_t static_ip;
    uint32_t netmask;
    uint32_t gateway;
    dhcp::Mode mode;
};

static QueuedConfig s_queued_config;

static bool IsQueuedMaskSet(uint32_t mask)
{
    return (s_queued_config.mask & mask) == mask;
}

static void SetQueuedStaticIp(uint32_t static_ip, uint32_t netmask)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR ", netmask=" IPSTR, IP2STR(static_ip), IP2STR(netmask));

    if (static_ip != 0)
    {
        s_queued_config.static_ip = static_ip;
    }
    else
    {
        s_queued_config.static_ip = network::GetPrimaryIp();
    }

    if (netmask != 0)
    {
        s_queued_config.netmask = netmask;
    }
    else
    {
        s_queued_config.netmask = network::GetNetmask();
    }

    s_queued_config.mask |= QueuedConfig::kStaticIp;
    s_queued_config.mask |= QueuedConfig::kNetmask;

    DEBUG_EXIT();
}

static void SetQueuedDefaultRoute(uint32_t gateway_ip)
{
    if (gateway_ip != 0)
    {
        s_queued_config.gateway = gateway_ip;
    }
    else
    {
        s_queued_config.gateway = network::GetGatewayIp();
    }

    s_queued_config.mask |= QueuedConfig::kGw;
}

static void SetQueuedDhcp(dhcp::Mode mode)
{
    s_queued_config.mode = mode;
    s_queued_config.mask |= QueuedConfig::kDhcp;
}

static void SetQueuedZeroconf()
{
    s_queued_config.mask |= QueuedConfig::kZeroconf;
}

static bool ApplyQueuedConfig()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_queued_config.mask=%x, " IPSTR ", " IPSTR, s_queued_config.mask, IP2STR(s_queued_config.static_ip), IP2STR(s_queued_config.netmask));

    if (s_queued_config.mask == QueuedConfig::kNone)
    {
        DEBUG_EXIT();
        return false;
    }

    if ((IsQueuedMaskSet(QueuedConfig::kStaticIp)) || (IsQueuedMaskSet(QueuedConfig::kNetmask)) || (IsQueuedMaskSet(QueuedConfig::kGw)))
    {
        // After SetIp all ip address might be zero.
        if (IsQueuedMaskSet(QueuedConfig::kStaticIp))
        {
            network::SetPrimaryIp(s_queued_config.static_ip);
        }

        if (IsQueuedMaskSet(QueuedConfig::kNetmask))
        {
            network::SetNetmask(s_queued_config.netmask);
        }

        if (IsQueuedMaskSet(QueuedConfig::kGw))
        {
            network::SetGatewayIp(s_queued_config.gateway);
        }

        s_queued_config.mask = QueuedConfig::kNone;

        DEBUG_EXIT();
        return true;
    }

    if (IsQueuedMaskSet(QueuedConfig::kDhcp))
    {
        if (s_queued_config.mode == dhcp::Mode::kActive)
        {
            network::iface::EnableDhcp();
        }
        else if (s_queued_config.mode == dhcp::Mode::kInactive)
        {
        }

        s_queued_config.mode = dhcp::Mode::kUnknown;
        s_queued_config.mask = QueuedConfig::kNone;

        DEBUG_EXIT();
        return true;
    }

    if (IsQueuedMaskSet(QueuedConfig::kZeroconf))
    {
        network::iface::SetAutoIp();
        s_queued_config.mask = QueuedConfig::kNone;

        DEBUG_EXIT();
        return true;
    }

    DEBUG_EXIT();
    return false;
}

/*
 * ANSI E1.37-2
 */

bool RDMHandler::CheckInterfaceID([[maybe_unused]] const struct TRdmMessageNoSc* rdm_data_in)
{
#if !defined(DMX_WORKSHOP_DEFECT)
    const auto kInterfaceID = static_cast<uint32_t>((rdm_data_in->param_data[0] << 24) + (rdm_data_in->param_data[1] << 16) +
                                                    (rdm_data_in->param_data[2] << 8) + rdm_data_in->param_data[3]);

    if (kInterfaceID != network::iface::InterfaceIndex())
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return false;
    }
#endif
    return true;
}

void RDMHandler::GetInterfaceList([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();
    // https://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
    const uint16_t kInterfaceHardwareType = 0x6; //	IEEE 802 Networks

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    const uint32_t kNetworkInterfaceId = network::iface::InterfaceIndex();

    rdm_data_out->param_data[0] = static_cast<uint8_t>(kNetworkInterfaceId >> 24);
    rdm_data_out->param_data[1] = static_cast<uint8_t>(kNetworkInterfaceId >> 16);
    rdm_data_out->param_data[2] = static_cast<uint8_t>(kNetworkInterfaceId >> 8);
    rdm_data_out->param_data[3] = static_cast<uint8_t>(kNetworkInterfaceId);
    rdm_data_out->param_data[4] = (kInterfaceHardwareType >> 8);
    rdm_data_out->param_data[5] = kInterfaceHardwareType;

    rdm_data_out->param_data_length = 6;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetInterfaceName([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);

    static const auto kLength = std::min(strlen(network::iface::InterfaceName()), static_cast<size_t>(32));

    memcpy(reinterpret_cast<char*>(&rdm_data_out->param_data[4]), network::iface::InterfaceName(), kLength);

    rdm_data_out->param_data_length = static_cast<uint8_t>(4 + kLength);

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetHardwareAddress([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    network::iface::CopyMacAddressTo(&rdm_data_out->param_data[4]);

    rdm_data_out->param_data_length = 10;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetDHCPMode([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    rdm_data_out->param_data[4] = network::iface::Dhcp() ? 1 : 0;

    rdm_data_out->param_data_length = 5;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetDHCPMode([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    if ((!network::iface::IsDhcpKnown()) || (!network::iface::IsDhcpCapable()))
    {
        RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

        DEBUG_EXIT();
        return;
    }

    const auto kMode = static_cast<dhcp::Mode>(rdm_data_in->param_data[4]);

    if ((kMode == dhcp::Mode::kActive) || kMode == dhcp::Mode::kInactive)
    {
        SetQueuedDhcp(kMode);
        RespondMessageAck();

        DEBUG_EXIT();
        return;
    }

    RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

    DEBUG_EXIT();
}

void RDMHandler::GetNameServers([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);
    const auto kNameServerIndex = rdm_data_in->param_data[0];

    if ((kNameServerIndex >= network::iface::NameServerCount()) || (kNameServerIndex > 2))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    uint32_t ip_address = network::iface::NameServer(kNameServerIndex);
    const auto* p = reinterpret_cast<const uint8_t*>(&ip_address);

    rdm_data_out->param_data[0] = kNameServerIndex;
    memcpy(&rdm_data_out->param_data[1], p, 4);

    rdm_data_out->param_data_length = 5;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetZeroconf([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    rdm_data_out->param_data[4] = network::iface::IsAutoIpCapable() ? (network::iface::AutoIp()) : 0;

    rdm_data_out->param_data_length = 5;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetAutoIp([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    if (!network::iface::IsAutoIpCapable())
    {
        RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

        DEBUG_EXIT();
        return;
    }

    if (rdm_data_in->param_data[4] == 1)
    {
        SetQueuedZeroconf();
        RespondMessageAck();

        DEBUG_EXIT();
        return;
    }

    if (rdm_data_in->param_data[4] == 0)
    {
        SetQueuedStaticIp(0, 0);
        RespondMessageAck();

        DEBUG_EXIT();
        return;
    }

    RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

    DEBUG_EXIT();
}

void RDMHandler::RenewDhcp([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    if (!network::iface::IsDhcpKnown())
    {
        RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

        DEBUG_EXIT();
        return;
    }

    if (!network::iface::Dhcp())
    {
        RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

        DEBUG_EXIT();
        return;
    }

    network::iface::EnableDhcp();

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetAddressNetmask([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    auto ip_address = network::GetPrimaryIp();
    const auto* p = reinterpret_cast<const uint8_t*>(&ip_address);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    memcpy(&rdm_data_out->param_data[4], p, 4);
    rdm_data_out->param_data[8] = static_cast<uint8_t>(network::GetNetmaskCIDR());
    rdm_data_out->param_data[9] = static_cast<uint8_t>(GetDhcpMode());

    rdm_data_out->param_data_length = 10;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetStaticAddress([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    uint32_t ip_address = network::GetPrimaryIp();
    const auto* p = reinterpret_cast<const uint8_t*>(&ip_address);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    memcpy(&rdm_data_out->param_data[4], p, 4);
    rdm_data_out->param_data[8] = static_cast<uint8_t>(network::GetNetmaskCIDR());

    rdm_data_out->param_data_length = 9;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetStaticAddress([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (rdm_data_in->param_data_length != 9)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);

        DEBUG_EXIT();
        return;
    }

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    uint32_t ip_address;
    auto* p = reinterpret_cast<uint8_t*>(&ip_address);
    memcpy(p, &rdm_data_in->param_data[4], 4);

    SetQueuedStaticIp(ip_address, network::CidrToNetmask(rdm_data_in->param_data[8]));

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::ApplyConfiguration([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    if (ApplyQueuedConfig())
    { // Not Queuing -> Apply
        RespondMessageAck();

        DEBUG_EXIT();
        return;
    }

    RespondMessageNack(E120_NR_FORMAT_ERROR);

    DEBUG_EXIT();
}

void RDMHandler::GetDefaultRoute([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    auto* rdm_data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    uint32_t ip_address = network::GetGatewayIp();
    const auto* p = reinterpret_cast<const uint8_t*>(&ip_address);

    memcpy(&rdm_data_out->param_data[0], &rdm_data_in->param_data[0], 4);
    memcpy(&rdm_data_out->param_data[4], p, 4);

    rdm_data_out->param_data_length = 8;

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetDefaultRoute([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* rdm_data_in = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (rdm_data_in->param_data_length != 8)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);

        DEBUG_EXIT();
        return;
    }

    if (!CheckInterfaceID(rdm_data_in))
    {
        DEBUG_EXIT();
        return;
    }

    uint32_t ip_address;
    auto* p = reinterpret_cast<uint8_t*>(&ip_address);
    memcpy(p, &rdm_data_in->param_data[4], 4);

    SetQueuedDefaultRoute(ip_address);

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetHostName([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* host_name = network::iface::HostName();
    HandleString(host_name, static_cast<uint32_t>(strlen(host_name)));

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetHostName([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    auto* rdm_data_in = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (rdm_data_in->param_data_length >= 64)
    {
        RespondMessageNack(E120_NR_HARDWARE_FAULT);

        DEBUG_EXIT();
        return;
    }

    rdm_data_in->param_data[rdm_data_in->param_data_length] = '\0';

    network::iface::SetHostname(reinterpret_cast<const char*>(rdm_data_in->param_data));

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::GetDomainName([[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    const auto* domain_name = network::iface::DomainName();
    HandleString(domain_name, static_cast<uint32_t>(strlen(domain_name)));

    RespondMessageAck();

    DEBUG_EXIT();
}

void RDMHandler::SetDomainName([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    DEBUG_ENTRY();

    RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

    DEBUG_EXIT();
}
