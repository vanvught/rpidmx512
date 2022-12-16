/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "network.h"
#include "ledblink.h"

#include "display.h"
#include "displayudfparams.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "httpd/httpd.h"

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

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "configstore.h"
#include "storedisplayudf.h"
#include "storee131.h"
#include "storemonitor.h"
#include "storenetwork.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

int main(int argc, char **argv) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	hw.Print();
	fw.Print("sACN E1.31 Real-time DMX Monitor {4 Universes}");

	ConfigStore configStore;

	StoreNetwork storeNetwork;

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	nw.Print();

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	StoreE131 storeE131;
	E131Params e131Params(&storeE131);

	E131Bridge bridge;

	if (e131Params.Load()) {
		e131Params.Dump();
		e131Params.Set();
	}

	StoreMonitor storeMonitor;
	DMXMonitorParams monitorParams(&storeMonitor);

	DMXMonitor monitor;

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

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}

	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_RDMNET_LLRP, LLRP_PORT, mdns::Protocol::UDP, "node=RDMNet LLRP Only");
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=sACN E1.31");
	mDns.Print();

	bridge.Print();

	HttpDaemon httpDaemon;
	httpDaemon.Start();

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::MONITOR, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (configStore.Flash())
		;

	llrpOnlyDevice.Start();
	bridge.Start();

	for (;;) {
		bridge.Run();
		mDns.Run();
		httpDaemon.Run();
		remoteConfig.Run();
		llrpOnlyDevice.Run();
		configStore.Flash();
	}

	return 0;
}
