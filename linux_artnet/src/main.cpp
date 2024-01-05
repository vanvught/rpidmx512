/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cstdlib>
#include <ctype.h>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"

#include "display.h"

#include "mdns.h"

#include "httpd/httpd.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "artnetmsgconst.h"
#include "artnetrdmresponder.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"

#include "factorydefaults.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "configstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace artnetnode {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace artnetnode

int main(int argc, char **argv) {
#ifndef NDEBUG
	if (argc > 2) {
		const int c = argv[2][0];
		if (isdigit(c)){
			artnetnode::configstore::DMXPORT_OFFSET = c - '0';
		}
	}
#endif
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw(argc, argv);
	MDNS mDns;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	hw.Print();
	fw.Print();
	nw.Print();

	ArtNetNode node;

	ArtNetParams artnetParams;
	artnetParams.Load();
	artnetParams.Set();

	printf("Art-Net %d Node - Real-time DMX Monitor {4 Universes}\n", node.GetVersion());

	DMXMonitor monitor;

	DMXMonitorParams monitorParams;
	monitorParams.Load();

	node.SetOutput(&monitor);

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality("Real-time DMX Monitor", &monitor)};

	ArtNetRdmResponder RdmResponder(pRDMPersonalities, 1);
	RdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;

	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&RdmResponder);

	RdmResponder.Print();

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		uint32_t nOffset = nPortIndex;
		if (nPortIndex >= artnetnode::configstore::DMXPORT_OFFSET) {
			nOffset = nPortIndex - artnetnode::configstore::DMXPORT_OFFSET;
		} else {
			continue;
		}

		printf(">> nPortIndex=%u, nOffset=%u\n", nPortIndex, nOffset);

		const auto nAddress = artnetParams.GetUniverse(nOffset);
		const auto portDirection = artnetParams.GetDirection(nOffset);

		if (portDirection == lightset::PortDir::OUTPUT) {
			node.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, nAddress);
			if (nPortIndex == 0) {
				node.SetRdm(static_cast<uint32_t>(0), true);
			}
		} else {
			node.SetUniverse(nPortIndex, lightset::PortDir::DISABLE, nAddress);
		}
	}

	const auto nActivePorts = node.GetActiveOutputPorts();

	node.SetRdmUID(RdmResponder.GetUID());
	node.SetRdmResponder(&RdmResponder);
	node.SetRdm(static_cast<uint32_t>(0), true);
	node.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::MONITOR, nActivePorts);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();
	node.Start();

	for (;;) {
		node.Run();
		mDns.Run();
		remoteConfig.Run();
		configStore.Flash();
	}

	return 0;
}
