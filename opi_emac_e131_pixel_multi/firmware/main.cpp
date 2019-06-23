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
#include "networkconst.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "e131const.h"
#include "e131bridge.h"
#include "e131params.h"

#include "lightset.h"

#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	Display display(DISPLAY_SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;

	SpiFlashStore spiFlashStore;

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");;
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Pixel controller {4x 4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	console_status(CONSOLE_YELLOW, E131Const::MSG_BRIDGE_PARAMS);
	display.TextStatus(E131Const::MSG_BRIDGE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	E131Bridge bridge;
	E131Params e131params((E131ParamsStore *)spiFlashStore.GetStoreE131());

	if (e131params.Load()) {
		e131params.Set(&bridge);
		e131params.Dump();
	}

	WS28xxDmxMulti ws28xxDmxMulti(WS28XXDMXMULTI_SRC_E131);
	WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) spiFlashStore.GetStoreWS28xxDmx());

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&ws28xxDmxMulti);
		ws28xxparms.Dump();
	}

	ws28xxDmxMulti.Start(0);

	const uint16_t nLedCount = ws28xxDmxMulti.GetLEDCount();
	const uint8_t nActivePorts = ws28xxDmxMulti.GetActivePorts();
	const uint8_t nUniverseStart = e131params.GetUniverse();

	bridge.SetDirectUpdate(true);
	bridge.SetOutput(&ws28xxDmxMulti);

	uint8_t nPortIndex = 0;

	for (uint32_t i = 0; i < nActivePorts; i++) {
		bridge.SetUniverse(nPortIndex, E131_OUTPUT_PORT, nPortIndex + nUniverseStart);

		if (ws28xxDmxMulti.GetLEDType() == WS28XXMULTI_SK6812W) {
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

	RemoteConfig remoteConfig(REMOTE_CONFIG_E131, REMOTE_CONFIG_MODE_PIXEL, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	for (unsigned i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "Eth sACN E1.31 Pixel");
	display.Write(2, "Orange Pi Zero");
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "U: %d", nUniverseStart);
	display.Printf(6, "AP: %d", nActivePorts);
	display.Printf(7, "%s:%d", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()), ws28xxparms.GetLedCount());

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
