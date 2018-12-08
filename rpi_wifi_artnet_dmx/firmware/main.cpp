/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"
#include "networkesp8266.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "wifi.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"
#include "artnetparams.h"

#include "timecode.h"
#include "timesync.h"

#include "dmxparams.h"
#include "dmxsend.h"
#ifndef H3
 #include "dmxmonitor.h"
#endif
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"

#include "software_version.h"

static const char NETWORK_INIT[] = "Network init ...";
static const char NODE_PARMAS[] = "Setting Node parameters ...";
static const char RUN_RDM[] = "Running RDM Discovery ...";
static const char START_NODE[] = "Starting the Node ...";
static const char NODE_STARTED[] = "Node started";

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkESP8266 nw;
	LedBlinkBaremetal lb;

	ArtNetParams artnetparams;

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	const TOutputType tOutputType = artnetparams.GetOutputType();

	Display display(0,8);

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi Art-Net 3 Node ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetparams.IsRdm() && (tOutputType == OUTPUT_TYPE_DMX)) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
#if defined (HAVE_SPI)
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
#endif
#ifdef H3
	console_putc('\n');
#endif

	console_set_top_row(3);

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Init();

	ArtNetNode node;
	DMXSend dmx;
	WS28xxDmx spi;
#ifndef H3
	DMXMonitor monitor;
#endif
	TimeCode timecode;
	TimeSync timesync;
	ArtNetRdmController discovery;

	console_status(CONSOLE_YELLOW, NODE_PARMAS);
	display.TextStatus(NODE_PARMAS);

	artnetparams.Set(&node);

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
		DMXParams dmxparams;
		if (dmxparams.Load()) {
			dmxparams.Dump();
		}
		dmxparams.Set(&dmx);

		node.SetOutput(&dmx);
		node.SetDirectUpdate(false);

		if(artnetparams.IsRdm()) {
			if (artnetparams.IsRdmDiscovery()) {
				console_status(CONSOLE_YELLOW, RUN_RDM);
				display.TextStatus(RUN_RDM);
				discovery.Full();
			}
			node.SetRdmHandler(&discovery);
		}
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		WS28xxDmxParams deviceparms;
		if (deviceparms.Load()) {
			deviceparms.Dump();
		}
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
	}
#ifndef H3
	else if (tOutputType == OUTPUT_TYPE_MONITOR) {
		node.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif

	node.Print();

	if (tOutputType != OUTPUT_TYPE_MONITOR) {
		console_puts("\n");
	}

	if (tOutputType == OUTPUT_TYPE_DMX) {
		dmx.Print();
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		spi.Print();
	}

	if (display.isDetected()) {
		display.Write(1, "WiFi Art-Net 3 ");

		switch (tOutputType) {
		case OUTPUT_TYPE_SPI:
			display.PutString("Pixel");
			break;
		case OUTPUT_TYPE_MONITOR:
			display.PutString("Monitor");
			break;
		default:
			if (artnetparams.IsRdm()) {
				display.PutString("RDM");
			} else {
				display.PutString("DMX");
			}
			break;
		}

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		uint8_t nAddress;
		node.GetUniverseSwitch((uint8_t) 0, nAddress);

		(void) display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));
		(void) display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
		(void) display.Printf(5, "SN: %s", node.GetShortName());
		(void) display.Printf(6, "N: %d SubN: %d U: %d", node.GetNetSwitch(),node.GetSubnetSwitch(), nAddress);
		(void) display.Printf(7, "Active ports: %d", node.GetActiveOutputPorts());
	}

	console_status(CONSOLE_YELLOW, START_NODE);
	display.TextStatus(START_NODE);

	node.Start();

	console_status(CONSOLE_GREEN, NODE_STARTED);
	display.TextStatus(NODE_STARTED);

	hw.WatchdogFeed();

	for (;;) {
		hw.WatchdogFeed();
		(void) node.HandlePacket();
		if (tOutputType == OUTPUT_TYPE_MONITOR) {
			timesync.ShowSystemTime();
		}
		lb.Run();
	}
}

}
