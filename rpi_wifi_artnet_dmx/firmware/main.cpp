/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <ap_params.h>
#include <artnetparams.h>
#include <devices_params.h>
#include <dmx_params.h>
#include <fota.h>
#include <fota_params.h>
#include <network_params.h>
#include <spisend.h>
#include <stdio.h>
#include <stdint.h>

#include "software_version.h"

#include "bcm2835_gpio.h"

#include "hardware.h"
#include "led.h"
#include "monitor.h"
#include "wifi.h"
#include "udp.h"
#include "console.h"

#include "artnetnode.h"
#include "dmxsend.h"

#define ESP822_FIRMWARE_VERSION_LENGTH	64

extern "C" {

void notmain(void) {
	_output_type output_type;
	uint32_t period = (uint32_t) 0;
	uint8_t mac_address[6];
	struct ip_info ip_config;
	ArtNetParams artnetparams;

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_22);

	hardware_init();

	// Discard return value. On failing, we use defaults.
	(void) artnetparams.Load();

	output_type = artnetparams.GetOutputType();

	if (output_type == OUTPUT_TYPE_SPI) {
		devices_params_init();
	} else {
		output_type = OUTPUT_TYPE_DMX;
		dmx_params_init();
	}

	printf("%s Compiled on %s at %s\n", hardware_get_board_model(), __DATE__, __TIME__);
	printf("WiFi ArtNet 3 Node DMX Output / Pixel controller [V%s]", SOFTWARE_VERSION);

	console_set_top_row(3);

	(void) ap_params_init();
	const char *ap_password = ap_params_get_password();

	hardware_watchdog_init();

	console_status(CONSOLE_YELLOW, "Starting Wifi ...");

	wifi_init(ap_password);

	hardware_watchdog_stop();

	printf("ESP8266 information\n");
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

	if (fota_params_init()) {
		console_newline();
		fota(fota_params_get_server());
		for(;;);
	}

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	udp_begin(6454);

	ArtNetNode node;
	DMXSend dmx;
	SPISend spi;

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	if (output_type == OUTPUT_TYPE_DMX) {
		node.SetOutput(&dmx);
		node.SetDirectUpdate(false);

		dmx.SetBreakTime(dmx_params_get_break_time());
		dmx.SetMabTime(dmx_params_get_mab_time());

		const uint8_t refresh_rate = dmx_params_get_refresh_rate();

		if (refresh_rate != (uint8_t) 0) {
			period = (uint32_t) (1E6 / refresh_rate);
		}

		dmx.SetPeriodTime(period);

	} else if (output_type == OUTPUT_TYPE_SPI) {
		spi.SetLEDCount(devices_params_get_led_count());
		spi.SetLEDType(devices_params_get_led_type());

		node.SetOutput(&spi);
		node.SetDirectUpdate(true);

		const uint16_t led_count = spi.GetLEDCount();
		const uint8_t universe = artnetparams.GetUniverse();

		if (led_count > 170) {
			node.SetDirectUpdate(true);
			node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, universe + 1);
		}

		if (led_count > 340) {
			node.SetDirectUpdate(true);
			node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, universe + 2);
		}

		if (led_count > 510) {
			node.SetDirectUpdate(true);
			node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, universe + 3);
		}
	}

	node.SetSubnetSwitch(artnetparams.GetSubnet());
	node.SetNetSwitch(artnetparams.GetNet());

	printf("\nNode configuration\n");
	const uint8_t *firmware_version = node.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" Short name   : %s\n", node.GetShortName());
	printf(" Long name    : %s\n", node.GetLongName());
	printf(" Net          : %d\n", node.GetNetSwitch());
	printf(" Sub-Net      : %d\n", node.GetSubnetSwitch());
	printf(" Universe     : %d\n", node.GetUniverseSwitch(0));
	printf(" Active ports : %d\n\n", node.GetActiveOutputPorts());

	if (output_type == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1E6 / dmx.GetPeriodTime()));
	} else if (output_type == OUTPUT_TYPE_SPI) {
		printf("Led stripe parameters\n");
		printf(" Type         : %s\n", devices_params_get_led_type_string());
		printf(" Count        : %d\n", (int) spi.GetLEDCount());
	}

	if (output_type == OUTPUT_TYPE_DMX) {
		hardware_watchdog_init();
	}

	console_status(CONSOLE_YELLOW, "Starting the Node ...");

	node.Start();

	console_status(CONSOLE_GREEN, "Node started");

	for (;;) {
		if (output_type == OUTPUT_TYPE_DMX) {
			hardware_watchdog_feed();
		}
		node.HandlePacket();
		led_blink();
	}
}

}
