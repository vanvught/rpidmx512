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

// DMX/RDM Output
#include "dmxparams.h"
#include "dmxsend.h"
#include "storedmxsend.h"
#include "artnetdiscovery.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"
// DMX Input
#include "dmxinput.h"

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
	LedBlink lb;
	NetworkH3emac nw;
	DisplayUdf display;
	DisplayUdfHandler displayUdfHandler;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreDmxSend storeDmxSend;
	StoreRDMDevice storeRdmDevice;

	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnetparams(&storeArtNet4);

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color (CONSOLE_GREEN);
	if (artnetparams.GetDirection() == ARTNET_INPUT_PORT) {
		console_puts("DMX Input");
	} else {
		console_puts("DMX Output");
		console_set_fg_color (CONSOLE_WHITE);
		console_puts(" / ");
		console_set_fg_color((artnetparams.IsRdm()) ? CONSOLE_GREEN : CONSOLE_WHITE);
		console_puts("RDM");
	}
	console_set_fg_color (CONSOLE_WHITE);
	console_puts(" {1 Universe}\n");

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&displayUdfHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNet4Node node;
	ArtNetRdmController discovery;

	artnetparams.Set(&node);

	node.SetIpProgHandler(new IpProg);
	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(StoreArtNet::Get());

	DMXSend *pDmxOutput;
	DmxInput *pDmxInput;

	if (artnetparams.GetDirection() == ARTNET_INPUT_PORT) {
		pDmxInput = new DmxInput;
		assert(pDmxInput != 0);

		node.SetUniverseSwitch(0, ARTNET_INPUT_PORT, artnetparams.GetUniverse());
		node.SetArtNetDmx(pDmxInput);
	} else {
		pDmxOutput = new DMXSend;
		assert(pDmxOutput != 0);

		DMXParams dmxparams(&storeDmxSend);

		if (dmxparams.Load()) {
			dmxparams.Set(pDmxOutput);
			dmxparams.Dump();
		}

		pDmxOutput->Print();

		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());
		node.SetDirectUpdate(false);
		node.SetOutput(pDmxOutput);

		if (artnetparams.IsRdm()) {
			RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

			if(rdmDeviceParams.Load()) {
				rdmDeviceParams.Set(&discovery);
				rdmDeviceParams.Dump();
			}

			discovery.Init();
			discovery.Print();

			if (artnetparams.IsRdmDiscovery()) {
				display.TextStatus(ArtNetMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);
				discovery.Full();
			}

			node.SetRdmHandler(&discovery);
		}
	}

	node.Print();

	display.SetTitle("Art-Net 4 %s", artnetparams.GetDirection() == ARTNET_INPUT_PORT ? "DMX Input" : (artnetparams.IsRdm() ? "RDM" : "DMX Output"));
	display.Set(2, DISPLAY_UDF_LABEL_NODE_NAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_HOSTNAME);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	const uint8_t nActivePorts = (artnetparams.GetDirection() == ARTNET_INPUT_PORT ? node.GetActiveInputPorts() : node.GetActiveOutputPorts());

	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, artnetparams.IsRdm() ? REMOTE_CONFIG_MODE_RDM : REMOTE_CONFIG_MODE_DMX, nActivePorts);

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
		node.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
