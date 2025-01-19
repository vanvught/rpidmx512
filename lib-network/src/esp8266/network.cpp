/**
 * @file network.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "network.h"

#include "networkparams.h"

#include "apparams.h"
#include "networkparams.h"
#include "fotaparams.h"
#include "wificonst.h"

#include "esp8266.h"
#include "esp8266_cmd.h"

#include "./../../lib-display/include/display.h"

#include "debug.h"

extern void fota(const uint32_t);

static constexpr auto SDK_VERSION_MAX = 255;
static constexpr auto FIRMWARE_VERSION_MAX = 255;

static char sdk_version[SDK_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};
static char firmware_version[FIRMWARE_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};

void mac_address_get(uint8_t paddr[]);

static net::UdpCallbackFunctionPtr s_callback;

Network::Network() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	strcpy(m_aIfName, "wlan0");
}

Network::~Network() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void Network::Init() {
	if (!Start()) {
		for (;;)
			;
	}

	m_IsInitDone = true;
}

int32_t Network::Begin(uint16_t nPort, net::UdpCallbackFunctionPtr callback) {
	s_callback = callback;

	esp8266_write_4bits(CMD_WIFI_UDP_BEGIN);

	esp8266_write_halfword(nPort);

	return 0;
}

int32_t Network::End([[maybe_unused]] uint16_t nPort) {
	return 0;
}

void Network::MacAddressCopyTo(uint8_t* pMacAddress) {
	assert(pMacAddress != 0);

	if (m_IsInitDone) {
		memcpy(pMacAddress, m_aNetMacaddr , net::MAC_SIZE);
	} else {
		mac_address_get(pMacAddress);
	}
}

void Network::JoinGroup([[maybe_unused]] int32_t nHandle, uint32_t nIp) {
	esp8266_write_4bits(CMD_WIFI_UDP_JOIN_GROUP);

	esp8266_write_word(nIp);
}

uint32_t Network::RecvFrom([[maybe_unused]] int32_t nHandle, void *pBuffer, uint32_t nLength, uint32_t *from_ip, uint16_t* from_port) {
	assert(pBuffer != nullptr);
	assert(from_ip != nullptr);
	assert(from_port != nullptr);

	esp8266_write_4bits(CMD_WIFI_UDP_RECEIVE);

	uint32_t nBytesReceived = esp8266_read_halfword();

	if (nBytesReceived != 0) {
		*from_ip = esp8266_read_word();
		*from_port = esp8266_read_halfword();
		esp8266_read_bytes(reinterpret_cast<uint8_t *>(pBuffer), std::min(nBytesReceived, nLength));
	} else {
		*from_ip = 0;
		*from_port = 0;
	}

	return nBytesReceived;
}

#define MAX_SEGMENT_LENGTH		1400

static uint8_t s_ReadBuffer[MAX_SEGMENT_LENGTH];

uint32_t  Network::RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort) {
	*ppBuffer = &s_ReadBuffer;
	return RecvFrom(nHandle, s_ReadBuffer, MAX_SEGMENT_LENGTH, pFromIp, pFromPort);
}

void Network::Run() {
	if (s_callback != nullptr) {
		uint32_t nFromIp;
		uint16_t nFromPort;
		auto nSize = RecvFrom(0, s_ReadBuffer, MAX_SEGMENT_LENGTH, &nFromIp, &nFromPort);
		if (nSize > 0) {
			s_callback(s_ReadBuffer, nSize, nFromIp, nFromPort);
		}
	}
}

void Network::SendTo([[maybe_unused]] int32_t nHandle, const void *pBuffer, uint32_t nLength, uint32_t to_ip, uint16_t remote_port) {
	assert(pBuffer != nullptr);

	esp8266_write_4bits(CMD_WIFI_UDP_SEND);

	esp8266_write_halfword((nLength));
	esp8266_write_word(to_ip);
	esp8266_write_halfword(remote_port);
	esp8266_write_bytes(reinterpret_cast<const uint8_t *>(pBuffer), nLength);
}

bool Network::Start() {
	struct IpInfo ip_config;

	if (!esp8266_detect()){
		Display::Get()->TextStatus(WifiConst::MSG_ERROR_BOARD_NOT_CONNECTED);
		return false;
	}

	ApParams apParams;

	if (apParams.Load()) {
		apParams.Dump();
	}

	Display::Get()->TextStatus(WifiConst::MSG_STARTING);

	const auto *ap_password = apParams.GetPassword();
	ApCreate(ap_password);

	printf("ESP8266 information\n");
	printf(" SDK      : %s\n", GetSystemSdkVersion());
	printf(" Firmware : %s\n", GetFirmwareVersion());

	NetworkParams networkParams;
	networkParams.Load();

	Display::Get()->TextStatus(WifiConst::MSG_CHANGING_TO_STATION_MODE);

	m_pSSID = const_cast<char *>(networkParams.GetSSid());

	if (networkParams.isDhcpUsed()) {
		StationCreate(m_pSSID, networkParams.GetPassword());
	} else {
		ip_config.ip.addr = networkParams.GetIpAddress();
		ip_config.netmask.addr = networkParams.GetNetMask();
		ip_config.gw.addr = networkParams.GetDefaultGateway();
		StationCreate(m_pSSID, networkParams.GetPassword(), &ip_config);
	}

	esp8266_write_4bits(CMD_WIFI_MODE);
	m_Mode = static_cast<_wifi_mode>(esp8266_read_byte());

	if (m_Mode == WIFI_STA) {
		Display::Get()->Printf(1, "SSID : %s", m_pSSID);
		printf("WiFi mode : Station (AP: %s)\n", m_pSSID);
	} else {
		printf("WiFi mode : Access Point (authenticate mode: %s)\n", *ap_password == '\0' ? "Open" : "WPA_WPA2_PSK");
	}

	esp8266_write_4bits(CMD_WIFI_MAC_ADDRESS);
	esp8266_read_bytes(m_aNetMacaddr, net::MAC_SIZE);

	printf(" MAC address : " MACSTR "\n", MAC2STR(m_aNetMacaddr));

	uint32_t nLength = HOST_NAME_MAX;

	esp8266_write_4bits(CMD_WIFI_HOST_NAME);
	esp8266_read_str(m_aHostName, &nLength);

	printf(" Hostname    : %s\n", m_aHostName);

	esp8266_write_4bits(CMD_WIFI_IP_INFO);
	esp8266_read_bytes(reinterpret_cast<const uint8_t *>(&ip_config), sizeof(struct IpInfo));

	m_nLocalIp = ip_config.ip.addr;
	m_nNetmask = ip_config.netmask.addr;
	m_nGatewayIp = ip_config.netmask.addr;

	printf(" IP-address  : " IPSTR "\n", IP2STR(m_nLocalIp));
	printf(" Netmask     : " IPSTR "\n", IP2STR(m_nNetmask));
	printf(" Gateway     : " IPSTR "\n", IP2STR(m_nGatewayIp));

	if (m_Mode == WIFI_STA) {
		const auto status = StationGetConnectStatus();
		printf(" Status : %s\n", StationStatus(status));

		if (status != WIFI_STATION_GOT_IP) {
			Display::Get()->TextStatus(WifiConst::MSG_ERROR_WIFI_NOT_CONNECTED);
			for (;;)
				;
		}
	}

	FotaParams fotaParams;

	if (fotaParams.Load()) {
		fotaParams.Dump();

		Display::Get()->TextStatus(WifiConst::MSG_FOTA_MODE);
		console_putc('\n');
		fota(fotaParams.GetServerIp());

		for (;;)
			;
	}

	Display::Get()->TextStatus(WifiConst::MSG_STARTED);

	return true;
}

/*
 * Wifi Station functions
 */

