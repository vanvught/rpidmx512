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

#include "oscclient.h"
#include "oscclientconst.h"
#include "oscclientparams.h"

#include "buttonsset.h"
#include "buttonsgpio.h"

#if defined(ORANGE_PI)
 #include "spiflashinstall.h"
 #include "spiflashstore.h"
 #include "storeoscclient.h"
 #include "remoteconfig.h"
 #include "remoteconfigparams.h"
 #include "storeremoteconfig.h"
#endif

#include "firmwareversion.h"

#include "software_version.h"

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
	StoreOscClient storeOscClient;

	OscClientParams params((OscClientParamsStore *)&storeOscClient);
#else
	OscClientParams params;
#endif

	OscClient client;

	if (params.Load()) {
		params.Dump();
		params.Set(&client);
	}

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

	console_status(CONSOLE_YELLOW, OscClientConst::MSG_CLIENT_PARAMS);
	display.TextStatus(OscClientConst::MSG_CLIENT_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_PARMAMS);

	client.Print();

	ButtonsSet *pButtonsSet = 0;
	// In the future,we can support more Buttons Classes
	pButtonsSet = new ButtonsGpio(&client);
	assert(pButtonsSet != 0);

	if (!pButtonsSet->Start()) {
		// TODO Show error message
	}

#if defined (ORANGE_PI)
	RemoteConfig remoteConfig(REMOTE_CONFIG_OSC_CLIENT, REMOTE_CONFIG_MODE_OSC, 0);

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

	display.Write(1, "Eth OSC Client");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "S: " IPSTR, IP2STR(client.GetServerIP()));
	display.Printf(5, "Out: %d", client.GetPortOutgoing());
	display.Printf(6, "In: %d", client.GetPortIncoming());

	console_status(CONSOLE_YELLOW, OscClientConst::MSG_CLIENT_START);
	display.TextStatus(OscClientConst::MSG_CLIENT_START, DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_START);

	client.Start();

	hw.SetLed(HARDWARE_LED_FLASH);

	console_status(CONSOLE_GREEN, OscClientConst::MSG_CLIENT_STARTED);
	display.TextStatus(OscClientConst::MSG_CLIENT_STARTED, DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_STARTED);

#if defined (ORANGE_PI)
	while (spiFlashStore.Flash())
		;
#endif

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		client.Run();
		pButtonsSet->Run();
#if defined (ORANGE_PI)
		remoteConfig.Run();
		spiFlashStore.Flash();
#endif
		lb.Run();
		display.Run();
	}
}
}
