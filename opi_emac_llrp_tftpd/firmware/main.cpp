/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"


#include "displayudf.h"
#include "displayudfparams.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmnetllrponly.h"
#endif

#include "mdns.h"

#include "ntpclient.h"

#include "factorydefaults.h"

#include "firmwareversion.h"
#include "software_version.h"

void Hardware::RebootHandler() {
	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);

		while (ConfigStore::Get()->Flash())
			;

		Network::Get()->Shutdown();

		printf("Rebooting ...\n");

		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...",
		Display7SegmentMessage::INFO_REBOOTING);
	}
}

void main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	MDNS mDns;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("RDMNet LLRP device only");
	nw.Print();

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

#if defined (NODE_RDMNET_LLRP_ONLY)
	RDMNetLLRPOnly device;
	device.Init();
	device.Print();
#endif

	RemoteConfig remoteConfig(remoteconfig::Node::RDMNET_LLRP_ONLY, remoteconfig::Output::CONFIG, 0);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();

	display.SetTitle("LLRP Only - TFTP");
	display.Set(2, displayudf::Labels::HOSTNAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(5, displayudf::Labels::VERSION);

	DisplayUdfParams displayUdfParams;
	displayUdfParams.Load();
	displayUdfParams.Set(&display);

	display.Show();

	display.Write(6, "mDNS enabled");
	display.TextStatus("Device running", Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);

	hw.SetMode(hardware::ledblink::Mode::NORMAL);

	for (;;) {
		nw.Run();
		mDns.Run();
#if defined (NODE_RDMNET_LLRP_ONLY)
		device.Run();
#endif
		remoteConfig.Run();
		ntpClient.Run();
		configStore.Flash();
		display.Run();
		hw.Run();
	}
}
