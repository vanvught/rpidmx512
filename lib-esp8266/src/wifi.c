/**
 * @file wifi.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "wifi.h"

#include "console.h"

#include "ap_params.h"
#include "network_params.h"
#include "fota.h"
#include "fota_params.h"

#include "util.h"

#if defined(HAVE_I2C)
 #include "oled.h"
#endif

static const char *WIFI_NOT_CONNECTED ALIGNED = "Wifi not connected";
static const char *STARTING_WIFI ALIGNED = "Starting Wifi ...";
static const char *CHANGING_TO_STATION_MODE ALIGNED = "Changing to Station mode ...";
static const char *WIFI_STARTED ALIGNED = "Wifi started";

static 	_wifi_mode opmode = WIFI_OFF;
static const char *ssid = NULL;

const bool wifi(const struct ip_info *info) {
	uint8_t mac_address[6] ALIGNED;
	char *ap_password = NULL;
	struct ip_info ip_config;
#if defined(HAVE_I2C)
	oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };
	const bool oled_connected =  oled_start(&oled_info);
#endif

	if (!wifi_detect()){
		(void) console_status(CONSOLE_YELLOW, WIFI_NOT_CONNECTED);
#if defined(HAVE_I2C)
		OLED_CONNECTED(oled_connected, oled_puts(&oled_info, WIFI_NOT_CONNECTED));
#endif
		return false;
	}

	(void) ap_params_init();
	ap_password = (char *) ap_params_get_password();

	(void) console_status(CONSOLE_YELLOW, STARTING_WIFI);
#if defined(HAVE_I2C)
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, STARTING_WIFI));
#endif

	wifi_ap_init(ap_password);

	printf("ESP8266 information\n");
	printf(" SDK      : %s\n", system_get_sdk_version());
	printf(" Firmware : %s\n\n", wifi_get_firmware_version());

	if (network_params_init()) {
		(void) console_status(CONSOLE_YELLOW, CHANGING_TO_STATION_MODE);
#if defined(HAVE_I2C)
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, CHANGING_TO_STATION_MODE));
#endif
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
		printf("WiFi mode : Station (AP: %s)\n", network_params_get_ssid());
	} else {
		printf("WiFi mode : Access Point (authenticate mode: %s)\n", *ap_password == '\0' ? "Open" : "WPA_WPA2_PSK");
	}

	if (wifi_get_macaddr(mac_address)) {
		printf(" MAC address : "MACSTR "\n", MAC2STR(mac_address));
	} else {
		(void) console_error("wifi_get_macaddr");
#if defined(HAVE_I2C)
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "E: wifi_get_macaddr"));
#endif
	}

	printf(" Hostname    : %s\n", wifi_get_hostname());

	if (wifi_get_ip_info(&ip_config)) {
		printf(" IP-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
		printf(" Netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
		printf(" Gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));
		if (opmode == WIFI_STA) {
			const _wifi_station_status status = wifi_station_get_connect_status();
			printf("      Status : %s\n", wifi_station_status(status));
			if (status != WIFI_STATION_GOT_IP) {
				(void) console_error("Not connected!");
#if defined(HAVE_I2C)
				if (oled_connected) {
					oled_set_cursor(&oled_info, 2, 0);
					(void) oled_puts(&oled_info, wifi_station_status(status));
					oled_set_cursor(&oled_info, 5, 0);
					(void) oled_printf(&oled_info, "SSID : %s\n", network_params_get_ssid());
					oled_status(&oled_info, "E: Not connected!");
				}
#endif
				for (;;)
					;
			}
		}
	} else {
		(void) console_error("wifi_get_ip_info");
#if defined(HAVE_I2C)
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "E: wifi_get_ip_info"));
#endif
	}

	if (fota_params_init()) {
#if defined(HAVE_I2C)
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "FOTA mode"));
#endif
		console_newline();
		fota(fota_params_get_server());
		for (;;)
			;
	}

	(void) console_status(CONSOLE_GREEN, WIFI_STARTED);
#if defined(HAVE_I2C)
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, WIFI_STARTED));
#endif

	memcpy((void *)info, (const void *)&ip_config, sizeof(struct ip_info));

	return true;
}

const char *wifi_get_ssid(void) {
	if (opmode == WIFI_STA ||opmode == WIFI_STA) {
		return ssid;
	} else {
		return NULL;
	}
}
