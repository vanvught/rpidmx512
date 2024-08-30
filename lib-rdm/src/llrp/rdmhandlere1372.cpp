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

#include "rdmhandler.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#include "network.h"

#include "debug.h"

enum {
	IPV4_UNCONFIGURED = 0x00000000,
	NO_DEFAULT_ROUTE = 0x00000000
};

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

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	strcpy(reinterpret_cast<char*>(&pRdmDataOut->param_data[4]), Network::Get()->GetIfName());

	pRdmDataOut->param_data_length = static_cast<uint8_t>(4 + strlen(Network::Get()->GetIfName()));

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

	const auto mode = static_cast<network::dhcp::Mode>(pRdmDataIn->param_data[4]);

	if ((mode == network::dhcp::Mode::ACTIVE) || mode == network::dhcp::Mode::INACTIVE) {
		Network::Get()->SetQueuedDhcp(mode);
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
		Network::Get()->SetQueuedZeroconf();
		RespondMessageAck();

		DEBUG_EXIT
		return;
	}

	if (pRdmDataIn->param_data[4] == 0) {
		Network::Get()->SetQueuedStaticIp(0, 0);
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
	pRdmDataOut->param_data[9] = static_cast<uint8_t>(Network::Get()->GetDhcpMode());

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

	Network::Get()->SetQueuedStaticIp(nIpAddress, network::cidr_to_netmask(pRdmDataIn->param_data[8]));

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

	if (Network::Get()->ApplyQueuedConfig()) { // Not Queuing -> Apply
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

	Network::Get()->SetQueuedDefaultRoute(nIpAddress);

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
