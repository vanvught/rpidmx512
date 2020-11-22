/**
 * @file wifi.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "wifi.h"

#include "ap_params.h"
#include "network_params.h"
#include "fota.h"
#include "fota_params.h"

#include "display.h"
#include "wificonst.h"

static 	_wifi_mode opmode = WIFI_OFF;
static const char *ssid = nullptr;

bool wifi(struct ip_info *info) {
	uint8_t mac_address[6];
	struct ip_info ip_config;

	if (!wifi_detect()){
		Display::Get()->TextStatus(WifiConst::MSG_ERROR_BOARD_NOT_CONNECTED);
		return false;
	}

	ap_params_init();

	const char *ap_password = ap_params_get_password();

	Display::Get()->TextStatus(WifiConst::MSG_STARTING);

	wifi_ap_init(ap_password);

	printf("ESP8266 information\n");
	printf(" SDK      : %s\n", system_get_sdk_version());
	printf(" Firmware : %s\n", wifi_get_firmware_version());

	if (network_params_init()) {
		Display::Get()->TextStatus(WifiConst::MSG_CHANGING_TO_STATION_MODE);

		ssid = network_params_get_ssid();
		if (network_params_is_use_dhcp()) {
			wifi_station(ssid, network_params_get_password());
		} else {
			ip_config.ip.addr = network_params_get_ip_address();
			ip_config.netmask.addr = network_params_get_net_mask();
			ip_config.gw.addr = network_params_get_default_gateway();
			wifi_station_ip(ssid, network_params_get_password(), &ip_config);
		}
	}

	opmode = wifi_get_opmode();

	if (opmode == WIFI_STA) {
		Display::Get()->Printf(1, "SSID : %s", network_params_get_ssid());
		printf("WiFi mode : Station (AP: %s)\n", network_params_get_ssid());
	} else {
		printf("WiFi mode : Access Point (authenticate mode: %s)\n", *ap_password == '\0' ? "Open" : "WPA_WPA2_PSK");
	}

	if (wifi_get_macaddr(mac_address)) {
		printf(" MAC address : " MACSTR "\n", MAC2STR(mac_address));
	} else {
		Display::Get()->TextStatus(WifiConst::MSG_ERROR_GET_MAC);
	}

	printf(" Hostname    : %s\n", wifi_get_hostname());

	if (wifi_get_ip_info(&ip_config)) {
		printf(" IP-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
		printf(" Netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
		printf(" Gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));

		if (opmode == WIFI_STA) {
			const auto status = wifi_station_get_connect_status();
			printf(" Status : %s\n", wifi_station_status(status));

			if (status != WIFI_STATION_GOT_IP) {
				Display::Get()->TextStatus(WifiConst::MSG_ERROR_WIFI_NOT_CONNECTED);
				for (;;)
					;
			}
		}
	} else {
		Display::Get()->TextStatus(WifiConst::MSG_ERROR_GET_IP);
	}

	if (fota_params_init()) {
		Display::Get()->TextStatus(WifiConst::MSG_FOTA_MODE);
		console_newline();
		fota(fota_params_get_server());
		for (;;)
			;
	}

	Display::Get()->TextStatus(WifiConst::MSG_STARTED);

	memcpy(info, &ip_config, sizeof(struct ip_info));

	return true;
}

const char *wifi_get_ssid(void) {
	if (opmode == WIFI_STA ||opmode == WIFI_STA) {
		return ssid;
	} else {
		return nullptr;
	}
}
