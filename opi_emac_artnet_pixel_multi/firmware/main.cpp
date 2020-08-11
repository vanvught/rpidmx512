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
#include <cassert>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "networkconst.h"

#include "artnet4node.h"
#include "artnet4params.h"
#include "storeartnet.h"
#include "storeartnet4.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

#include "ipprog.h"

#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"
#include "ws28xx.h"
#include "storews28xxdmx.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayudfhandler.h"
#include "displayhandler.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	DisplayUdfHandler displayUdfHandler;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreWS28xxDmx storeWS28xxDmx;

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Pixel controller {4x/8x 4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.Init(StoreNetwork::Get());
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&displayUdfHandler);
	nw.Print();

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	WS28xxDmxMulti ws28xxDmxMulti(WS28XXDMXMULTI_SRC_ARTNET);
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&ws28xxDmxMulti);
		ws28xxparms.Dump();
	}

	ws28xxDmxMulti.Initialize();

	const uint8_t nActivePorts = ws28xxDmxMulti.GetActivePorts();

	ArtNet4Node node(nActivePorts);

	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnetparams(&storeArtNet4);

	if (artnetparams.Load()) {
		artnetparams.Set(&node);
		artnetparams.Dump();
	}

	node.SetIpProgHandler(new IpProg);
	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(StoreArtNet::Get());
	node.SetDirectUpdate(true);
	node.SetOutput(&ws28xxDmxMulti);

	const uint16_t nLedCount = ws28xxDmxMulti.GetLEDCount();
	const uint8_t nUniverseStart = artnetparams.GetUniverse();

	uint8_t nPortIndex = 0;
	uint8_t nPage = 1;

	for (uint32_t i = 0; i < nActivePorts; i++) {

		node.SetUniverseSwitch(nPortIndex, ARTNET_OUTPUT_PORT,  nUniverseStart);

		if (ws28xxDmxMulti.GetLEDType() == SK6812W) {
			if (nLedCount > 128) {
				node.SetUniverseSwitch(nPortIndex + 1, ARTNET_OUTPUT_PORT, nUniverseStart + 1);
			}

			if (nLedCount > 256) {
				node.SetUniverseSwitch(nPortIndex + 2, ARTNET_OUTPUT_PORT, nUniverseStart + 2);
			}

			if (nLedCount > 384) {
				node.SetUniverseSwitch(nPortIndex + 3, ARTNET_OUTPUT_PORT, nUniverseStart + 3);
			}
		} else {
			if (nLedCount > 170) {
				node.SetUniverseSwitch(nPortIndex + 1, ARTNET_OUTPUT_PORT, nUniverseStart + 1);
			}

			if (nLedCount > 340) {
				node.SetUniverseSwitch(nPortIndex + 2, ARTNET_OUTPUT_PORT, nUniverseStart + 2);
			}

			if (nLedCount > 510) {
				node.SetUniverseSwitch(nPortIndex + 3, ARTNET_OUTPUT_PORT, nUniverseStart + 3);
			}
		}

		if (nPage < ArtNet::MAX_PAGES) {
			uint8_t nSubnetSwitch = node.GetSubnetSwitch(nPage - 1);
			nSubnetSwitch = (nSubnetSwitch + 1) & 0x0F;
			node.SetSubnetSwitch(nSubnetSwitch, nPage);

			if (nSubnetSwitch == 0) {
				uint8_t nNetSwitch = node.GetNetSwitch(nPage);
				nNetSwitch = (nNetSwitch + 1) & 0x0F;
				node.SetNetSwitch(nNetSwitch, nPage);
			}
			nPage++;
		}

		nPortIndex += ArtNet::MAX_PORTS;
	}

	node.Print();
	ws28xxDmxMulti.Print();

	display.SetTitle("Eth Art-Net 4 Pixel %c", ws28xxDmxMulti.GetBoard() == WS28XXMULTI_BOARD_8X ? '8' : (ws28xxDmxMulti.GetBoard() == WS28XXMULTI_BOARD_4X ? '4' : ' '));
	display.Set(2, DISPLAY_UDF_LABEL_VERSION);
	display.Set(3, DISPLAY_UDF_LABEL_NODE_NAME);
	display.Set(4, DISPLAY_UDF_LABEL_HOSTNAME);
	display.Set(5, DISPLAY_UDF_LABEL_IP);
	display.Set(6, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Printf(7, "%d-%s:%d", ws28xxDmxMulti.GetActivePorts(), WS28xx::GetLedTypeString(ws28xxDmxMulti.GetLEDType()), ws28xxDmxMulti.GetLEDCount());

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, REMOTE_CONFIG_MODE_PIXEL, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();;
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
