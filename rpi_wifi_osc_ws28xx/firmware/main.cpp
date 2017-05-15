/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdint.h>

#include "hardware.h"
#include "led.h"
#include "console.h"

#include "oscparams.h"
#include "oscws28xx.h"

#include "deviceparams.h"

#include "wifi.h"
#include "udp.h"
#include "ap_params.h"
#include "network_params.h"

#include "fota.h"
#include "fota_params.h"

#include "oled.h"

#include "software_version.h"

#define OLED_CONNECTED(b,f)	\
do {						\
	if(b) {					\
		f;					\
	}						\
} while (0);

extern "C" {

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

void notmain(void) {
	uint8_t mac_address[6];
	struct ip_info ip_config;
	DeviceParams deviceparms;
	OSCParams oscparms;
	uint16_t incoming_port;
	uint16_t outgoing_port;

	oled_info_t oled_info;
	bool oled_connected;

	hardware_init();

	oled_info.slave_address = 0;
	oled_info.type = OLED_PANEL_128x64;

	oled_connected = oled_start(&oled_info);

	(void) oscparms.Load();
	(void) deviceparms.Load();

	incoming_port = oscparms.GetIncomingPort();
	outgoing_port = oscparms.GetOutgoingPort();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("WiFi OSC Pixel controller, Incoming port: %d, Outgoing port: %d", incoming_port, outgoing_port);

	OLED_CONNECTED(oled_connected, oled_puts(&oled_info, "WiFi OSC Pixel"));

	console_set_top_row(3);

	(void) ap_params_init();
	const char *ap_password = ap_params_get_password();

	hardware_watchdog_init();

	console_status(CONSOLE_YELLOW, "Starting Wifi ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting Wifi ..."));

	wifi_init(ap_password);

	hardware_watchdog_stop();

	printf("ESP8266 information\n");
	printf(" SDK      : %s\n", system_get_sdk_version());
	printf(" Firmware : %s\n\n", wifi_get_firmware_version());

	if (network_params_init()) {
		console_status(CONSOLE_YELLOW, "Changing to Station mode ...");
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Changing to Station mode ..."));
		if (network_params_is_use_dhcp()) {
			wifi_station(network_params_get_ssid(), network_params_get_password());
		} else {
			ip_config.ip.addr = network_params_get_ip_address();
			ip_config.netmask.addr = network_params_get_net_mask();
			ip_config.gw.addr = network_params_get_default_gateway();
			wifi_station_ip(network_params_get_ssid(), network_params_get_password(), &ip_config);
		}
	}

	const _wifi_mode opmode = wifi_get_opmode();

	if (opmode == WIFI_STA) {
		printf("WiFi mode : Station\n");
	} else {
		printf("WiFi mode : Access Point (authenticate mode : %s)\n", *ap_password == '\0' ? "Open" : "WPA_WPA2_PSK");
	}

	if (wifi_get_macaddr(mac_address)) {
		printf(" MAC address : "MACSTR "\n", MAC2STR(mac_address));
	} else {
		console_error("wifi_get_macaddr");
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "E: wifi_get_macaddr"));
	}

	printf(" Hostname    : %s\n", wifi_station_get_hostname());

	if (wifi_get_ip_info(&ip_config)) {
		printf(" IP-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
		printf(" Netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
		printf(" Gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));
		if (opmode == WIFI_STA) {
			const _wifi_station_status status = wifi_station_get_connect_status();
			printf("      Status : %s\n", wifi_station_status(status));
			if (status != WIFI_STATION_GOT_IP) {
				printf("SSID : %s\n", network_params_get_ssid());
				console_error("Not connected!");
				if (oled_connected) {
					oled_set_cursor(&oled_info, 2, 0);
					oled_puts(&oled_info, wifi_station_status(status));
					oled_set_cursor(&oled_info, 5, 0);
					oled_printf(&oled_info, "SSID : %s\n", network_params_get_ssid());
					oled_status(&oled_info, "<Not connected!>");
				}
				for (;;)
					;
			}
		}
	} else {
		console_error("wifi_get_ip_info");
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "E: wifi_get_ip_info"));
	}

	if (fota_params_init()) {
		OLED_CONNECTED(oled_connected, oled_status(&oled_info, "FOTA mode"));
		console_newline();
		fota(fota_params_get_server());
		for(;;);
	}

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting UDP ..."));

	udp_begin(incoming_port);

	udp_sendto((const uint8_t *)"osc", (const uint16_t) 3, ip_config.ip.addr | ~ip_config.netmask.addr, outgoing_port);

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Setting Node parameters ..."));

	printf(" Type  : %s\n", deviceparms.GetLedTypeString());
	printf(" Count : %d", (int) deviceparms.GetLedCount());

	if (oled_connected) {
		oled_set_cursor(&oled_info, 1, 0);
		if (opmode == WIFI_STA) {
			oled_puts(&oled_info, "Station");
		} else {
			oled_printf(&oled_info, "AP (%s)\n", *ap_password == '\0' ? "Open" : "WPA_WPA2_PSK");
		}
		oled_set_cursor(&oled_info, 2, 0);
		oled_printf(&oled_info, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		oled_set_cursor(&oled_info, 3, 0);
		oled_printf(&oled_info, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		oled_set_cursor(&oled_info, 4, 0);
		oled_printf(&oled_info, "I: %4d O: %4d", (int) incoming_port, (int) outgoing_port);
		oled_set_cursor(&oled_info, 5, 0);
		oled_printf(&oled_info, "Led type: %s", deviceparms.GetLedTypeString());
		oled_set_cursor(&oled_info, 6, 0);
		oled_printf(&oled_info, "Led count: %d", (int) deviceparms.GetLedCount());
	}

	console_set_top_row(16);

	hardware_watchdog_init();

	OSCWS28xx oscws28xx(outgoing_port, deviceparms.GetLedCount(), deviceparms.GetLedType(), deviceparms.GetLedTypeString());

	console_status(CONSOLE_GREEN, "Starting ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting ..."));

	oscws28xx.Start();

	console_status(CONSOLE_GREEN, "Controller started");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Controller started"));

	for (;;) {
		hardware_watchdog_feed();
		oscws28xx.Run();
		led_blink();
	}
}

}
