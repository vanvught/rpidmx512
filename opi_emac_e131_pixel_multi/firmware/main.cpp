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
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"

#include "reboot.h"

#include "lightset.h"

#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"
#include "ws28xx.h"
#include "storews28xxdmx.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storee131.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayudfnetworkhandler.h"
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
	StoreWS28xxDmx storeWS28xxDmx;

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");;
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Pixel controller {4x/8x 4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(new DisplayUdfNetworkHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	E131Bridge bridge;
	E131Params e131params(&storeE131);

	if (e131params.Load()) {
		e131params.Set(&bridge);
		e131params.Dump();
	}

	WS28xxDmxMulti ws28xxDmxMulti(WS28XXDMXMULTI_SRC_E131);
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&ws28xxDmxMulti);
		ws28xxparms.Dump();
	}

	ws28xxDmxMulti.Initialize();

	bridge.SetDirectUpdate(true);
	bridge.SetOutput(&ws28xxDmxMulti);

	const uint16_t nLedCount = ws28xxDmxMulti.GetLEDCount();
	const uint8_t nActivePorts = ws28xxDmxMulti.GetActivePorts();
	const uint8_t nUniverseStart = e131params.GetUniverse();

	uint8_t nPortIndex = 0;

	for (uint32_t i = 0; i < nActivePorts; i++) {
		bridge.SetUniverse(nPortIndex, E131_OUTPUT_PORT, nPortIndex + nUniverseStart);

		if (ws28xxDmxMulti.GetLEDType() == SK6812W) {
			if (nLedCount > 128) {
				bridge.SetUniverse(nPortIndex + 1, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 1);
			}

			if (nLedCount > 256) {
				bridge.SetUniverse(nPortIndex + 2, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 2);
			}

			if (nLedCount > 384) {
				bridge.SetUniverse(nPortIndex + 3, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 3);
			}
		} else {
			if (nLedCount > 170) {
				bridge.SetUniverse(nPortIndex + 1, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 1);
			}

			if (nLedCount > 340) {
				bridge.SetUniverse(nPortIndex + 2, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 2);
			}

			if (nLedCount > 510) {
				bridge.SetUniverse(nPortIndex + 3, E131_OUTPUT_PORT, nUniverseStart + nPortIndex + 3);
			}
		}

		nPortIndex += ws28xxDmxMulti.GetUniverses();
	}

	bridge.Print();
	ws28xxDmxMulti.Print();

	Reboot reboot;
	hw.SetRebootHandler(&reboot);

	display.SetTitle("Eth sACN Pixel %c", ws28xxDmxMulti.GetBoard() == WS28XXMULTI_BOARD_8X ? '8' : (ws28xxDmxMulti.GetBoard() == WS28XXMULTI_BOARD_4X ? '4' : ' '));
	display.Set(2, DISPLAY_UDF_LABEL_HOSTNAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_BOARDNAME);
	display.Printf(7, "%d-%s:%d", ws28xxDmxMulti.GetActivePorts(), WS28xx::GetLedTypeString(ws28xxDmxMulti.GetLEDType()), ws28xxDmxMulti.GetLEDCount());

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	RemoteConfig remoteConfig(REMOTE_CONFIG_E131, REMOTE_CONFIG_MODE_PIXEL, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(E131MsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	bridge.Start();

	display.TextStatus(E131MsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

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
