/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "artnetnode.h"
#include "artnetparams.h"

#include "dmxmonitor.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "identify.h"
#include "artnetrdmresponder.h"

#if defined (__linux__)
 #include "ipprog.h"
#endif

#include "software_version.h"

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	LedBlinkLinux lbt;
	uint8_t nTextLength;
	ArtNetParams artnetparams;
	ArtNetNode node;
	DMXMonitor monitor;
#if defined (__linux__)
	IpProg ipprog;
#endif

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name [max_dmx_channels]\n", argv[0]);
		return -1;
	}

	if (argc == 3) {
		uint16_t max_channels = atoi(argv[2]);
		if (max_channels > 512) {
			max_channels = 512;
		}
		monitor.SetMaxDmxChannels(max_channels);
	}

	if (artnetparams.Load()) {
		artnetparams.Dump();
		artnetparams.Set(&node);
	}

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);
	puts("Art-Net 3 Node - Real-time DMX Monitor");

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());
	node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse() + 1);
	node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse() + 2);
	node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse() + 3);

	node.SetOutput(&monitor);

	RDMPersonality personality("Real-time DMX Monitor", monitor.GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, &monitor);

	node.SetRdmHandler(&RdmResponder, true);

	Identify identify;

#if defined (__linux__)
	if (getuid() == 0) {
		node.SetIpProgHandler(&ipprog);
	}
#endif

	char *params_long_name = (char *)artnetparams.GetLongName();
	if (*params_long_name == 0) {
		char LongName[ARTNET_LONG_NAME_LENGTH];
		snprintf(LongName, ARTNET_LONG_NAME_LENGTH, "%s Open Source Art-Net 3 Node", hw.GetSysName(nTextLength));
		node.SetLongName(LongName);
	}

	nw.Print();
	node.Print();
	RdmResponder.GetRDMDeviceResponder()->Print();

	node.Start();

	for (;;) {
		(void) node.HandlePacket();
		identify.Run();
	}

	return 0;
}
