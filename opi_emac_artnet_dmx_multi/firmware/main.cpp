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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "storeartnet.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

#include "artnetdiscovery.h"

// DMX/RDM Output
#include "dmxparams.h"
#include "dmxsend.h"
#include "storedmxsend.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"
#include "dmxconfigudp.h"
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
	Network nw;
	DisplayUdf display;
	DisplayUdfHandler displayUdfHandler;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color (CONSOLE_GREEN);
	console_puts("DMX");
	console_set_fg_color (CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("RDM");
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
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	StoreArtNet storeArtNet;
	ArtNetParams artnetParams(&storeArtNet);

	ArtNet4Node node;

	if (artnetParams.Load()) {
		artnetParams.Set(&node);
		artnetParams.Dump();
	}

	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(StoreArtNet::Get());

	uint8_t nAddress;
	bool bIsSet;

	nAddress = artnetParams.GetUniverse(0, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(0, artnetParams.GetDirection(0), nAddress);
	}
	nAddress = artnetParams.GetUniverse(1, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(1, artnetParams.GetDirection(1), nAddress);
	}
#if defined (ORANGE_PI_ONE)
	nAddress = artnetParams.GetUniverse(2, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(2, artnetParams.GetDirection(2), nAddress);
	}
#ifndef DO_NOT_USE_UART0
	nAddress = artnetParams.GetUniverse(3, bIsSet);
	if (bIsSet) {
		node.SetUniverseSwitch(3, artnetParams.GetDirection(3), nAddress);
	}
#endif
#endif

	StoreDmxSend storeDmxSend;
	DmxParams dmxparams(&storeDmxSend);

	Dmx dmx;

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	DmxSend dmxSend;

	dmxSend.Print();

	DmxConfigUdp *pDmxConfigUdp = nullptr;

	if (node.GetActiveOutputPorts() != 0) {
		node.SetOutput(&dmxSend);
		pDmxConfigUdp = new DmxConfigUdp;
		assert(pDmxConfigUdp != nullptr);
	}

	DmxInput dmxInput;

	if (node.GetActiveInputPorts() != 0) {
		node.SetArtNetDmx(&dmxInput);
	}

	StoreRDMDevice storeRdmDevice;

	if (node.GetActiveOutputPorts() != 0) {
		if(artnetParams.IsRdm()) {
			auto pDiscovery = new ArtNetRdmController;
			assert(pDiscovery != nullptr);

			RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

			if(rdmDeviceParams.Load()) {
				rdmDeviceParams.Set(pDiscovery);
				rdmDeviceParams.Dump();
			}

			pDiscovery->Init();
			pDiscovery->Print();

			display.TextStatus(ArtNetMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);

			for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
				uint8_t nAddress;
				if (node.GetUniverseSwitch(i, nAddress, lightset::PortDir::OUTPUT)) {
					pDiscovery->Full(i);
				}
			}

			node.SetRdmHandler(pDiscovery);
		}
	}

	node.Print();

	const auto nActivePorts = static_cast<uint32_t>(node.GetActiveInputPorts() + node.GetActiveOutputPorts());

	display.SetTitle("Art-Net 4 %u", nActivePorts);
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

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, artnetParams.IsRdm() ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, nActivePorts);

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
		if (pDmxConfigUdp != nullptr) {
			pDmxConfigUdp->Run();
		}
	}
}

}
