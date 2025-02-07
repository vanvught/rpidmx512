/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "network.h"


#include "net/apps/mdns.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"
#include "e131sync.h"

#include "artnetcontroller.h"
#include "artnetoutput.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace hal {
void reboot_handler() {
	E131Bridge::Get()->Stop();
}
}  // namespace hal

int main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("sACN E1.31 -> Art-Net");

	E131Bridge bridge;

	E131Params e131params;
	e131params.Load();
	e131params.Set();

	bridge.SetDisableSynchronize(true);

	ArtNetController controller;
	ArtNetOutput artnetOutput;

	bridge.SetOutput(&artnetOutput);
	bridge.SetE131Sync(&artnetOutput);
	
	bool bIsSetIndividual = false;
	uint16_t nUniverse[e131params::MAX_PORTS];

	for (uint32_t nPortIndex = 0; nPortIndex < e131params::MAX_PORTS; nPortIndex++) {
		bool bIsSet;
		nUniverse[nPortIndex] = e131params.GetUniverse(nPortIndex, bIsSet);

		for (uint32_t j = 0; j < nPortIndex; j++) {
			if (nUniverse[nPortIndex] == nUniverse[j]) {
				bIsSet = false;
				break;
			}
		}

		if (bIsSet) {
			bridge.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, nUniverse[nPortIndex]);
			bIsSetIndividual = true;
		}
	}

	if (!bIsSetIndividual) {
		for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			bridge.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nPortIndex + 1));
		}
	}
	
	bridge.Print();
	artnetOutput.Print();
	controller.Print();

	display.SetTitle("sACN E1.31 Art-Net %d", bridge.GetActiveOutputPorts());
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(4, displayudf::Labels::UNIVERSE_PORT_B);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_C);
	display.Set(6, displayudf::Labels::UNIVERSE_PORT_D);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::ARTNET, bridge.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	RDMPersonality *pPersonalities[1] = { new RDMPersonality("RDMNet LLRP device only", static_cast<uint16_t>(0)) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	constexpr char aLabel[] = "sACN E1.31 to Art-Net";

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, (sizeof(aLabel) / sizeof(aLabel[0])) - 1);
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();

	display.TextStatus(E131MsgConst::START, CONSOLE_YELLOW);

	bridge.Start();
	controller.Start();

	display.TextStatus(E131MsgConst::STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		controller.Run();
		display.Run();
		hw.Run();
	}
}
