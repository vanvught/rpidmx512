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

#include "hardwarebaremetal.h"
#include "networkh3emac.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "networkconst.h"

#include "oscserver.h"
#include "oscserverparms.h"

// DMX Out
#include "dmxparams.h"
#include "dmxsend.h"
// Pixel controller
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxgrouping.h"
#include "handler.h"
// PWM Led
#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "handlertlc59711.h"

#if defined(ORANGE_PI)
 #include "spiflashinstall.h"
 #include "spiflashstore.h"
 #include "storeoscserver.h"
 #include "remoteconfig.h"
 #include "remoteconfigparams.h"
 #include "storeremoteconfig.h"
#endif

#include "firmwareversion.h"

#include "software_version.h"

static const char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
static const char START_BRIDGE[] = "Starting the Bridge ...";
static const char BRIDGE_STARTED[] = "Bridge started";

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	Display display(DISPLAY_SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

#if defined (ORANGE_PI)
	SpiFlashInstall spiFlashInstall;

	SpiFlashStore spiFlashStore;
	StoreOscServer storeOscServer;

	OSCServerParams params((OSCServerParamsStore *)&storeOscServer);
#else
	OSCServerParams params;
#endif

	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	const TLightSetOutputType tOutputType = params.GetOutputType();

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

#if defined (ORANGE_PI)
	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
#else
	nw.Init();
#endif
	nw.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	DMXSend dmx;
	LightSet *pSpi;
	OscServerHandler *pHandler;

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		bool isLedTypeSet = false;

#if defined (ORANGE_PI)
		TLC59711DmxParams pwmledparms((TLC59711DmxParamsStore *) spiFlashStore.GetStoreTLC59711());
#else
		TLC59711DmxParams pwmledparms;
#endif

		if (pwmledparms.Load()) {
			if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
				TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
				assert(pTLC59711Dmx != 0);
				pwmledparms.Dump();
				pwmledparms.Set(pTLC59711Dmx);
				pSpi = pTLC59711Dmx;

				pHandler = new HandlerTLC59711(pTLC59711Dmx);
				assert(pHandler != 0);

				display.Printf(7, "%s:%d", pwmledparms.GetLedTypeString(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
			}
		}

		if (!isLedTypeSet) {
#if defined (ORANGE_PI)
			WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) spiFlashStore.GetStoreWS28xxDmx());
#else
			WS28xxDmxParams ws28xxparms;
#endif
			if (ws28xxparms.Load()) {
				ws28xxparms.Dump();
			}

			display.Printf(7, "%s:%d %c", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()), ws28xxparms.GetLedCount(), ws28xxparms.IsLedGrouping() ? 'G' : ' ');

			if (ws28xxparms.IsLedGrouping()) {
				WS28xxDmxGrouping *pWS28xxDmxGrouping = new WS28xxDmxGrouping;
				assert(pWS28xxDmxGrouping != 0);
				ws28xxparms.Set(pWS28xxDmxGrouping);
				pSpi = pWS28xxDmxGrouping;
				display.Printf(7, "%s:%d G", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()), ws28xxparms.GetLedCount());
				pHandler = new Handler(pWS28xxDmxGrouping);
				assert(pHandler != 0);
			} else  {
				WS28xxDmx *pWS28xxDmx = new WS28xxDmx;
				assert(pWS28xxDmx != 0);
				ws28xxparms.Set(pWS28xxDmx);
				pSpi = pWS28xxDmx;

				const uint16_t nLedCount = pWS28xxDmx->GetLEDCount();

				// For the time being, just 1 Universe
				if (pWS28xxDmx->GetLEDType() == SK6812W) {
					if (nLedCount > 128) {
						pWS28xxDmx->SetLEDCount(128);
					}
				} else {
					if (nLedCount > 170) {
						pWS28xxDmx->SetLEDCount(170);
					}
				}
				display.Printf(7, "%s:%d", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()), nLedCount);
				pHandler = new Handler(pWS28xxDmx);
				assert(pHandler != 0);
			}
		}

		server.SetOutput(pSpi);
		server.SetOscServerHandler(pHandler);
	} else {
#if defined (ORANGE_PI)
		DMXParams dmxparams((DMXParamsStore *)spiFlashStore.GetStoreDmxSend());
#else
		DMXParams dmxparams;
#endif
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}

		server.SetOutput(&dmx);
	}

	server.Print();

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else {
		dmx.Print();
	}

#if defined (ORANGE_PI)
	RemoteConfig remoteConfig(REMOTE_CONFIG_OSC, tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? REMOTE_CONFIG_MODE_PIXEL : (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR ? REMOTE_CONFIG_MODE_MONITOR : REMOTE_CONFIG_MODE_DMX), 1);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}
#endif

	for (unsigned i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "Eth OSC %s", tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? "Pixel" : "DMX");
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

#if defined (ORANGE_PI)
	while (spiFlashStore.Flash())
		;
#endif

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
#if defined (ORANGE_PI)
		remoteConfig.Run();
		spiFlashStore.Flash();
#endif
		lb.Run();
		display.Run();
	}
}
}
