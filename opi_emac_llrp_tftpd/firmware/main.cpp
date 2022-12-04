/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "storenetwork.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"

#include "rdmnetllrponly.h"

#include "mdns.h"
#include "mdnsservices.h"
#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

// System Time
#include "ntpclient.h"

// Handler
#include "factorydefaults.h"

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

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;

	fw.Print("RDMNet LLRP device only");

	hw.SetLed(hardware::LedStatus::ON);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=RDMNet LLRP Only");
#endif
	mDns.Print();
#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
	httpDaemon.Start();
#endif

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	RDMNetLLRPOnly device;

	device.Init();
	device.Print();
	device.Start();

	RemoteConfig remoteConfig(remoteconfig::Node::RDMNET_LLRP_ONLY, remoteconfig::Output::CONFIG, 0);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (configStore.Flash())
		;

	display.SetTitle("LLRP Only - TFTP");
	display.Set(2, displayudf::Labels::HOSTNAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(5, displayudf::Labels::VERSION);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show();
	display.Write(6, "mDNS enabled");
	display.TextStatus("Device running", Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);

	lb.SetMode(ledblink::Mode::NORMAL);

	for (;;) {
		nw.Run();
		mDns.Run();
		device.Run();
		remoteConfig.Run();
		ntpClient.Run();
		configStore.Flash();
		lb.Run();
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
	}
}

}
