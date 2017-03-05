/**
 * @file main.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardware.h"
#include "console.h"

#include "lcd.h"

#include "display_7segment.h"

#include "midi_send.h"

#include "wifi.h"
#include "udp.h"
#include "ap_params.h"
#include "network_params.h"

#include "ltc_reader.h"
#include "ltc_reader_params.h"

#include "software_version.h"

static struct _ltc_reader_output output = { true, false, false, false };

extern "C" {

static void handle_bool(const bool b) {
	if (b) {
		console_save_color();
		console_set_fg_color(CONSOLE_GREEN);
		console_puts("Yes");
		console_restore_color();
	} else {
		console_puts("No");
	}
}

void notmain(void) {
	hardware_init();

	ltc_reader_params_init();

	output.console_output = ltc_reader_params_is_console_output();
	output.lcd_output = ltc_reader_params_is_lcd_output();
	output.segment_output = ltc_reader_params_is_7segment_output();
	output.midi_output = ltc_reader_params_is_midi_output();
	output.artnet_output = ltc_reader_params_is_artnet_output();

	if(output.midi_output) {
		midi_send_init();
	}

	if (output.lcd_output) {
		output.lcd_output = lcd_detect();
	}

	if (output.segment_output) {
		display_7segment_init();
	}

	if (output.artnet_output) {
		output.artnet_output = wifi_detect();
	}

	ltc_reader_init(&output);

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("SMPTE TimeCode LTC Reader / Converter");

	console_set_top_row(3);

	console_set_cursor(0, 8);
	console_puts("Console output   : "); handle_bool(output.console_output); console_putc('\n');
	console_puts("LCD output       : "); handle_bool(output.lcd_output); console_putc('\n');
	console_puts("7-Segment output : "); handle_bool(output.segment_output); console_putc('\n');
	console_puts("MIDI output      : "); handle_bool(output.midi_output); console_putc('\n');
	console_puts("ArtNet output    : "); handle_bool(output.artnet_output);console_puts(" (not implemented)\n");

	if (output.artnet_output) {
		uint8_t mac_address[6];
		struct ip_info ip_config;

		(void) ap_params_init();
		const char *ap_password = ap_params_get_password();

		console_status(CONSOLE_YELLOW, "Starting Wifi ...");

		wifi_init(ap_password);

		printf("\nESP8266 information\n");
		printf(" SDK      : %s\n", system_get_sdk_version());
		printf(" Firmware : %s\n\n", wifi_get_firmware_version());

		if (network_params_init()) {
			console_status(CONSOLE_YELLOW, "Changing to Station mode ...");
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
		}

		printf(" Hostname    : %s\n", wifi_station_get_hostname());

		if (wifi_get_ip_info(&ip_config)) {
			printf(" IP-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
			printf(" Netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
			printf(" Gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));
			if (opmode == WIFI_STA) {
				const _wifi_station_status status = wifi_station_get_connect_status();
				printf("      Status : %s\n", wifi_station_status(status));
				if (status != WIFI_STATION_GOT_IP){
					console_error("Not connected!");
					for(;;);
				}
			}
		} else {
			console_error("wifi_get_ip_info");
		}

		console_status(CONSOLE_YELLOW, "Starting UDP ...");
		udp_begin(6454);

		console_status(CONSOLE_GREEN, "Wifi is started");
	}

	for (;;) {
		ltc_reader();
	}
}

}
