/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "networkh3emac.h"
#include "networkconst.h"
#include "ledblink.h"

#include "console.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "e131bridge.h"
#include "e131const.h"
#include "e131params.h"
#include "storee131.h"

#include "artnetcontroller.h"
#include "artnetoutput.h"

#include "reboot.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	Reboot reboot;
	hw.SetRebootHandler(&reboot);

	fw.Print();

	console_puts("sACN E1.31 -> Art-Net\n");

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.SetNetworkStore((NetworkStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	console_status(CONSOLE_YELLOW, E131Const::MSG_BRIDGE_PARAMS);
	display.TextStatus(E131Const::MSG_BRIDGE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	StoreE131 storeE131;
	E131Params e131params((E131ParamsStore*) &storeE131);

	E131Bridge bridge;

	if (e131params.Load()) {
		e131params.Dump();
		e131params.Set(&bridge);
	}

	bridge.SetDirectUpdate(true);

	ArtNetController controller;
	ArtNetOutput artnetOutput;

	bridge.SetOutput(&artnetOutput);

	uint16_t nUniverse;
	bool bIsSetIndividual = false;
	bool bIsSet;

	nUniverse = e131params.GetUniverse(0, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(0, E131_OUTPUT_PORT, nUniverse);
		bIsSetIndividual = true;
	}

	nUniverse = e131params.GetUniverse(1, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(1, E131_OUTPUT_PORT, nUniverse);
		bIsSetIndividual = true;
	}

	nUniverse = e131params.GetUniverse(2, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(2, E131_OUTPUT_PORT, nUniverse);
		bIsSetIndividual = true;
	}

	nUniverse = e131params.GetUniverse(3, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(3, E131_OUTPUT_PORT, nUniverse);
		bIsSetIndividual = true;
	}

	if (!bIsSetIndividual) {
		for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		nUniverse = e131params.GetUniverse();
			bridge.SetUniverse(i, E131_OUTPUT_PORT, i + nUniverse);
		}
	}

	bridge.Print();
	controller.Print();

	display.SetTitle("sACN E1.31 Art-Net %d", bridge.GetActiveOutputPorts());
	display.Set(2, DISPLAY_UDF_LABEL_IP);
	display.Set(3, DISPLAY_UDF_LABEL_UNIVERSE_PORT_A);
	display.Set(4, DISPLAY_UDF_LABEL_UNIVERSE_PORT_B);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE_PORT_C);
	display.Set(6, DISPLAY_UDF_LABEL_UNIVERSE_PORT_D);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	RemoteConfig remoteConfig(REMOTE_CONFIG_E131, REMOTE_CONFIG_MODE_ARTNET, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;

	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	console_status(CONSOLE_YELLOW, E131Const::MSG_BRIDGE_START);
	display.TextStatus(E131Const::MSG_BRIDGE_START, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_START);

	bridge.Start();
	controller.Start();

	console_status(CONSOLE_GREEN, E131Const::MSG_BRIDGE_STARTED);
	display.TextStatus(E131Const::MSG_BRIDGE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		//
		bridge.Run();
		controller.Run();
		//
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
