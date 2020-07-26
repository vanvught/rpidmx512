/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "display.h"

#include "networkconst.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "oscserver.h"
#include "oscserverparms.h"
#include "oscservermsgconst.h"

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

#include "displayhandler.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(DisplayType::SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreOscServer storeOscServer;
	StoreDmxSend storeDmxSend;

	OSCServerParams params(&storeOscServer);
	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.Init(StoreNetwork::Get());
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Print();

	MDNS mDns;

	mDns.Start();
	mDns.AddServiceRecord(0, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(0, MDNS_SERVICE_OSC, server.GetPortIncoming(), "type=server");
	mDns.Print();

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	DMXSend dmx;
	DMXParams dmxparams(&storeDmxSend);

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

	display.Printf(1, "OSC DMX 1");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	display.TextStatus(OscServerMsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	server.Start();

	hw.SetLed(HARDWARE_LED_FLASH);

	display.TextStatus(OscServerMsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

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
