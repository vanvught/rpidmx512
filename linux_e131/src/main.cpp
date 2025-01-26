/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cctype>
#include <signal.h>

#include "hardware.h"
#include "network.h"

#include "display.h"
#include "displayudfparams.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmnetconst.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "configstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"
#include "software_version_id.h"

static bool keepRunning = true;

void intHandler(int) {
    keepRunning = false;
}

namespace e131bridge {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace e131bridge

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, nullptr);
#ifndef NDEBUG
	if (argc > 2) {
		const int c = argv[2][0];
		if (isdigit(c)){
			e131bridge::configstore::DMXPORT_OFFSET = c - '0';
		}
	}
#endif
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw(argc, argv);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__, DEVICE_SOFTWARE_VERSION_ID);

	hw.Print();
	fw.Print();
	nw.Print();

	DisplayUdfParams displayUdfParams;

	E131Bridge bridge;

	E131Params e131Params;
	e131Params.Load();
	e131Params.Set();

	DMXMonitor monitor;

	DMXMonitorParams monitorParams;
	monitorParams.Load();

	bridge.SetOutput(&monitor);

	for (uint32_t nPortIndex = 0; nPortIndex < e131params::MAX_PORTS; nPortIndex++) {
		uint32_t nOffset = nPortIndex;
		if (nPortIndex >= e131bridge::configstore::DMXPORT_OFFSET) {
			nOffset = nPortIndex - e131bridge::configstore::DMXPORT_OFFSET;
		} else {
			continue;
		}

		printf(">> nPortIndex=%u, nOffset=%u\n", nPortIndex, nOffset);

		bool bIsSet;
		const auto nUniverse = e131Params.GetUniverse(nOffset, bIsSet);
		const auto portDirection = e131Params.GetDirection(nPortIndex);

		if (portDirection == lightset::PortDir::OUTPUT) {
			bridge.SetUniverse(nPortIndex,lightset::PortDir::OUTPUT, nUniverse);
		} else {
			bridge.SetUniverse(nPortIndex,lightset::PortDir::DISABLE	, nUniverse);
		}
	}

	const auto nActivePorts = bridge.GetActiveOutputPorts();

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN E1.31 DMX %d", nActivePorts);

	uint8_t nLength;
	const auto *aLabel = hw.GetBoardName(nLength);

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, nLength);
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();


	RDMDeviceParams rdmDeviceParams;

	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();

	bridge.Print();

#if defined (NODE_SHOWFILE)
	ShowFile showFile;

	ShowFileParams showFileParams;
	showFileParams.Load();
	showFileParams.Set();

	if (showFile.IsAutoStart()) {
		showFile.Play();
	}

	showFile.Print();
#endif

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::MONITOR, bridge.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	bridge.Start();

	while (keepRunning) {
		nw.Run();
		bridge.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		hw.Run();
	}

	return 0;
}
