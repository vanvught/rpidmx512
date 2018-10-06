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

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	uint8_t nHwTextLength;

	ArtNetParams artnetparams;

	if (!hw.IsButtonPressed()) {
		if (artnetparams.Load()) {
			artnetparams.Dump();
		}
	}

	Display display(0,8);
	const bool oled_connected = display.isDetected();

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

	console_set_top_row(3);

	console_status(CONSOLE_YELLOW, "Network init ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Network init ..."));

	nw.Init();
	nw.Print();

	ArtNetNode node;

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Setting Node parameters ..."));

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, 0 + artnetparams.GetUniverse());
	node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, 1 + artnetparams.GetUniverse());
#if defined (ORANGE_PI_ONE)
	node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, 2 + artnetparams.GetUniverse());
 #ifndef DO_NOT_USE_UART0
	node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, 3 + artnetparams.GetUniverse());
 #endif
#endif

	DMXSendMulti dmx;
	DMXParams dmxparams;

	if (!hw.IsButtonPressed()) {
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}
	}

	dmx.Print();

	IpProg ipprog;
	ArtNetRdmController discovery;

	artnetparams.Set(&node);

	node.SetIpProgHandler(&ipprog);
	node.SetOutput(&dmx);
	node.SetDirectUpdate(false);
	node.Print();

	if (oled_connected) {
		uint8_t nAddress;
		node.GetUniverseSwitch((uint8_t) 0, nAddress);

		(void) display.Printf(1, "Eth Art-Net 3 %s", artnetparams.IsRdm() ? "RDM" : "DMX");
		(void) display.Printf(2, "%s", hw.GetBoardName(nHwTextLength));
		(void) display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));
		(void) display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
		(void) display.Printf(5, "SN: %s", node.GetShortName());
		(void) display.Printf(6, "N: %d SubN: %d U: %d", node.GetNetSwitch() ,node.GetSubnetSwitch(), nAddress);
		(void) display.Printf(7, "Active ports: %d", node.GetActiveOutputPorts());
	}

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
	}
}

}
