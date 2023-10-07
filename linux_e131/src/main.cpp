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
#include "displayudfparams.h"

#include "mdns.h"


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
	Display display;
	ConfigStore configStore;
	StoreNetwork storeNetwork;
	Network nw(argc, argv);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	hw.Print();
	fw.Print();
	nw.Print();

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	uint32_t nPortIndexOffset = 0;

	StoreE131 storeE131;
	E131Params e131Params(&storeE131);

	E131Bridge bridge;

	if (e131Params.Load()) {
		e131Params.Dump();
#ifndef NDEBUG
		if (argc > 2) {
			const int c = argv[2][0];
			if (isdigit(c)){
				nPortIndexOffset = c - '0';
			}
#endif
		}
		e131Params.Set(nPortIndexOffset);
	}

	StoreMonitor storeMonitor;
	DMXMonitorParams monitorParams(&storeMonitor);

	DMXMonitor monitor;

	if (monitorParams.Load()) {
		monitorParams.Dump();
		monitorParams.Set(&monitor);
	}

	bridge.SetOutput(&monitor);

	for (uint32_t nPortIndex = 0; nPortIndex < e131params::MAX_PORTS; nPortIndex++) {
		uint32_t nOffset = nPortIndex;
		if (nPortIndex >= nPortIndexOffset) {
			nOffset = nPortIndex - nPortIndexOffset;
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

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}

	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);

	MDNS mDns;
	mDns.AddServiceRecord(nullptr, mdns::Services::CONFIG, "node=sACN E1.31");
	mDns.AddServiceRecord(nullptr, mdns::Services::RDMNET_LLRP, "node=RDMNet LLRP Only");
	mDns.AddServiceRecord(nullptr, mdns::Services::HTTP);
	mDns.Print();

	bridge.Print();

	HttpDaemon httpDaemon;

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::MONITOR, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (configStore.Flash())
		;

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
