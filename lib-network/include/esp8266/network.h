/**
 * @file network.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ESP8266_NETWORK_H_
#define ESP8266_NETWORK_H_

#if !defined(ESP8266)
# error
#endif

#include <cstdint>
#include <net/if.h>

struct ip_addr {
    uint32_t addr;
};

typedef struct ip_addr ip_addr_t;

struct IpInfo {
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

class Network {
public:
	Network();
	~Network();

	void Init();

	int32_t Begin(uint16_t nPort) ;
	int32_t End(uint16_t nPort) ;

	void MacAddressCopyTo(uint8_t *pMacAddress);

	void JoinGroup(int32_t nHandle, uint32_t nIp);
	void LeaveGroup([[maybe_unused]] int32_t nHandle, [[maybe_unused]] uint32_t nIp)  {
		// Not supported
	}

	uint32_t RecvFrom(int32_t nHandle, void *pBuffer, uint32_t nLength, uint32_t *pFromIp, uint16_t *pFromPort);
	uint32_t RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort);
	void SendTo(int32_t nHandle, const void *pBuffer, uint32_t nLength, uint32_t nToIp, uint16_t nRemotePort) ;

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

	uint32_t GetSecondaryIp() const {
		return m_nLocalIp;
	}

	void SetIp([[maybe_unused]] uint32_t nIp) {
	}

	uint32_t GetIp() const {
		return m_nLocalIp;
	}

	void SetNetmask([[maybe_unused]] uint32_t nNetmask) {
	}

	uint32_t GetNetmask() const {
		return m_nNetmask;
	}

	const char *GetHostName() const {
		return m_aHostName;
	}

	void SetGatewayIp([[maybe_unused]] uint32_t nGatewayIp) {
	}

	uint32_t GetGatewayIp() const {
		return m_nGatewayIp;
	}

	bool SetZeroconf() {
		return false;
	}

	uint32_t GetBroadcastIp() const {
		return m_nLocalIp | ~m_nNetmask;
	}

	bool IsValidIp(uint32_t nIp) {
		return (m_nLocalIp & m_nNetmask) == (nIp & m_nNetmask);
	}


	bool IsDhcpCapable() const {
		return m_IsDhcpCapable;
	}

	bool IsDhcpUsed() const {
		return m_IsDhcpUsed;
	}

	 bool IsDhcpKnown() const {
		return true;
	}

	bool EnableDhcp() {
		return false;
	}

	static Network *Get() {
		return s_pThis;
	}

private:
	bool Start();
	const char *GetSystemSdkVersion();
	const char *GetFirmwareVersion();
	void ApCreate(const char *pPassword);
	void StationCreate(const char *pSsid, const char *pPassword) ;
	void StationCreate(const char *pSsid, const char *pPassword, const struct IpInfo *pInfo);
	_wifi_station_status StationGetConnectStatus();
	const char *StationStatus(_wifi_station_status status);

private:
	bool m_IsInitDone { false };
	bool m_IsDhcpCapable { true };
	bool m_IsDhcpUsed { false };
	uint32_t m_nLocalIp { 0 };
	uint32_t m_nGatewayIp { 0 };
	uint32_t m_nNetmask { 0 };
	char m_aHostName[network::HOSTNAME_SIZE];
	char m_aDomainName[network::DOMAINNAME_SIZE];
	uint8_t m_aNetMacaddr[network::MAC_SIZE];
	char m_aIfName[IFNAMSIZ];
	_wifi_mode m_Mode { WIFI_OFF };
	bool m_isApOpen { true };
	char *m_pSSID { nullptr };

	static Network *s_pThis;
};

#endif /* ESP8266_NETWORK_H_ */
