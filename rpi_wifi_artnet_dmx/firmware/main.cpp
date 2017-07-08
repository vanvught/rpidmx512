/**
 * @file main.cpp
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

#include <dmxsend.h.txt>
#include <stdio.h>
#include <stdint.h>

#include "bcm2835_gpio.h"

#include "hardware.h"
#include "led.h"
#include "monitor.h"
#include "console.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "dmxparams.h"
#include "dmxsender.h"

#include "dmxmonitor.h"

#include "spisend.h"
#include "deviceparams.h"

#include "timecode.h"
#include "timesync.h"
#include "artnetdiscovery.h"

#include "oled.h"

#include "wifi.h"
#include "wifi_udp.h"

#include "software_version.h"

extern "C" {

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void notmain(void) {
	_output_type output_type = OUTPUT_TYPE_DMX;
	uint32_t period = (uint32_t) 0;
	struct ip_info ip_config;
	ArtNetParams artnetparams;
	DMXParams dmxparams;
	DeviceParams deviceparms;
	oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };
	bool oled_connected = false;

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_22);

	oled_connected = oled_start(&oled_info);

	(void) artnetparams.Load();

	output_type = artnetparams.GetOutputType();

	if (output_type == OUTPUT_TYPE_SPI) {
		(void) deviceparms.Load();
	} else if (output_type != OUTPUT_TYPE_MONITOR){
		output_type = OUTPUT_TYPE_DMX;
		(void) dmxparams.Load();
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);

	console_puts("WiFi ArtNet 3 Node ");
	console_set_fg_color(output_type == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(artnetparams.IsRdm() ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(output_type == OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Monitor");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(output_type == OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" {4 Universes}");

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting UDP ..."));

	wifi_udp_begin(6454);

	console_status(CONSOLE_YELLOW, "UDP started");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "UDP started"));

	ArtNetNode node;
	DMXSender dmx;
	SPISend spi;
	DMXMonitor monitor;
	TimeCode timecode;
	TimeSync timesync;
	ArtNetDiscovery discovery;

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Setting Node parameters ..."));

	if (artnetparams.IsUseTimeCode() || output_type == OUTPUT_TYPE_MONITOR) {
		timecode.Start();
		node.SetTimeCodeHandler(&timecode);
	}

	if (artnetparams.IsUseTimeSync() || output_type == OUTPUT_TYPE_MONITOR) {
		timesync.Start();
		node.SetTimeSyncHandler(&timesync);
	}

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	if (output_type == OUTPUT_TYPE_DMX) {
		node.SetOutput(&dmx);
		node.SetDirectUpdate(false);

		dmx.SetBreakTime(dmxparams.GetBreakTime());
		dmx.SetMabTime(dmxparams.GetMabTime());

		const uint8_t refresh_rate = dmxparams.GetRefreshRate();

		if (refresh_rate != (uint8_t) 0) {
			period = (uint32_t) (1000000 / refresh_rate);
		}

		dmx.SetPeriodTime(period);

		if(artnetparams.IsRdm()) {
			if (artnetparams.IsRdmDiscovery()) {
				discovery.Full();
			}
			node.SetRdmHandler(&discovery);
			node.SetLongName("Raspberry Pi Art-Net 3 Node RDM Controller");
		}
	} else if (output_type == OUTPUT_TYPE_SPI) {
		spi.SetLEDCount(deviceparms.GetLedCount());
		spi.SetLEDType(deviceparms.GetLedType());

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
	} else if (output_type == OUTPUT_TYPE_MONITOR) {
		node.SetOutput(&monitor);
		console_set_top_row(20);
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
	printf(" Active ports : %d", node.GetActiveOutputPorts());

	if (output_type != OUTPUT_TYPE_MONITOR) {
		console_puts("\n\n");
	}

	if (output_type == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1E6 / dmx.GetPeriodTime()));
	} else if (output_type == OUTPUT_TYPE_SPI) {
		printf("Led stripe parameters\n");
		printf(" Type         : %s\n", deviceparms.GetLedTypeString());
		printf(" Count        : %d\n", (int) spi.GetLEDCount());
	}

	if (oled_connected) {
		oled_set_cursor(&oled_info, 0, 0);
		oled_puts(&oled_info, "WiFi ArtNet 3 ");

		switch (output_type) {
		case OUTPUT_TYPE_DMX:
			if (artnetparams.IsRdm()) {
				oled_puts(&oled_info, "RDM");
			} else {
				oled_puts(&oled_info, "DMX Out");
			}
			break;
		case OUTPUT_TYPE_SPI:
			oled_puts(&oled_info, "Pixel");
			break;
		case OUTPUT_TYPE_MONITOR:
			oled_puts(&oled_info, "Monitor");
			break;
		default:
			break;
		}

		oled_set_cursor(&oled_info, 1, 0);
		if (wifi_get_opmode() == WIFI_STA) {
			(void) oled_printf(&oled_info, "S: %s", wifi_get_ssid());
		} else {
			(void) oled_printf(&oled_info, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		oled_set_cursor(&oled_info, 2, 0);
		(void) oled_printf(&oled_info, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		oled_set_cursor(&oled_info, 3, 0);
		(void) oled_printf(&oled_info, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		oled_set_cursor(&oled_info, 4, 0);
		(void) oled_printf(&oled_info, "SN: %s", node.GetShortName());
		oled_set_cursor(&oled_info, 5, 0);
		(void) oled_printf(&oled_info, "N: %d SubN: %d U: %d", node.GetNetSwitch(),node.GetSubnetSwitch(), node.GetUniverseSwitch(0));
		oled_set_cursor(&oled_info, 6, 0);
		(void) oled_printf(&oled_info, "Active ports: %d", node.GetActiveOutputPorts());
	}

	console_status(CONSOLE_YELLOW, "Starting the Node ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting the Node ..."));

	node.Start();

	console_status(CONSOLE_GREEN, "Node started");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Node started"));

	hardware_watchdog_init();

	for (;;) {
		hardware_watchdog_feed();

		(void) node.HandlePacket();

		if (output_type == OUTPUT_TYPE_MONITOR) {
			timesync.ShowSystemTime();
		}

		led_blink();
	}
}

}
