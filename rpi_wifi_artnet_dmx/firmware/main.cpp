/**
 * @file main.c
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

#include <stdio.h>
#include <stdint.h>

#include "bcm2835_gpio.h"

#include "artnetnode.h"
#include "artnet_params.h"
#include "dmx_params.h"
#include "dmxsend.h"
#include "spisend.h"
#include "network_params.h"
#include "devices_params.h"

#include "hardware.h"
#include "led.h"
#include "monitor.h"
#include "wifi.h"
#include "udp.h"
#include "console.h"

extern "C" {

void notmain(void) {
	uint8_t output_type;
	uint32_t period = (uint32_t) 0;
	uint8_t mac_address[6];
	struct ip_info ip_config;

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_22);

	hardware_init();
	artnet_params_init();

	output_type = artnet_params_get_output_type();

	if (output_type == OUTPUT_TYPE_SPI) {
		devices_params_init();
	} else {
		output_type = OUTPUT_TYPE_DMX;
		dmx_params_init();
	}

	printf("%s Compiled on %s at %s\n", hardware_get_board_model(), __DATE__, __TIME__);
	printf("WiFi ArtNet 3 Node DMX Output");

	console_set_top_row(4);

	hardware_watchdog_init();
	(void)wifi_init();
	hardware_watchdog_stop();

	printf("sdk_version : %s\n", system_get_sdk_version());
	printf("cpu freq    : %d\n\n", system_get_cpu_freq());
	printf("WiFi mode   : %s\n\n", wifi_get_opmode() == WIFI_STA ? "Station" : "Access Point");

	if (wifi_get_macaddr(mac_address)) {
		printf("MAC address : "MACSTR "\n", MAC2STR(mac_address));
	} else {
		printf("Error : wifi_get_macaddr\n");
	}

	printf("hostname    : %s\n", wifi_station_get_hostname());

	if (wifi_get_ip_info(&ip_config)) {
		printf("ip-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
		printf("netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
		printf("gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));
	} else {
		printf("Error : wifi_get_ip_info\n");
	}

	if (network_params_init()) {
		if (network_params_is_use_dhcp()) {
			wifi_station(network_params_get_ssid(), network_params_get_password());
		} else {
			ip_config.ip.addr = network_params_get_ip_address();
			ip_config.netmask.addr = network_params_get_net_mask();
			ip_config.gw.addr = network_params_get_default_gateway();
			wifi_station_ip(network_params_get_ssid(), network_params_get_password(), &ip_config);
		}

		printf("\nWiFi mode   : %s\n\n", wifi_get_opmode() == WIFI_STA ? "Station" : "Access Point");

		if (wifi_get_ip_info(&ip_config)) {
			printf("ip-address  : " IPSTR "\n", IP2STR(ip_config.ip.addr));
			printf("netmask     : " IPSTR "\n", IP2STR(ip_config.netmask.addr));
			printf("gateway     : " IPSTR "\n", IP2STR(ip_config.gw.addr));
		} else {
			printf("Error : wifi_get_ip_info\n");
		}
	}

	udp_begin(6454);

	ArtNetNode node;
	DMXSend dmx;
	SPISend spi;

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnet_params_get_universe());

	if (output_type == OUTPUT_TYPE_DMX) {
		node.SetOutput(&dmx);
		node.SetDirectUpdate(false);

		dmx.SetBreakTime(dmx_params_get_break_time());
		dmx.SetMabTime(dmx_params_get_mab_time());

		uint8_t refresh_rate = dmx_params_get_refresh_rate();
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
		const uint8_t universe = artnet_params_get_universe();

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

	node.SetSubnetSwitch(artnet_params_get_subnet());
	node.SetNetSwitch(artnet_params_get_net());

	printf("\nNode configuration :\n");
	const uint8_t *firmware_version = node.GetSoftwareVersion();
	printf(" Firmware   : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" Short name : %s\n", node.GetShortName());
	printf(" Long name  : %s\n", node.GetLongName());
	printf(" Net        : %d\n", node.GetNetSwitch());
	printf(" Sub-Net    : %d\n", node.GetSubnetSwitch());
	printf(" Universe   : %d\n", node.GetUniverseSwitch(0));

	if (output_type == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters :\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1E6 / dmx.GetPeriodTime()));
	} else if (output_type == OUTPUT_TYPE_SPI) {
		printf("Led stripe parameters :\n");
		printf(" Type         : %s\n", devices_params_get_led_type_string());
		printf(" Count        : %d\n", (int) spi.GetLEDCount());
	}

	hardware_watchdog_init();

	node.Start();

	for (;;) {
		hardware_watchdog_feed();
		node.HandlePacket();
		led_blink();
	}
}

}
