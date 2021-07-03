/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

// Addressable led
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "h3/ws28xxdmxstartstop.h"
#include "handler.h"
#include "storews28xxdmx.h"
// PWM Led
#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "handlertlc59711.h"
#include "storetlc59711.h"

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
	StoreWS28xxDmx storeWS28xxDmx;
	StoreTLC59711 storeTLC59711;

	OSCServerParams params(&storeOscServer);
	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	fw.Print();

	hw.SetLed(hardware::LedStatus::ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.Init(StoreNetwork::Get());
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Print();

	MDNS mDns;

	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_OSC, server.GetPortIncoming(), "type=server");
	mDns.Print();

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	LightSet *pSpi = nullptr;
	OscServerHandler *pHandler;

	bool isLedTypeSet = false;

	TLC59711DmxParams pwmledparms(&storeTLC59711);

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			auto *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != nullptr);
			pwmledparms.Dump();
			pwmledparms.Set(pTLC59711Dmx);
			pSpi = pTLC59711Dmx;

			pHandler = new HandlerTLC59711(pTLC59711Dmx);
			assert(pHandler != nullptr);

			display.Printf(7, "%s:%d", pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
		}
	}

	if (!isLedTypeSet) {
		assert(pSpi == nullptr);

		PixelDmxConfiguration pixelDmxConfiguration;

		WS28xxDmxParams ws28xxparms(new StoreWS28xxDmx);

		if (ws28xxparms.Load()) {
			ws28xxparms.Set(&pixelDmxConfiguration);
			ws28xxparms.Dump();
		}

		// For the time being, just 1 Universe
		if (pixelDmxConfiguration.GetType() == pixel::Type::SK6812W) {
			if (pixelDmxConfiguration.GetCount() > 128) {
				pixelDmxConfiguration.SetCount(128);
			}
		} else {
			if (pixelDmxConfiguration.GetCount() > 170) {
				pixelDmxConfiguration.SetCount(170);
			}
		}

		auto *pPixelDmx = new WS28xxDmx(pixelDmxConfiguration);
		assert(pPixelDmx != nullptr);

		pPixelDmx->SetPixelDmxHandler(new PixelDmxStartStop);
		pSpi = pPixelDmx;

		display.Printf(7, "%s:%d G%d", PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

		pHandler = new Handler(pPixelDmx);
		assert(pHandler != nullptr);
	}

	server.SetOutput(pSpi);
	server.SetOscServerHandler(pHandler);
	server.Print();

	pSpi->Print();

	RemoteConfig remoteConfig(remoteconfig::Node::OSC,  remoteconfig::Output::PIXEL, 1);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	for (uint8_t i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "OSC Pixel 1");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	display.TextStatus(OscServerMsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	server.Start();

	display.TextStatus(OscServerMsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	while (spiFlashStore.Flash())
		;

	lb.SetMode(ledblink::Mode::NORMAL);

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
