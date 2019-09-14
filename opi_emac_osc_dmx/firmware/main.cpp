/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "networkh3emac.h"
#include "ledblink.h"

#include "console.h"
#include "display.h"

#include "networkconst.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "oscserver.h"
#include "oscserverparms.h"

// DMX Out
#include "dmxparams.h"
#include "dmxsend.h"
#include "storedmxsend.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storeoscserver.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"

#include "software_version.h"

static const char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
static const char START_BRIDGE[] = "Starting the Bridge ...";
static const char BRIDGE_STARTED[] = "Bridge started";

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(DISPLAY_SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;
	StoreOscServer storeOscServer;
	StoreDmxSend storeDmxSend;

	OSCServerParams params((OSCServerParamsStore *)&storeOscServer);
	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	MDNS mDns;

	mDns.Start();
	mDns.AddServiceRecord(0, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(0, MDNS_SERVICE_OSC, server.GetPortIncoming(), "type=server");
	mDns.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	DMXSend dmx;
	DMXParams dmxparams((DMXParamsStore *)&storeDmxSend);

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	server.SetOutput(&dmx);
	server.Print();

	dmx.Print();

	RemoteConfig remoteConfig(REMOTE_CONFIG_OSC, REMOTE_CONFIG_MODE_DMX, 1);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	for (unsigned i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "Eth OSC DMX");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_START);

	server.Start();

	hw.SetLed(HARDWARE_LED_FLASH);

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_STARTED);

	while (spiFlashStore.Flash())
		;

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		mDns.Run();
		lb.Run();
		display.Run();
	}
}
}
