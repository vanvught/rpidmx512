/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal.h"
#include "network.h"

#include "net/apps/mdns.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"

#include "dmxserial.h"
#include "dmxserialparams.h"

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
	ArtNetNode::Get()->Stop();
}
}  // namespace hal

int main() {
	hal_init();
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("Art-Net 4 Node Serial [UART/SPI/I2C] {1 Universe}");

	DmxSerial dmxSerial;

	DmxSerialParams dmxSerialParams;
	dmxSerialParams.Load();
	dmxSerialParams.Set();

	DmxNodeNode dmxNodeNode;
	dmxNodeNode.SetOutput(&dmxSerial);
	dmxNodeNode.Print();

	dmxSerial.Init();
	dmxSerial.Print();

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

	display.SetTitle("Art-Net 4 %s", dmxSerial.GetSerialType());
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::HOSTNAME);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	uint32_t nFilesCount = dmxSerial.GetFilesCount();
	if (nFilesCount == 0) {
		display.Printf(7, "No files [SDCard?]");
	} else {
		display.Printf(7, "Channel%s: %d", nFilesCount == 1 ?  "" : "s", nFilesCount);
	}

	RemoteConfig remoteConfig(remoteconfig::NodeType::ARTNET, remoteconfig::Output::SERIAL, dmxNodeNode.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

#if defined (NODE_RDMNET_LLRP_ONLY)
	RDMPersonality *pPersonalities[1] = { new RDMPersonality("RDMNet LLRP device only", static_cast<uint16_t>(0)) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	constexpr char aLabel[] = "Art-Net 4 Serial [UART/SPI/I2C]";

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, (sizeof(aLabel) / sizeof(aLabel[0])) - 1);
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&llrpOnlyDevice);

	llrpOnlyDevice.Print();
#endif

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
