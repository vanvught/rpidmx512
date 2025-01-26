/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <signal.h>

#include "hardware.h"
#include "network.h"

#include "display.h"
#include "displayudfparams.h"

#include "net/apps/mdns.h"

#include "pp.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmnetconst.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

static bool keepRunning = true;

void intHandler(int) {
    keepRunning = false;
}

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, nullptr);
	Hardware hw;
	Display display;
	ConfigStore configStore;
	Network nw(argc, argv);
//	MDNS mDns;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	hw.Print();
	fw.Print();
	nw.Print();

	DisplayUdfParams displayUdfParams;

	PixelPusher pp;

	const uint32_t nActivePorts = (argc == 3 ? atoi(argv[2]) : 2);

	pp.SetCount(256, nActivePorts, true);

	DMXMonitor monitor;

	DMXMonitorParams monitorParams;
	monitorParams.Load();

	pp.SetOutput(&monitor);

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "PixelPusher");

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

	mdns_service_record_add(nullptr, mdns::Services::RDMNET_LLRP, "node=RDMNet LLRP Only");

	pp.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::PP, remoteconfig::Output::MONITOR, nActivePorts);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	pp.Start();

	while (keepRunning) {
		nw.Run();
		pp.Run();
		hw.Run();
	}

	return 0;
}
