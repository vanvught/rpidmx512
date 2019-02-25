/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarelinux.h"
#include "networklinux.h"
#include "ledblinklinux.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "artnet4params.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "identify.h"
#include "artnetrdmresponder.h"

#if defined (__linux__)
 #include "ipprog.h"
#endif

#if defined (RASPPI)
 #include "spiflashstore.h"
#endif

#include "software_version.h"

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	LedBlinkLinux lb;

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	uint8_t nTextLength;
	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

#if defined (RASPPI)
	SpiFlashStore spiFlashStore;
	ArtNet4Params artnet4params((ArtNet4ParamsStore *)spiFlashStore.GetStoreArtNet4());
	ArtNetParams artnetparams((ArtNetParamsStore *)spiFlashStore.GetStoreArtNet());
#else
	ArtNet4Params artnet4params;
	ArtNetParams artnetparams;
#endif

	if (artnet4params.Load()) {
		artnet4params.Dump();
	}

	ArtNet4Node node(artnet4params.IsMapUniverse0());

	if (artnetparams.Load()) {
		artnetparams.Dump();
		artnetparams.Set(&node);
	}

	if(artnetparams.IsRdm()) {
		printf("Art-Net %d Node - Real-time DMX Monitor / RDM Responder {1 Universe}\n", node.GetVersion());
	} else {
		printf("Art-Net %d Node - Real-time DMX Monitor {4 Universes}\n", node.GetVersion());
	}

	if (fopen("direct.update", "r") != NULL) {
		node.SetDirectUpdate(true);
	} // No worries about closing this file pointer

	DMXMonitor monitor;
	DMXMonitorParams params;

	if (params.Load()) {
		params.Dump();
		params.Set(&monitor);
	}

	node.SetOutput(&monitor);

	RDMPersonality personality("Real-time DMX Monitor", monitor.GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, &monitor);

	if(artnetparams.IsRdm()) {
		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

		if (artnetparams.IsRdmDiscovery()) {
			RdmResponder.Full(0);
		}

		node.SetRdmHandler(&RdmResponder, true);
	} else {
		uint8_t nAddress;
		bool bIsSetIndividual = false;
		bool bIsSet;

		for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
			nAddress = artnetparams.GetUniverse(i, bIsSet);

			if (bIsSet) {
				node.SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, nAddress);
				bIsSetIndividual = true;
			}
		}

		if (!bIsSetIndividual) {
			node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, 0 + artnetparams.GetUniverse());
			node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, 1 + artnetparams.GetUniverse());
			node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, 2 + artnetparams.GetUniverse());
			node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, 3 + artnetparams.GetUniverse());
		}
	}

	Identify identify;

#if defined (__linux__)
	IpProg ipprog;

	if (getuid() == 0) {
		node.SetIpProgHandler(&ipprog);
	}
#endif

#if defined (RASPPI)
	node.SetArtNetStore((ArtNetStore *)spiFlashStore.GetStoreArtNet());

	spiFlashStore.Dump();
#endif

	nw.Print();
	node.Print();

	if(artnetparams.IsRdm()) {
		RdmResponder.GetRDMDeviceResponder()->Print();
	}

	node.Start();

	for (;;) {
		node.HandlePacket();
		identify.Run();
#if defined (RASPPI)
		spiFlashStore.Flash();
#endif
	}

	return 0;
}
