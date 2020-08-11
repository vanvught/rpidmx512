/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "networkh3emac.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"

#include "rdmnetllrponly.h"

#include "mdns.h"
#include "mdnsservices.h"

// Handlers
#include "displayudfnetworkhandler.h"
#include "factorydefaults.h"
#include "reboot.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	console_puts("RDMNet LLRP device only\n");

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new Reboot);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(new DisplayUdfNetworkHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	MDNS mDns;

	mDns.Start();
	mDns.AddServiceRecord(0, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(0, MDNS_SERVICE_TFTP, 69);
	mDns.Print();

	RDMNetLLRPOnly device;

	device.GetRDMNetDevice()->SetRDMFactoryDefaults(new FactoryDefaults);

	device.Init();
	device.Print();
	device.Start();

	RemoteConfig remoteConfig(REMOTE_CONFIG_RDMNET_LLRP_ONLY, REMOTE_CONFIG_MODE_CONFIG, 0);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.SetTitle("LLRP Only - TFTP");
	display.Set(2, DISPLAY_UDF_LABEL_HOSTNAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Show();

	display.Write(5, "mDNS enabled");

	display.TextStatus("Device running", Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);

	lb.SetMode(LEDBLINK_MODE_NORMAL);

	for (;;) {
		nw.Run();
		mDns.Run();
		device.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
	}
}

}
