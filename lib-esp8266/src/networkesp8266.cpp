/**
 * @file networkesp8266.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>
#include <algorithm>
#include <cassert>

#include "networkesp8266.h"

#include "networkparams.h"

#include "apparams.h"
#include "networkparams.h"
#include "fotaparams.h"

#include "display.h"
#include "wificonst.h"

#include "esp8266.h"
#include "esp8266_cmd.h"

#include "debug.h"

extern void fota(const uint32_t);

static constexpr auto SDK_VERSION_MAX = 255;
static constexpr auto FIRMWARE_VERSION_MAX = 255;

static char sdk_version[SDK_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};
static char firmware_version[FIRMWARE_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};

extern "C" {
	int32_t hardware_get_mac_address(uint8_t *mac_address);
}

NetworkESP8266::NetworkESP8266() {
	DEBUG_ENTRY

	strcpy(m_aIfName, "wlan0");

	DEBUG_EXIT
}

NetworkESP8266::~NetworkESP8266() {
	DEBUG_ENTRY

	DEBUG_EXIT
}


void NetworkESP8266::Init() {
	if (!Start()) {
		for (;;)
			;
	}

	m_IsInitDone = true;
}

int32_t NetworkESP8266::Begin(uint16_t nPort) {
	esp8266_write_4bits(CMD_WIFI_UDP_BEGIN);

	esp8266_write_halfword(nPort);

	return 0;
}

int32_t NetworkESP8266::End(__attribute__((unused)) uint16_t nPort) {
	return 0;
}

void NetworkESP8266::MacAddressCopyTo(uint8_t* pMacAddress) {
	assert(pMacAddress != 0);

	if (m_IsInitDone) {
		memcpy(pMacAddress, m_aNetMacaddr , NETWORK_MAC_SIZE);
	} else {
		hardware_get_mac_address(pMacAddress);
	}
}

void NetworkESP8266::JoinGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	esp8266_write_4bits(CMD_WIFI_UDP_JOIN_GROUP);

	esp8266_write_word(nIp);
}

uint16_t NetworkESP8266::RecvFrom(__attribute__((unused)) int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *from_ip, uint16_t* from_port) {
	assert(pBuffer != nullptr);
	assert(from_ip != nullptr);
	assert(from_port != nullptr);

	esp8266_write_4bits(CMD_WIFI_UDP_RECEIVE);

	auto nBytesReceived = esp8266_read_halfword();

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

void NetworkESP8266::SendTo(__attribute__((unused)) int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t to_ip, uint16_t remote_port) {
	assert(pBuffer != nullptr);

	esp8266_write_4bits(CMD_WIFI_UDP_SEND);

	esp8266_write_halfword(nLength);
	esp8266_write_word(to_ip);
	esp8266_write_halfword(remote_port);
	esp8266_write_bytes(reinterpret_cast<const uint8_t *>(pBuffer), nLength);
}

bool NetworkESP8266::Start() {
	struct ip_info ip_config;

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

	if (networkParams.Load()) {
		networkParams.Dump();

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
	esp8266_read_bytes(m_aNetMacaddr, NETWORK_MAC_SIZE);

	printf(" MAC address : " MACSTR "\n", MAC2STR(m_aNetMacaddr));

	uint16_t nLength = HOST_NAME_MAX;

	esp8266_write_4bits(CMD_WIFI_HOST_NAME);
	esp8266_read_str(m_aHostName, &nLength);

	printf(" Hostname    : %s\n", m_aHostName);

	esp8266_write_4bits(CMD_WIFI_IP_INFO);
	esp8266_read_bytes(reinterpret_cast<const uint8_t *>(&ip_config), sizeof(struct ip_info));

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
		console_newline();
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

void NetworkESP8266::StationCreate(const char *pSsid, const char *pPassword) {
	assert(pSsid != nullptr);
	assert(pPassword != nullptr);

	esp8266_write_4bits(CMD_WIFI_MODE_STA);

	esp8266_write_str(pSsid);
	esp8266_write_str(pPassword);

	m_IsDhcpUsed = true;
}

void NetworkESP8266::StationCreate(const char *pSsid, const char *pPassword, const struct ip_info *pInfo) {
	assert(pSsid != nullptr);
	assert(pPassword != nullptr);
	assert(pInfo != nullptr);

	esp8266_write_4bits(CMD_WIFI_MODE_STA_IP);

	esp8266_write_str(pSsid);
	esp8266_write_str(pPassword);

	esp8266_write_bytes(reinterpret_cast<const uint8_t *>(pInfo), sizeof(struct ip_info));

	m_IsDhcpUsed = false;
}


_wifi_station_status NetworkESP8266::StationGetConnectStatus() {
	esp8266_write_4bits(CMD_WIFI_MODE_STA_STATUS);
	return static_cast<_wifi_station_status>(esp8266_read_byte());
}

const char *NetworkESP8266::StationStatus(_wifi_station_status status) {
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

void NetworkESP8266::ApCreate(const char *pPassword) {
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

const char *NetworkESP8266::GetSystemSdkVersion() {
	uint16_t nLength = SDK_VERSION_MAX;

	esp8266_write_4bits(CMD_SYSTEM_SDK_VERSION);
	esp8266_read_str(sdk_version, &nLength);

	return sdk_version;
}

const char *NetworkESP8266::GetFirmwareVersion() {
	uint16_t nLength = FIRMWARE_VERSION_MAX;

	esp8266_write_4bits(CMD_SYSTEM_FIRMWARE_VERSION);
	esp8266_read_str(firmware_version, &nLength);

	return firmware_version;
}
