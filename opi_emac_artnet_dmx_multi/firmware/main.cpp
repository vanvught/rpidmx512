/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetdiscovery.h"

// DMX/RDM Output
#include "dmxparams.h"
#include "dmxsendmulti.h"
#include "storedmxsend.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"
// DMX Input
#include "dmxinput.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
//
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "artnet/displayudfhandler.h"
#include "displayhandler.h"

using namespace artnet;

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

	const auto portDir = artnetparams.GetDirection();

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color(CONSOLE_GREEN);

	if (portDir == PortDir::INPUT) {
		console_puts("DMX Input");
	} else {
		console_puts("DMX Output");
		console_set_fg_color(CONSOLE_WHITE);
		console_puts(" / ");
		console_set_fg_color((artnetparams.IsRdm()) ? CONSOLE_GREEN : CONSOLE_WHITE);
		console_puts("RDM");
	}

	console_set_fg_color(CONSOLE_WHITE);
#if defined(ORANGE_PI)
	console_puts(" {2 Universes}\n");
#else
	console_puts(" {4 Universes}\n");
#endif

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&displayUdfHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	ArtNet4Node node;

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	artnetparams.Set(&node);

	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(StoreArtNet::Get());

	uint8_t nAddress;
	bool bIsSetIndividual = false;
	bool bIsSet;
printf("%d\n", __LINE__);
	nAddress = artnetparams.GetUniverse(0, bIsSet);
printf("%d\n", __LINE__);
	if (bIsSet) {
		node.SetUniverseSwitch(0, portDir, nAddress);
		bIsSetIndividual = true;
	}
printf("%d\n", __LINE__);
	nAddress = artnetparams.GetUniverse(1, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(1, portDir, nAddress);
		bIsSetIndividual = true;
	}
#if defined (ORANGE_PI_ONE)
	nAddress = artnetparams.GetUniverse(2, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(2, portDir, nAddress);
		bIsSetIndividual = true;
	}
#ifndef DO_NOT_USE_UART0
	nAddress = artnetparams.GetUniverse(3, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(3, portDir, nAddress);
		bIsSetIndividual = true;
	}
#endif
#endif

	if (!bIsSetIndividual) { // Backwards compatibility
		node.SetUniverseSwitch(0, portDir, 0 + artnetparams.GetUniverse());
		node.SetUniverseSwitch(1, portDir, 1 + artnetparams.GetUniverse());
#if defined (ORANGE_PI_ONE)
		node.SetUniverseSwitch(2, portDir, 2 + artnetparams.GetUniverse());
#ifndef DO_NOT_USE_UART0
		node.SetUniverseSwitch(3, portDir, 3 + artnetparams.GetUniverse());
#endif
#endif
	}

	// DMX/RDM Output
	DMXSendMulti *pDmxOutput;
	ArtNetRdmController *pDiscovery;
	// DMX Input
	DmxInput *pDmxInput;

	if (portDir == PortDir::INPUT) {
		pDmxInput = new DmxInput;
		assert(pDmxInput != nullptr);

		node.SetArtNetDmx(pDmxInput);
	} else {
		pDmxOutput = new DMXSendMulti;
		assert(pDmxOutput != nullptr);
		DMXParams dmxParams(&storeDmxSend);

		if (dmxParams.Load()) {
			dmxParams.Dump();
			dmxParams.Set(pDmxOutput);
		}
		node.SetOutput(pDmxOutput);

		pDmxOutput->Print();

		pDiscovery = new ArtNetRdmController;
		assert(pDiscovery != nullptr);

		if(artnetparams.IsRdm()) {
			RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

			if(rdmDeviceParams.Load()) {
				rdmDeviceParams.Set(pDiscovery);
				rdmDeviceParams.Dump();
			}

			pDiscovery->Init();
			pDiscovery->Print();

			display.TextStatus(ArtNetMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);

			for (uint8_t i = 0; i < ArtNet::PORTS; i++) {
				uint8_t nAddress;
				if (node.GetUniverseSwitch(i, nAddress, PortDir::OUTPUT)) {
					pDiscovery->Full(i);
				}
			}

			node.SetRdmHandler(pDiscovery);
		}
	}

	node.Print();

	display.SetTitle("Art-Net 4 %s", artnetparams.GetDirection() == PortDir::INPUT ? "DMX Input" : (artnetparams.IsRdm() ? "RDM" : "DMX Output"));
	display.Set(2, displayudf::Labels::NODE_NAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(6, displayudf::Labels::UNIVERSE_PORT_B);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	const uint32_t nActivePorts = (artnetparams.GetDirection() == PortDir::INPUT ? node.GetActiveInputPorts() : node.GetActiveOutputPorts());

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, artnetparams.IsRdm() ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;

	if (SpiFlashStore::Get()->HaveFlashChip()) {
		RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

		if (remoteConfigParams.Load()) {
			remoteConfigParams.Set(&remoteConfig);
			remoteConfigParams.Dump();
		}

		while (spiFlashStore.Flash())
			;
	} else {
		remoteConfig.SetDisable(true);
		printf("Remote configuration is disabled\n");
	}

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
