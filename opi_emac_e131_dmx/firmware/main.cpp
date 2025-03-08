/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "dmxparams.h"
#include "dmxsend.h"

#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmdeviceparams.h"
# include "rdmnetdevice.h"
# include "rdmnetconst.h"
# include "rdmpersonality.h"
# include "rdm_e120.h"
#endif

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"

#include "firmwareversion.h"
#include "software_version.h"

namespace hal {
void reboot_handler() {
	Dmx::Get()->Blackout();
	E131Bridge::Get()->Stop();
}
}  // namespace hal

static constexpr uint32_t PORT_INDEX = 0;

int main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("sACN E1.31 DMX {1 Universe}");

	Dmx dmx;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	DmxSend dmxSend;
	dmxSend.Print();

	DmxNodeNode dmxNodeNode;
	dmxNodeNode.SetOutput(&dmxSend);
	
	const auto portDirection = (dmxNodeNode.GetPortDirection(PORT_INDEX) == dmxnode::PortDirection::OUTPUT ? dmx::PortDirection::OUTP : dmx::PortDirection::INP);
	dmx.SetPortDirection(PORT_INDEX, portDirection , false);

	const auto nActivePorts = dmxNodeNode.GetActiveInputPorts() + dmxNodeNode.GetActiveOutputPorts();

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_CONFIG, CONSOLE_YELLOW);
	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN E1.31 DMX %u", nActivePorts);

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, H3_BOARD_NAME " DMX");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();
#endif

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

	dmxNodeNode.Print();

	display.SetTitle("sACN E1.31 DMX %s", portDirection == dmx::PortDirection::INP ? "Input" : "Output");
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::HOSTNAME);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::NodeType::E131, remoteconfig::Output::DMX, nActivePorts);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	display.TextStatus(DmxNodeMsgConst::START, CONSOLE_YELLOW);

	dmxNodeNode.Start();

	display.TextStatus(DmxNodeMsgConst::STARTED, CONSOLE_GREEN);

	hal::watchdog_init();

	for (;;) {
		hal::watchdog_feed();
		nw.Run();
		dmxNodeNode.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		display.Run();
		hal::run();
	}
}
