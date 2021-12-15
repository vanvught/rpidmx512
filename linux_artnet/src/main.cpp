/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "hardware.h"
#include "network.h"
#include "storenetwork.h"
#include "ledblink.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "storeartnet.h"
#include "artnetmsgconst.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"
#include "storemonitor.h"

#include "artnetrdmresponder.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"
#include "storerdmsubdevices.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "display.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

using namespace artnet;

int main(int argc, char **argv) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display(DisplayType::UNKNOWN); 	// Display is not supported. We just need a pointer to object
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	fw.Print();

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreNetwork storeNetwork;

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	StoreArtNet storeArtNet;

	ArtNetParams artnetParams(StoreArtNet::Get());

	ArtNet4Node node;

	if (artnetParams.Load()) {
		artnetParams.Dump();
		artnetParams.Set(&node);
	}

	if(artnetParams.IsRdm()) {
		printf("Art-Net %d Node - Real-time DMX Monitor / RDM Responder {1 Universe}\n", node.GetVersion());
	} else {
		printf("Art-Net %d Node - Real-time DMX Monitor {4 Universes}\n", node.GetVersion());
	}

	DMXMonitorParams monitorParams(new StoreMonitor);

	DMXMonitor monitor;
	monitor.SetDmxMonitorStore(StoreMonitor::Get());

	if (monitorParams.Load()) {
		monitorParams.Dump();
		monitorParams.Set(&monitor);
	}

	node.SetOutput(&monitor);
	node.SetArtNetStore(StoreArtNet::Get());

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality("Real-time DMX Monitor", &monitor)};
	ArtNetRdmResponder RdmResponder(pRDMPersonalities, 1);

	node.SetRdmUID(RdmResponder.GetUID());

	if(artnetParams.IsRdm()) {
		RDMDeviceParams rdmDeviceParams(new StoreRDMDevice);

		RdmResponder.SetRDMDeviceStore(StoreRDMDevice::Get());

		if (rdmDeviceParams.Load()) {
			rdmDeviceParams.Set(&RdmResponder);
			rdmDeviceParams.Dump();
		}

		RdmResponder.Init();

		bool isSet;
		node.SetUniverseSwitch(0, lightset::PortDir::OUTPUT, artnetParams.GetUniverse(0, isSet));

		RdmResponder.Full(0);

		node.SetRdmHandler(&RdmResponder, true);
	} else {
		for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
			bool bIsSet;
			const auto nAddress = artnetParams.GetUniverse(i, bIsSet);

			if (bIsSet) {
				node.SetUniverseSwitch(i, lightset::PortDir::OUTPUT, nAddress);
			}
		}
	}

	hw.Print();
	nw.Print();
	node.Print();

	if(artnetParams.IsRdm()) {
		RdmResponder.Print();
	}

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=Art-Net 4");
	mDns.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::MONITOR, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	node.Start();

	for (;;) {
		node.Run();
		mDns.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
	}

	return 0;
}
