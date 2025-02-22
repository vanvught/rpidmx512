/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "displayudf.h"
#include "displayudfparams.h"

#include "dmxnodenode.h"
#include "dmxnodemsgconst.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "artnetrdmresponder.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "dmxnodechain.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "sparkfundmx.h"
#include "sparkfundmxconst.h"

#if defined (NODE_SHOWFILE)
# include "showfile.h"
# include "showfileparams.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

namespace hal {
void reboot_handler() {
	ArtNetNode::Get()->Stop();
}
}  // namespace hal

int main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("Art-Net 4 Stepper L6470");

	display.TextStatus(SparkFunDmxConst::MSG_INIT, CONSOLE_YELLOW);

	DmxNodeChain dmxNodeChain;

	SparkFunDmx sparkFunDmx;
	dmxNodeChain.SetSparkfunDmx(&sparkFunDmx);

	sparkFunDmx.ReadConfigFiles();

	auto nMotorsConnected = sparkFunDmx.GetMotorsConnected();
	auto isLedTypeSet = false;

	TLC59711DmxParams pwmledparms;
	pwmledparms.Load();

	TLC59711Dmx tlc59711Dmx;

	if ((isLedTypeSet = pwmledparms.IsSetLedType())) {
		dmxNodeChain.SetTLC59711Dmx(&tlc59711Dmx);
	}

	char aDescription[64];
	if (isLedTypeSet) {
		snprintf(aDescription, sizeof(aDescription) - 1, "Sparkfun [%d] with %s [%d]", nMotorsConnected, pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	} else {
		snprintf(aDescription, sizeof(aDescription) - 1, "Sparkfun [%d]", nMotorsConnected);
	}

	DmxNodeNode dmxNodeNode;
	
	dmxNodeNode.SetLongName(aDescription);
	dmxNodeNode.SetOutput(&dmxNodeChain);

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality(aDescription, &dmxNodeChain)};

	ArtNetRdmResponder rdmResponder(pRDMPersonalities, 1);

	rdmResponder.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	rdmResponder.SetProductDetail(E120_PRODUCT_DETAIL_LED);

	RDMSensorsParams rdmSensorsParams;
	rdmSensorsParams.Load();
	rdmSensorsParams.Set();

#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
	RDMSubDevicesParams rdmSubDevicesParams;
	rdmSubDevicesParams.Load();
	rdmSubDevicesParams.Set();
#endif

	rdmResponder.Init();

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load();
	rdmDeviceParams.Set(&rdmResponder);

	rdmResponder.Print();

	dmxNodeNode.SetRdmResponder(&rdmResponder);
	dmxNodeNode.Print();

	dmxNodeChain.Print();

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

	display.SetTitle("Art-Net 4 L6470");
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::VERSION);
	display.Set(4, displayudf::Labels::HOSTNAME);
	display.Set(5, displayudf::Labels::DMX_START_ADDRESS);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	if (isLedTypeSet) {
		display.Printf(7, "%s:%d", pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	}

	RemoteConfig remoteConfig(remoteconfig::NodeType::ARTNET, remoteconfig::Output::STEPPER, dmxNodeNode.GetActiveOutputPorts());

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	display.TextStatus(DmxNodeMsgConst::START, CONSOLE_YELLOW);

	dmxNodeNode.Start();

	display.TextStatus(DmxNodeMsgConst::STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		dmxNodeNode.Run();
#if defined (NODE_SHOWFILE)
		showFile.Run();
#endif
		display.Run();
		hw.Run();
	}
}
