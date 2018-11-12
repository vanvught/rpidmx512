/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "networkh3emac.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "artnetnode.h"
#include "artnetdiscovery.h"
#include "artnetparams.h"

#include "ipprog.h"

#include "dmxparams.h"
#include "h3/dmxsendmulti.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;

	if (hw.GetBootDevice() == BOOT_DEVICE_MMC0) {
		SpiFlashInstall spiFlashInstall;
	}

	SpiFlashStore spiFlashStore;

	ArtNetParams artnetparams((ArtNetParamsStore *)spiFlashStore.GetStoreArtNet());

	if (!hw.IsButtonPressed()) {
		if (artnetparams.Load()) {
			artnetparams.Dump();
		}
	}

	Display display(0,8);
	const bool oled_connected = display.isDetected();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("Ethernet Art-Net 3 Node ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetparams.IsRdm()) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" {4 Universes}\n");

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, "Network init ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Network init ..."));

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	ArtNetNode node;

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Setting Node parameters ..."));

	artnetparams.Set(&node);

	uint8_t nAddress;
	bool bIsSetIndividual = false;
	bool bIsSet;

	nAddress = artnetparams.GetUniverse(0, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, nAddress);
		bIsSetIndividual = true;
	}

	nAddress = artnetparams.GetUniverse(1, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nAddress);
		bIsSetIndividual = true;
	}
#if defined (ORANGE_PI_ONE)
	nAddress = artnetparams.GetUniverse(2, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nAddress);
		bIsSetIndividual = true;
	}
#ifndef DO_NOT_USE_UART0
	nAddress = artnetparams.GetUniverse(3, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nAddress);
		bIsSetIndividual = true;
	}
#endif
#endif

	if (!bIsSetIndividual) { // Backwards compatibility
		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, 0 + artnetparams.GetUniverse());
		node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, 1 + artnetparams.GetUniverse());
#if defined (ORANGE_PI_ONE)
		node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, 2 + artnetparams.GetUniverse());
#ifndef DO_NOT_USE_UART0
		node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, 3 + artnetparams.GetUniverse());
#endif
#endif
	}

	DMXSendMulti dmx;
	DMXParams dmxparams((DMXParamsStore *)spiFlashStore.GetStoreDmxSend());

	if (!hw.IsButtonPressed()) {
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}
	}

	node.SetOutput(&dmx);
	node.SetDirectUpdate(false);

	IpProg ipprog;

	node.SetIpProgHandler(&ipprog);

	node.SetArtNetStore((ArtNetStore *)spiFlashStore.GetStoreArtNet());

	spiFlashStore.Dump();

	node.Print();
	dmx.Print();

	if (oled_connected) {
		uint8_t nAddress;
		node.GetUniverseSwitch(0, nAddress);

		(void) display.Printf(1, "Eth Art-Net 3 %s", artnetparams.IsRdm() ? "RDM" : "DMX");
		(void) display.Printf(2, "%s", hw.GetBoardName(nHwTextLength));
		(void) display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));
		if (nw.IsDhcpKnown()) {
			if (nw.IsDhcpUsed()) {
				display.PutString(" D");
			} else {
				display.PutString(" S");
			}
		}
		(void) display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
		(void) display.Printf(5, "SN: %s", node.GetShortName());
		(void) display.Printf(6, "N: %d SubN: %d U: %d", node.GetNetSwitch() ,node.GetSubnetSwitch(), nAddress);
		(void) display.Printf(7, "Active ports: %d", node.GetActiveOutputPorts());
	}

	ArtNetRdmController discovery;

	if(artnetparams.IsRdm()) {
		if (artnetparams.IsRdmDiscovery()) {
			console_status(CONSOLE_YELLOW, "Running RDM Discovery ...");
			DISPLAY_CONNECTED(oled_connected, display.TextStatus("Running RDM Discovery ..."));

			for (uint8_t i = 0; i < ARTNET_MAX_PORTS; i++) {
				uint8_t nAddress;
				if (node.GetUniverseSwitch(i, nAddress)) {
					discovery.Full(i);
				}
			}
		}
		node.SetRdmHandler(&discovery);
	}

	console_status(CONSOLE_YELLOW, "Starting the Node ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Starting the Node ..."));

	node.Start();

	console_status(CONSOLE_GREEN, "Node started");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Node started"));

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		(void) node.HandlePacket();
		lb.Run();
		(void) spiFlashStore.Flash();
	}
}

}
