/**
 * @file networkesp8266.h
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

#ifndef NETWORKESP8266_H_
#define NETWORKESP8266_H_

#include <cstdint>

#include "network.h"

struct ip_addr {
    uint32_t addr;
};

typedef struct ip_addr ip_addr_t;

struct ip_info {
    struct ip_addr ip;
    struct ip_addr netmask;
    struct ip_addr gw;
};

typedef enum wifi_mode {
	WIFI_OFF = 0,
	WIFI_STA = 1,
	WIFI_AP = 2,
	WIFI_AP_STA = 3
} _wifi_mode;

typedef enum wifi_station_status {
	WIFI_STATION_IDLE = 0,        /**< ESP8266 station idle */
	WIFI_STATION_CONNECTING,      /**< ESP8266 station is connecting to AP*/
	WIFI_STATION_WRONG_PASSWORD,  /**< the password is wrong*/
	WIFI_STATION_NO_AP_FOUND,     /**< ESP8266 station can not find the target AP*/
	WIFI_STATION_CONNECT_FAIL,    /**< ESP8266 station fail to connect to AP*/
	WIFI_STATION_GOT_IP           /**< ESP8266 station got IP address from AP*/
} _wifi_station_status;

typedef enum wifiphy_phy_mode {
	WIFI_PHY_MODE_11B = 1,
	WIFI_PHY_MODE_11G = 2,
	WIFI_PHY_MODE_11N = 3
} _wifiphy_phy_mode;

#define STATION_IF	0x00
#define SOFTAP_IF	0x01

#define HOST_NAME_MAX			255

class NetworkESP8266 final : public Network {
public:
	NetworkESP8266();
	~NetworkESP8266();

	void Init();

	int32_t Begin(uint16_t nPort) override ;
	int32_t End(uint16_t nPort) override ;

	void MacAddressCopyTo(uint8_t *pMacAddress);

	void JoinGroup(int32_t nHandle, uint32_t nIp);
	void LeaveGroup(__attribute__((unused)) int32_t nHandle, __attribute__((unused)) uint32_t nIp)  override {
		// Not supported
	}

	uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *pFromIp, uint16_t *pFromPort) override ;
	void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t nToIp, uint16_t nRemotePort) override ;

	void Print() {
	}

	_wifi_mode GetOpmode() const {
		return m_Mode;
	}

	bool IsApOpen() const {
		return m_isApOpen;
	}

	const char *GetSsid() const {
		return m_pSSID;
	}

	void SetIp(__attribute__((unused)) uint32_t nIp) {
	}

	void SetNetmask(__attribute__((unused)) uint32_t nNetmask) {
	}

	bool SetZeroconf() {
		return false;
	}

	bool EnableDhcp() {
		return false;
	}

private:
	bool Start();
	const char *GetSystemSdkVersion();
	const char *GetFirmwareVersion();
	void ApCreate(const char *pPassword);
	void StationCreate(const char *pSsid, const char *pPassword) ;
	void StationCreate(const char *pSsid, const char *pPassword, const struct ip_info *pInfo);
	_wifi_station_status StationGetConnectStatus();
	const char *StationStatus(_wifi_station_status status);

private:
	bool m_IsInitDone { false };
	_wifi_mode m_Mode { WIFI_OFF };
	bool m_isApOpen { true };
	char *m_pSSID { nullptr };
};

#endif /* NETWORKESP8266_H_ */
