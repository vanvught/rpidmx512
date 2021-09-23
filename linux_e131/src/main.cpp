/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardware.h"
#include "network.h"
#include "ledblink.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "e131bridge.h"
#include "e131params.h"
#include "storee131.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"
#include "storemonitor.h"

#include "spiflashstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "display.h"

using namespace e131;

int main(int argc, char **argv) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display(DisplayType::UNKNOWN); 	// Display is not supported. We just need an object
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	fw.Print();

	puts("sACN E1.31 Real-time DMX Monitor {4 Universes}");

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	SpiFlashStore spiFlashStore;

	E131Params e131Params(new StoreE131);
	E131Bridge bridge;

	if (e131Params.Load()) {
		e131Params.Dump();
		e131Params.Set(&bridge);
	}

	DMXMonitor monitor;
	DMXMonitorParams monitorParams(new StoreMonitor);

	if (monitorParams.Load()) {
		monitorParams.Dump();
		monitorParams.Set(&monitor);
	}

	bridge.SetOutput(&monitor);

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		bool bIsSet;
		const auto nUniverse = e131Params.GetUniverse(i, bIsSet);

		if (bIsSet) {
			bridge.SetUniverse(i,lightset::PortDir::OUTPUT, nUniverse);
		}
	}

	nw.Print();
	bridge.Print();

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=sACN E1.31");
	mDns.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::MONITOR, bridge.GetActiveOutputPorts());
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	bridge.Start();

	for (;;) {
		bridge.Run();
		mDns.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
	}

	return 0;
}
