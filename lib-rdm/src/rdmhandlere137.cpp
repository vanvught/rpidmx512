/**
 * @file rdmhandlere137.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <string.h>

#include "rdmhandler.h"

#include "rdmidentify.h"
#include "rdm_e120.h"

#include "network.h"

enum TDHCPMode {
	DHCP_STATUS_INACTIVE = 0x00,	///< The IP address was not obtained via DHCP
	DHCP_STATUS_ACTIVE = 0x01,		///< The IP address was obtained via DHCP
	DHCP_STATUS_UNKNOWN = 0x02		///< The system cannot determine if the address was obtained via DHCP
};

enum {
	IPV4_UNCONFIGURED = 0x00000000,
	NO_DEFAULT_ROUTE = 0x00000000
};

/*
 * ANSI E1.37-1
 */

void RDMHandler::GetIdentifyMode(uint16_t nSubDevice) {
	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 1;

	pRdmDataOut->param_data[0] = RDMIdentify::Get()->GetMode();

	RespondMessageAck();
}

void RDMHandler::SetIdentifyMode(bool IsBroadcast, uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *rdm_command = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != 0) && (rdm_command->param_data[0] != 0xFF)) {
		RespondMessageNack( E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	RDMIdentify::Get()->SetMode(static_cast<TRdmIdentifyMode>(rdm_command->param_data[0]));

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

/*
 * ANSI E1.37-2
 */

bool RDMHandler::CheckInterfaceID(const struct TRdmMessageNoSc *pRdmDataIn) {
#if !defined(DMX_WORKSHOP_DEFECT)
	const uint32_t nInterfaceID = (pRdmDataIn->param_data[0] << 24)
			+ (pRdmDataIn->param_data[1] << 16)
			+ (pRdmDataIn->param_data[2] << 8) + pRdmDataIn->param_data[3];

	if (nInterfaceID != Network::Get()->GetIfIndex()) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return false;
	}
#endif
	return true;
}

#define CHECK_INTERFACE_ID if(!CheckInterfaceID) { return; }

void RDMHandler::GetInterfaceList(uint16_t nSubDevice) {
	// https://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
	const uint16_t nInterfaceHardwareType = 0x6; //	IEEE 802 Networks

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	const uint32_t nNetworkInterfaceId = Network::Get()->GetIfIndex();

	pRdmDataOut->param_data[0] = (nNetworkInterfaceId >> 24);
	pRdmDataOut->param_data[1] = (nNetworkInterfaceId >> 16);
	pRdmDataOut->param_data[2] = (nNetworkInterfaceId >> 8);
	pRdmDataOut->param_data[3] = nNetworkInterfaceId;
	pRdmDataOut->param_data[4] = (nInterfaceHardwareType >> 8);
	pRdmDataOut->param_data[5] = nInterfaceHardwareType;

	pRdmDataOut->param_data_length = 6;

	RespondMessageAck();
}

void RDMHandler::GetInterfaceName(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	strcpy(reinterpret_cast<char*>(&pRdmDataOut->param_data[4]), Network::Get()->GetIfName());

	pRdmDataOut->param_data_length = 4 + strlen(Network::Get()->GetIfName());

	RespondMessageAck();
}

void RDMHandler::GetHardwareAddress(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	Network::Get()->MacAddressCopyTo(&pRdmDataOut->param_data[4]);

	pRdmDataOut->param_data_length = 10;

	RespondMessageAck();
}

void RDMHandler::GetDHCPMode(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	pRdmDataOut->param_data[4] = Network::Get()->IsDhcpUsed() ? 1 : 0;

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();
}

void RDMHandler::SetDHCPMode(bool IsBroadcast, uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	if (pRdmDataIn->param_data[4] == 1) {
		Network::Get()->EnableDhcp();
	}

	RespondMessageAck();
}

void RDMHandler::GetNameServers(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);
	const uint16_t nNameServerIndex = pRdmDataIn->param_data[0];

	if (nNameServerIndex >  2) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	// TODO The Network class does not have GetNameServers

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memset(&pRdmDataOut->param_data[1], 0, 4);

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();
}

void RDMHandler::GetZeroconf(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	pRdmDataOut->param_data[4] = 0;

	pRdmDataOut->param_data_length = 5;

	RespondMessageAck();
}

void RDMHandler::SetZeroconf(bool IsBroadcast, uint16_t nSubDevice) {
	RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);
}

void RDMHandler::GetAddressNetmask(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetIp();
	const uint8_t *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);
	pRdmDataOut->param_data[8] = Network::Get()->GetNetmaskCIDR();
	pRdmDataOut->param_data[9] = Network::Get()->IsDhcpKnown() ? Network::Get()->IsDhcpUsed() : DHCP_STATUS_UNKNOWN;

	pRdmDataOut->param_data_length = 10;

	RespondMessageAck();
}

void RDMHandler::GetStaticAddress(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetIp();
	const uint8_t *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);
	pRdmDataOut->param_data[8] = Network::Get()->GetNetmaskCIDR();

	pRdmDataOut->param_data_length = 9;

	RespondMessageAck();
}

void RDMHandler::SetStaticAddress(bool IsBroadcast, uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 9) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	 uint32_t nIpAddress;
	uint8_t *p = reinterpret_cast<uint8_t*>(&nIpAddress);
	 memcpy(p, &pRdmDataIn->param_data[4], 4);

	if (Network::Get()->SetStaticIp(true, nIpAddress, Network::CIDRToNetmask(pRdmDataIn->param_data[8]))) {
		RespondMessageAck();
	}

	RespondMessageNack(E120_NR_FORMAT_ERROR);
}

void RDMHandler::ApplyConfiguration(bool IsBroadcast, uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	if (Network::Get()->SetStaticIp(false, 0, 0)) { // Not Queuing -> Apply
		RespondMessageAck();
	}

	RespondMessageNack(E120_NR_FORMAT_ERROR);
}

void RDMHandler::GetDefaultRoute(uint16_t nSubDevice) {
	const struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (!CheckInterfaceID(pRdmDataIn)) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	uint32_t nIpAddress = Network::Get()->GetGatewayIp();
	const uint8_t *p = reinterpret_cast<const uint8_t*>(&nIpAddress);

	memcpy(&pRdmDataOut->param_data[0], &pRdmDataIn->param_data[0], 4);
	memcpy(&pRdmDataOut->param_data[4], p, 4);

	pRdmDataOut->param_data_length = 8;

	RespondMessageAck();
}

void RDMHandler::SetDefaultRoute(bool IsBroadcast, uint16_t nSubDevice) {
	RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);
}

void RDMHandler::GetHostName(uint16_t nSubDevice) {
	const char *pHostName = Network::Get()->GetHostName();
	HandleString(pHostName, strlen(pHostName));

	RespondMessageAck();
}

void RDMHandler::SetHostName(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length >= 64) {
		RespondMessageNack(E120_NR_HARDWARE_FAULT);
		return;
	}

	pRdmDataIn->param_data[pRdmDataIn->param_data_length] = '\0';

	Network::Get()->SetHostName(reinterpret_cast<const char*>(pRdmDataIn->param_data));

	RespondMessageAck();
}

void RDMHandler::GetDomainName(uint16_t nSubDevice) {
	const char *pDomainName = Network::Get()->GetDomainName();
	HandleString(pDomainName, strlen(pDomainName));

	RespondMessageAck();
}

void RDMHandler::SetDomainName(bool IsBroadcast, uint16_t nSubDevice) {
	RespondMessageNack(E137_2_NR_ACTION_NOT_SUPPORTED);
}
