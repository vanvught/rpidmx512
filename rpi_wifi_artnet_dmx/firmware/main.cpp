/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "hardware.h"
#include "networkesp8266.h"
#include "ledblink.h"

#include "console.h"
#include "display.h"

#include "wifi.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "timecode.h"
#include "timesync.h"

// DMX output / RDM
#include "dmxparams.h"
#include "dmxsend.h"
#include "rdmdeviceparams.h"
#include "artnetdiscovery.h"
#ifndef H3
 // Monitor Output
# include "dmxmonitor.h"
#endif
// Pixel Controller
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxgrouping.h"
#include "ws28xx.h"

#if defined(ORANGE_PI)
# include "spiflashinstall.h"
# include "spiflashstore.h"
# include "storeartnet.h"
# include "storerdmdevice.h"
# include "storedmxsend.h"
# include "storews28xxdmx.h"
#endif

#include "software_version.h"

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char NODE_PARMAS[] = "Setting Node parameters ...";
constexpr char RUN_RDM[] = "Running RDM Discovery ...";
constexpr char START_NODE[] = "Starting the Node ...";
constexpr char NODE_STARTED[] = "Node started";

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkESP8266 nw;
	LedBlink lb;
	Display display(DisplayType::SSD1306);

#if defined (ORANGE_PI)
	if (hw.GetBootDevice() == BOOT_DEVICE_MMC0) {
		SpiFlashInstall spiFlashInstall;
	}

	SpiFlashStore spiFlashStore;

	StoreDmxSend storeDmxSend;
	StoreWS28xxDmx storeWS28xxDmx;
	StoreRDMDevice storeRdmDevice;

	ArtNetParams artnetparams(StoreArtNet::Get());
#else
	ArtNetParams artnetparams;
#endif

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	const TLightSetOutputType tOutputType = artnetparams.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi Art-Net 3 Node ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetparams.IsRdm() && (tOutputType == LIGHTSET_OUTPUT_TYPE_DMX)) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif

	console_set_top_row(3);

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

#if defined (ORANGE_PI)
	nw.Init();
#else
	nw.Init();
#endif

	ArtNetNode node;

#ifndef H3
	DMXMonitor monitor;
#endif
	TimeCode timecode;
	TimeSync timesync;
	ArtNetRdmController discovery;

	console_status(CONSOLE_YELLOW, NODE_PARMAS);
	display.TextStatus(NODE_PARMAS);

	artnetparams.Set(&node);

	if (artnetparams.IsUseTimeCode() || tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		timecode.Start();
		node.SetTimeCodeHandler(&timecode);
	}

	if (artnetparams.IsUseTimeSync() || tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		timesync.Start();
		node.SetTimeSyncHandler(&timesync);
	}

	const uint8_t nUniverse = artnetparams.GetUniverse();

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, nUniverse);
	node.SetDirectUpdate(false);

	DMXSend dmx;
	LightSet *pSpi;

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
#if defined (ORANGE_PI)
		WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) StoreWS28xxDmx::Get());
#else
		WS28xxDmxParams ws28xxparms;
#endif
		if (ws28xxparms.Load()) {
			ws28xxparms.Dump();
		}

		display.Printf(7, "%s:%d %c", WS28xx::GetLedTypeString(ws28xxparms.GetLedType()), ws28xxparms.GetLedCount(), ws28xxparms.IsLedGrouping() ? 'G' : ' ');

		if (ws28xxparms.IsLedGrouping()) {
			WS28xxDmxGrouping *pWS28xxDmxGrouping = new WS28xxDmxGrouping;
			assert(pWS28xxDmxGrouping != 0);
			ws28xxparms.Set(pWS28xxDmxGrouping);
			pSpi = pWS28xxDmxGrouping;
		} else  {
			WS28xxDmx *pWS28xxDmx = new WS28xxDmx;
			assert(pWS28xxDmx != 0);
			ws28xxparms.Set(pWS28xxDmx);
			pSpi = pWS28xxDmx;

			const uint16_t nLedCount = pWS28xxDmx->GetLEDCount();

			if (pWS28xxDmx->GetLEDType() == SK6812W) {
				if (nLedCount > 128) {
					node.SetDirectUpdate(true);
					node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
				}
				if (nLedCount > 256) {
					node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
				}
				if (nLedCount > 384) {
					node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
				}
			} else {
				if (nLedCount > 170) {
					node.SetDirectUpdate(true);
					node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
				}
				if (nLedCount > 340) {
					node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
				}
				if (nLedCount > 510) {
					node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
				}
			}
		}
		node.SetOutput(pSpi);
	}
#ifndef H3
	else if (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		// There is support for HEX output only
		node.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {
#if defined (ORANGE_PI)
		DMXParams dmxparams(&storeDmxSend);
#else
		DMXParams dmxparams;
#endif
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}

		node.SetOutput(&dmx);

		if (artnetparams.IsRdm()) {
#if defined (ORANGE_PI)
			RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
#else
			RDMDeviceParams rdmDeviceParams;
#endif
			if(rdmDeviceParams.Load()) {
				rdmDeviceParams.Set(&discovery);
				rdmDeviceParams.Dump();
			}

			discovery.Init();
			discovery.Print();

			if (artnetparams.IsRdmDiscovery()) {
				console_status(CONSOLE_YELLOW, RUN_RDM);
				display.TextStatus(RUN_RDM);
				discovery.Full();
			}

			node.SetRdmHandler((ArtNetRdm *)&discovery);
		}
	}

	node.Print();

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else if (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		// Nothing
	} else {
		dmx.Print();
	}

	for (unsigned i = 0; i < 7; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "WiFi Art-Net 3 ");

	switch (tOutputType) {
	case LIGHTSET_OUTPUT_TYPE_SPI:
		display.PutString("Pixel");
		break;
	case LIGHTSET_OUTPUT_TYPE_MONITOR:
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
		display.Printf(2, "S: %s", wifi_get_ssid());
	} else {
		display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
	}

	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));

	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}

	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "U: %d", nUniverse);
	display.Printf(6, "Active ports: %d", node.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, START_NODE);
	display.TextStatus(START_NODE);

	node.Start();

	console_status(CONSOLE_GREEN, NODE_STARTED);
	display.TextStatus(NODE_STARTED);

	hw.WatchdogFeed();

	for (;;) {
		hw.WatchdogFeed();
		node.Run();
		if (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
			timesync.ShowSystemTime();
		}
		lb.Run();
#if defined (ORANGE_PI)
		spiFlashStore.Flash();
#endif
	}
}

}