void Network::StationCreate(const char *pSsid, const char *pPassword) {
	assert(pSsid != nullptr);
	assert(pPassword != nullptr);

	esp8266_write_4bits(CMD_WIFI_MODE_STA);

	esp8266_write_str(pSsid);
	esp8266_write_str(pPassword);

	m_IsDhcpUsed = true;
}

void Network::StationCreate(const char *pSsid, const char *pPassword, const struct IpInfo *pInfo) {
	assert(pSsid != nullptr);
	assert(pPassword != nullptr);
	assert(pInfo != nullptr);

	esp8266_write_4bits(CMD_WIFI_MODE_STA_IP);

	esp8266_write_str(pSsid);
	esp8266_write_str(pPassword);

	esp8266_write_bytes(reinterpret_cast<const uint8_t *>(pInfo), sizeof(struct IpInfo));

	m_IsDhcpUsed = false;
}


_wifi_station_status Network::StationGetConnectStatus() {
	esp8266_write_4bits(CMD_WIFI_MODE_STA_STATUS);
	return static_cast<_wifi_station_status>(esp8266_read_byte());
}

const char *Network::StationStatus(_wifi_station_status status) {
	switch (status) {
	case WIFI_STATION_IDLE:
		return "ESP8266 station idle";
		//break;
	case WIFI_STATION_CONNECTING:
		return "ESP8266 station is connecting to AP";
		//break;
	case WIFI_STATION_WRONG_PASSWORD:
		return "The password is wrong";
		//break;
	case WIFI_STATION_NO_AP_FOUND:
		return "ESP8266 station can not find the target AP";
		//break;
	case WIFI_STATION_CONNECT_FAIL:
		return "ESP8266 station fail to connect to AP";
		//break;
	case WIFI_STATION_GOT_IP:
		return "ESP8266 station got IP address from AP";
		//break;
	default:
		return "Unknown Status";
		//break;
	}
}

/*
 * WiFi AP function
 */

void Network::ApCreate(const char *pPassword) {
	esp8266_init();

	esp8266_write_4bits(CMD_WIFI_MODE_AP);
	esp8266_write_str(pPassword == nullptr ? "" : pPassword);

	if (pPassword != nullptr) {
		if (*pPassword != '\0') {
			m_isApOpen = false;
		}
	}

	return;
}

/*
 * Getter's
 */

const char *Network::GetSystemSdkVersion() {
	uint32_t nLength = SDK_VERSION_MAX;

	esp8266_write_4bits(CMD_SYSTEM_SDK_VERSION);
	esp8266_read_str(sdk_version, &nLength);

	return sdk_version;
}

const char *Network::GetFirmwareVersion() {
	uint32_t nLength = FIRMWARE_VERSION_MAX;

	esp8266_write_4bits(CMD_SYSTEM_FIRMWARE_VERSION);
	esp8266_read_str(firmware_version, &nLength);

	return firmware_version;
}
