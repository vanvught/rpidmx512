/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "hardware.h"
#include "networklinux.h"
#include "ledblink.h"

#include "artnet4node.h"
#include "artnet4params.h"
#include "storeartnet.h"
#include "storeartnet4.h"
#include "artnetmsgconst.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"
#include "storemonitor.h"

#include "identify.h"
#include "artnetrdmresponder.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"

#if defined (__linux__)
# include "ipprog.h"
#endif

#include "spiflashstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

int main(int argc, char **argv) {
	Hardware hw;
	NetworkLinux nw;
	LedBlink lb;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	fw.Print();

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	SpiFlashStore spiFlashStore;

	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnet4Params(StoreArtNet4::Get());

	ArtNet4Node node;

	if (artnet4Params.Load()) {
		artnet4Params.Dump();
		artnet4Params.Set(&node);
	}

	if(artnet4Params.IsRdm()) {
		printf("Art-Net %d Node - Real-time DMX Monitor / RDM Responder {1 Universe}\n", node.GetVersion());
	} else {
		printf("Art-Net %d Node - Real-time DMX Monitor {4 Universes}\n", node.GetVersion());
	}

	if (fopen("direct.update", "r") != NULL) {
		node.SetDirectUpdate(true);
	} // No worries about closing this file pointer

	DMXMonitor monitor;
	DMXMonitorParams monitorParams(new StoreMonitor);

	if (monitorParams.Load()) {
		monitorParams.Dump();
		monitorParams.Set(&monitor);
	}

	node.SetOutput(&monitor);
#if defined (__linux__)
	if (getuid() == 0) {
		node.SetIpProgHandler(new IpProg);
	}
#endif
	node.SetArtNetStore(StoreArtNet::Get());

	RDMPersonality personality("Real-time DMX Monitor", monitor.GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, &monitor);

	if(artnet4Params.IsRdm()) {
		RDMDeviceParams rdmDeviceParams;
		if (rdmDeviceParams.Load()) {
			rdmDeviceParams.Set(&RdmResponder);
			rdmDeviceParams.Dump();
		}

		RdmResponder.Init();

		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnet4Params.GetUniverse());

		if (artnet4Params.IsRdmDiscovery()) {
			RdmResponder.Full(0);
		}

		node.SetRdmHandler(&RdmResponder, true);
	} else {
		uint8_t nAddress;
		bool bIsSetIndividual = false;
		bool bIsSet;

		for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
			nAddress = artnet4Params.GetUniverse(i, bIsSet);

			if (bIsSet) {
				node.SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, nAddress);
				bIsSetIndividual = true;
			}
		}

		if (!bIsSetIndividual) {
			for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
				node.SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, i + artnet4Params.GetUniverse());
			}
		}
	}

	Identify identify;

	nw.Print();
	node.Print();

	if(artnet4Params.IsRdm()) {
		RdmResponder.Print();
	}

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
		identify.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
	}

	return 0;
}
