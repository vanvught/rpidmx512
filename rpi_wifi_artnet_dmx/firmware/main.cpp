/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardware.h"
#include "monitor.h"
#include "console.h"

#include "ledblinktask.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"
#include "artnetparams.h"

#include "dmxparams.h"
#include "dmxsender.h"
#include "dmxmonitor.h"

#include "spisend.h"
#include "deviceparams.h"

#include "timecode.h"
#include "timesync.h"

#include "display.h"

#include "wifi.h"
#include "network.h"

#include "software_version.h"

extern "C" {
extern void network_init(void);

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void notmain(void) {
	struct ip_info ip_config;
	ArtNetParams artnetparams;
	DMXParams dmxparams;
	DeviceParams deviceparms;
	Display display(0,8);
	bool oled_connected = false;
	LedBlinkTask ledblinktask;

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_22);

	oled_connected = display.isDetected();

	(void) artnetparams.Load();

	TOutputType tOutputType = artnetparams.GetOutputType();

	if (tOutputType == OUTPUT_TYPE_SPI) {
		(void) deviceparms.Load();
	} else if (tOutputType != OUTPUT_TYPE_MONITOR){
		tOutputType = OUTPUT_TYPE_DMX;
		(void) dmxparams.Load();
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);

	console_puts("WiFi Art-Net 3 Node ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(artnetparams.IsRdm() ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Monitor");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	network_init();

	ArtNetNode node;
	DMXSender dmx;
	SPISend spi;
	DMXMonitor monitor;
	TimeCode timecode;
	TimeSync timesync;
	ArtNetDiscovery discovery;

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Setting Node parameters ..."));

	artnetparams.Set(&node);

	node.SetLedBlink(&ledblinktask);

	if (artnetparams.IsUseTimeCode() || tOutputType == OUTPUT_TYPE_MONITOR) {
		timecode.Start();
		node.SetTimeCodeHandler(&timecode);
	}

	if (artnetparams.IsUseTimeSync() || tOutputType == OUTPUT_TYPE_MONITOR) {
		timesync.Start();
		node.SetTimeSyncHandler(&timesync);
	}

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	if (tOutputType == OUTPUT_TYPE_DMX) {
		dmxparams.Set(&dmx);

		node.SetOutput(&dmx);
		node.SetDirectUpdate(false);

		if(artnetparams.IsRdm()) {
			if (artnetparams.IsRdmDiscovery()) {
				discovery.Full();
			}
			node.SetRdmHandler(&discovery);
			node.SetLongName("Raspberry Pi Art-Net 3 Node RDM Controller");
		}
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		deviceparms.Set(&spi);

		node.SetOutput(&spi);
		node.SetDirectUpdate(true);

		const uint16_t nLedCount = spi.GetLEDCount();
		const uint8_t nUniverse = artnetparams.GetUniverse();

		if (spi.GetLEDType() == SK6812W) {
			if (nLedCount > 128) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
			}
			if (nLedCount > 256) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
			}
			if (nLedCount > 384) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
			}
		} else {
			if (nLedCount > 170) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
			}
			if (nLedCount > 340) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
			}
			if (nLedCount > 510) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
			}
		}
	} else if (tOutputType == OUTPUT_TYPE_MONITOR) {
		node.SetOutput(&monitor);
		console_set_top_row(20);
	}

	printf("\nNode configuration\n");
	const uint8_t *firmware_version = node.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" Short name   : %s\n", node.GetShortName());
	printf(" Long name    : %s\n", node.GetLongName());
	printf(" Net          : %d\n", node.GetNetSwitch());
	printf(" Sub-Net      : %d\n", node.GetSubnetSwitch());
	printf(" Universe     : %d\n", node.GetUniverseSwitch(0));
	printf(" Active ports : %d", node.GetActiveOutputPorts());

	if (tOutputType != OUTPUT_TYPE_MONITOR) {
		console_puts("\n\n");
	}

	if (tOutputType == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1000000 / dmx.GetPeriodTime()));
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		TWS28XXType tType = spi.GetLEDType();

		printf("Led stripe parameters\n");
		printf(" Type         : %s [%d]\n", DeviceParams::GetLedTypeString(tType), tType);
		printf(" Count        : %d\n", (int) spi.GetLEDCount());
	}

	if (oled_connected) {
		display.Write(1, "WiFi Art-Net 3 ");

		switch (tOutputType) {
		case OUTPUT_TYPE_DMX:
			if (artnetparams.IsRdm()) {
				display.PutString("RDM");
			} else {
				display.PutString("DMX");
			}
			break;
		case OUTPUT_TYPE_SPI:
			display.PutString("Pixel");
			break;
		case OUTPUT_TYPE_MONITOR:
			display.PutString("Monitor");
			break;
		default:
			break;
		}

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		(void) (void) display.Printf(3, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		(void) (void) display.Printf(4, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		(void) (void) display.Printf(5, "SN: %s", node.GetShortName());
		(void) (void) display.Printf(6, "N: %d SubN: %d U: %d", node.GetNetSwitch(),node.GetSubnetSwitch(), node.GetUniverseSwitch(0));
		(void) (void) display.Printf(7, "Active ports: %d", node.GetActiveOutputPorts());
	}

	console_status(CONSOLE_YELLOW, "Starting the Node ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Starting the Node ..."));

	node.Start();

	console_status(CONSOLE_GREEN, "Node started");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Node started"));

	hardware_watchdog_init();

	for (;;) {
		hardware_watchdog_feed();

		(void) node.HandlePacket();

		if (tOutputType == OUTPUT_TYPE_MONITOR) {
			timesync.ShowSystemTime();
		}

		ledblinktask.Run();
	}
}

}
