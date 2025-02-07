/**
 * @file rdmhandlere137.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_RDM_LLRP)
# undef NDEBUG
#endif

#include <cstring>
#include <algorithm>

#include "rdmhandler.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#include "network.h"

#include "debug.h"

namespace dhcp {
enum class Mode: uint8_t {
	INACTIVE = 0x00,	///< The IP address was not obtained via DHCP
	ACTIVE = 0x01,		///< The IP address was obtained via DHCP
	UNKNOWN = 0x02		///< The system cannot determine if the address was obtained via DHCP
};
}  // namespace dhcp

static dhcp::Mode get_dhcp_mode() {
	if (Network::Get()->IsDhcpUsed()) {
		return dhcp::Mode::ACTIVE;
	}

	return dhcp::Mode::INACTIVE;
}

struct QueuedConfig {
	static constexpr uint32_t NONE = 0;
	static constexpr uint32_t STATIC_IP = (1U << 0);
	static constexpr uint32_t NETMASK   = (1U << 1);
	static constexpr uint32_t GW        = (1U << 2);
	static constexpr uint32_t DHCP      = (1U << 3);
	static constexpr uint32_t ZEROCONF  = (1U << 4);
	uint32_t nMask = QueuedConfig::NONE;
	uint32_t nStaticIp;
	uint32_t nNetmask;
	uint32_t nGateway;
	dhcp::Mode mode;
};

static QueuedConfig s_QueuedConfig;

static bool is_queued_mask_set(const uint32_t nMask) {
	return (s_QueuedConfig.nMask & nMask) == nMask;
}

static void set_queued_static_ip(const uint32_t nStaticIp, const uint32_t nNetmask) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR ", nNetmask=" IPSTR, IP2STR(nStaticIp), IP2STR(nNetmask));

	if (nStaticIp != 0) {
		s_QueuedConfig.nStaticIp = nStaticIp;
	} else {
		s_QueuedConfig.nStaticIp = Network::Get()->GetIp();
	}

	if (nNetmask != 0) {
		s_QueuedConfig.nNetmask = nNetmask;
	} else {
		s_QueuedConfig.nNetmask = Network::Get()->GetNetmask();
	}

	s_QueuedConfig.nMask |= QueuedConfig::STATIC_IP;
	s_QueuedConfig.nMask |= QueuedConfig::NETMASK;

	DEBUG_EXIT
}

static void set_queued_default_route(const uint32_t nGatewayIp) {
	if (nGatewayIp != 0) {
		s_QueuedConfig.nGateway = nGatewayIp;
	} else {
		s_QueuedConfig.nGateway = Network::Get()->GetGatewayIp();
	}

	s_QueuedConfig.nMask |= QueuedConfig::GW;
}

static void set_queued_dhcp(const dhcp::Mode mode) {
	s_QueuedConfig.mode = mode;
	s_QueuedConfig.nMask |= QueuedConfig::DHCP;
}

static void set_queued_zeroconf() {
	s_QueuedConfig.nMask |= QueuedConfig::ZEROCONF;
}

static bool apply_queued_config() {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_QueuedConfig.nMask=%x, " IPSTR ", " IPSTR, s_QueuedConfig.nMask, IP2STR(s_QueuedConfig.nStaticIp), IP2STR(s_QueuedConfig.nNetmask));

	if (s_QueuedConfig.nMask == QueuedConfig::NONE) {
		DEBUG_EXIT
		return false;
	}

	if ((is_queued_mask_set(QueuedConfig::STATIC_IP)) || (is_queued_mask_set(QueuedConfig::NETMASK)) || (is_queued_mask_set(QueuedConfig::GW))) {
		// After SetIp all ip address might be zero.
		if (is_queued_mask_set(QueuedConfig::STATIC_IP)) {
			Network::Get()->SetIp(s_QueuedConfig.nStaticIp);
		}

		if (is_queued_mask_set(QueuedConfig::NETMASK)) {
			Network::Get()->SetNetmask(s_QueuedConfig.nNetmask);
		}

		if (is_queued_mask_set(QueuedConfig::GW)) {
			Network::Get()->SetGatewayIp(s_QueuedConfig.nGateway);
		}

		s_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	if (is_queued_mask_set(QueuedConfig::DHCP)) {
		if (s_QueuedConfig.mode == dhcp::Mode::ACTIVE) {
			Network::Get()->EnableDhcp();
		} else if (s_QueuedConfig.mode == dhcp::Mode::INACTIVE) {

		}

		s_QueuedConfig.mode = dhcp::Mode::UNKNOWN;
		s_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	if (is_queued_mask_set(QueuedConfig::ZEROCONF)) {
		Network::Get()->SetZeroconf();
		s_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return false;
}

/*
 * ANSI E1.37-2
 */

