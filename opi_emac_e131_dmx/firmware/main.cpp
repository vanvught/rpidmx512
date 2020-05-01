/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "networkconst.h"
#include "e131const.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131.h"

#include "reboot.h"

// DMX Output
#include "dmxparams.h"
#include "dmxsend.h"
#include "storedmxsend.h"
// DMX Input
#include "dmxinput.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storee131.h"
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
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreE131 storeE131;
	StoreDmxSend storeDmxSend;

	E131Params e131params(&storeE131);

	if (e131params.Load()) {
		e131params.Dump();
	}

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(CONSOLE_GREEN);
	if (e131params.GetDirection() == E131_INPUT_PORT) {
		console_puts("DMX Input");
	} else {
		console_puts("DMX Output");
	}
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" {1 Universe}\n");

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new Reboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init(spiFlashStore.GetStoreNetwork());
	nw.SetNetworkStore(spiFlashStore.GetStoreNetwork());
	nw.Print();

	console_status(CONSOLE_YELLOW, E131Const::MSG_BRIDGE_PARAMS);
	display.TextStatus(E131Const::MSG_BRIDGE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	E131Bridge bridge;

	e131params.Set(&bridge);

	DMXSend *pDmxOutput;
	DmxInput *pDmxInput;

	const uint16_t nUniverse = e131params.GetUniverse();

	if (e131params.GetDirection() == E131_INPUT_PORT) {
		pDmxInput = new DmxInput;
		assert(pDmxInput != 0);

		bridge.SetUniverse(0, E131_INPUT_PORT, nUniverse);
		bridge.SetE131Dmx(pDmxInput);
	} else {
		pDmxOutput = new DMXSend;
		assert(pDmxOutput != 0);

		DMXParams dmxparams(&storeDmxSend);

		if (dmxparams.Load()) {
			dmxparams.Set(pDmxOutput);
			dmxparams.Dump();
		}

		pDmxOutput->Print();

		bridge.SetUniverse(0, E131_OUTPUT_PORT, nUniverse);
		bridge.SetDirectUpdate(false);
		bridge.SetOutput(pDmxOutput);
	}

	bridge.Print();

	display.SetTitle("sACN E1.31 DMX %s", e131params.GetDirection() == E131_INPUT_PORT ? "Input" : "Output");
	display.Set(2, DISPLAY_UDF_LABEL_BOARDNAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_AP);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	const uint32_t nActivePorts = (e131params.GetDirection() == E131_INPUT_PORT ? bridge.GetActiveInputPorts() : bridge.GetActiveOutputPorts());

	RemoteConfig remoteConfig(REMOTE_CONFIG_E131, REMOTE_CONFIG_MODE_DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	console_status(CONSOLE_YELLOW, E131Const::MSG_BRIDGE_START);
	display.TextStatus(E131Const::MSG_BRIDGE_START, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_START);

	bridge.Start();

	console_status(CONSOLE_GREEN, E131Const::MSG_BRIDGE_STARTED);
	display.TextStatus(E131Const::MSG_BRIDGE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