bool RDMHandler::CheckInterfaceID([[maybe_unused]] const struct TRdmMessageNoSc *pRdmDataIn) {
#if !defined(DMX_WORKSHOP_DEFECT)
	const auto nInterfaceID = static_cast<uint32_t>((pRdmDataIn->param_data[0] << 24)
			+ (pRdmDataIn->param_data[1] << 16)
			+ (pRdmDataIn->param_data[2] << 8) + pRdmDataIn->param_data[3]);

	if (nInterfaceID != Network::Get()->GetIfIndex()) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return false;
	}
#endif
	return true;
}

void RDMHandler::GetInterfaceList([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY
	// https://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
	const uint16_t nInterfaceHardwareType = 0x6; //	IEEE 802 Networks

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	const uint32_t nNetworkInterfaceId = Network::Get()->GetIfIndex();

	pRdmDataOut->param_data[0] = static_cast<uint8_t>(nNetworkInterfaceId >> 24);
	pRdmDataOut->param_data[1] = static_cast<uint8_t>(nNetworkInterfaceId >> 16);
	pRdmDataOut->param_data[2] = static_cast<uint8_t>(nNetworkInterfaceId >> 8);
	pRdmDataOut->param_data[3] = static_cast<uint8_t>(nNetworkInterfaceId);
	pRdmDataOut->param_data[4] = (nInterfaceHardwareType >> 8);
	pRdmDataOut->param_data[5] = nInterfaceHardwareType;

	pRdmDataOut->param_data_length = 6;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetInterfaceName([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage *>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);

	static const auto nLength = std::min(strlen(Network::Get()->GetIfName()), static_cast<size_t>(32));

	memcpy(reinterpret_cast<char *>(&pRdmDataOut->param_data[4]), Network::Get()->GetIfName(), nLength);

	pRdmDataOut->param_data_length = static_cast<uint8_t>(4 + nLength);

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetHardwareAddress([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	Network::Get()->MacAddressCopyTo(&pRdmDataOut->param_data[4]);

	pRdmDataOut->param_data_length = 10;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetDHCPMode([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	pRdmDataOut->param_data[4] = Network::Get()->IsDhcpUsed() ? 1 : 0;

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetDHCPMode([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	if ((!Network::Get()->IsDhcpKnown()) || (!Network::Get()->IsDhcpCapable())) {
		RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

		DEBUG_EXIT
		return;
	}

	const auto mode = static_cast<dhcp::Mode>(pRdmDataIn->param_data[4]);

	if ((mode == dhcp::Mode::ACTIVE) || mode == dhcp::Mode::INACTIVE) {
		set_queued_dhcp(mode);
		RespondMessageAck();

		DEBUG_EXIT
		return;
	}

	RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

	DEBUG_EXIT
}

void RDMHandler::GetNameServers([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);
	const auto nNameServerIndex = pRdmDataIn->param_data[0];

	if ((nNameServerIndex >= Network::Get()->GetNameServers()) || (nNameServerIndex >  2)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetNameServer(nNameServerIndex);
	const auto *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	pRdmDataOut->param_data[0] = nNameServerIndex;
	memcpy(&pRdmDataOut->param_data[1], p, 4);

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetZeroconf([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	pRdmDataOut->param_data[4] = Network::Get()->IsZeroconfCapable() ? (Network::Get()->IsZeroconfUsed()) : 0;

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetZeroconf([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	if (!Network::Get()->IsZeroconfCapable()) {
		RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

		DEBUG_EXIT
		return;
	}

	if (pRdmDataIn->param_data[4] == 1) {
		set_queued_zeroconf();
		RespondMessageAck();

		DEBUG_EXIT
		return;
	}

	if (pRdmDataIn->param_data[4] == 0) {
		set_queued_static_ip(0, 0);
		RespondMessageAck();

		DEBUG_EXIT
		return;
	}

	RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);

	DEBUG_EXIT
}

void RDMHandler::RenewDhcp([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	if (!Network::Get()->IsDhcpKnown()) {
		RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

		DEBUG_EXIT
		return;
	}

	if (!Network::Get()->IsDhcpUsed()) {
		RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

		DEBUG_EXIT
		return;
	}

	Network::Get()->EnableDhcp();

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetAddressNetmask([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	auto nIpAddress = Network::Get()->GetIp();
	const auto *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);
	pRdmDataOut->param_data[8] = static_cast<uint8_t>(Network::Get()->GetNetmaskCIDR());
	pRdmDataOut->param_data[9] = static_cast<uint8_t>(get_dhcp_mode());

	pRdmDataOut->param_data_length = 10;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetStaticAddress([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetIp();
	const auto *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);
	pRdmDataOut->param_data[8] = static_cast<uint8_t>(Network::Get()->GetNetmaskCIDR());

	pRdmDataOut->param_data_length = 9;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetStaticAddress([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 9) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);

		DEBUG_EXIT
		return;
	}

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	uint32_t nIpAddress;
	auto *p = reinterpret_cast<uint8_t*>(&nIpAddress);
	memcpy(p, &pRdmDataIn->param_data[4], 4);

	set_queued_static_ip(nIpAddress, net::cidr_to_netmask(pRdmDataIn->param_data[8]));

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::ApplyConfiguration([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	if (apply_queued_config()) { // Not Queuing -> Apply
		RespondMessageAck();

		DEBUG_EXIT
		return;
	}

	RespondMessageNack(E120_NR_FORMAT_ERROR);

	DEBUG_EXIT
}

void RDMHandler::GetDefaultRoute([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetGatewayIp();
	const auto *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);

	pRdmDataOut->param_data_length = 8;

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetDefaultRoute([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc *>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 8) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);

		DEBUG_EXIT
		return;
	}

	if (!CheckInterfaceID(pRdmDataIn)) {

		DEBUG_EXIT
		return;
	}

	uint32_t nIpAddress;
	auto *p = reinterpret_cast<uint8_t *>(&nIpAddress);
	memcpy(p, &pRdmDataIn->param_data[4], 4);

	set_queued_default_route(nIpAddress);

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetHostName([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pHostName = Network::Get()->GetHostName();
	HandleString(pHostName, static_cast<uint32_t>(strlen(pHostName)));

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetHostName([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length >= 64) {
		RespondMessageNack(E120_NR_HARDWARE_FAULT);

		DEBUG_EXIT
		return;
	}

	pRdmDataIn->param_data[pRdmDataIn->param_data_length] = '\0';

	Network::Get()->SetHostName(reinterpret_cast<const char*>(pRdmDataIn->param_data));

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::GetDomainName([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pDomainName = Network::Get()->GetDomainName();
	HandleString(pDomainName, static_cast<uint32_t>(strlen(pDomainName)));

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetDomainName([[maybe_unused]] bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);

	DEBUG_EXIT
}
