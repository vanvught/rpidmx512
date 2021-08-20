/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "ledblink.h"
// Display
#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"
#include "displayhandler.h"
// NTP Client
#include "ntpclient.h"
// mDNS
#include "mdns.h"
#include "mdnsservices.h"
// DDP
#include "ddpdisplay.h"
#include "ddpdisplayparams.h"
#include "ddpdisplaypixelconfiguration.h"
#include "pixeltype.h"
#include "storeddpdisplay.h"
// DMX

// Pixel Test pattern
#include "pixeltestpattern.h"
#include "pixelreboot.h"
// RDMNet LLRP Only
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"
//
#include "spiflashinstall.h"
#include "spiflashstore.h"
//
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"
//
#include "firmwareversion.h"
#include "software_version.h"
//
#include "reboot.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Ethernet DDP Node " "\x1b[32m" "Pixel controller 8x with 2x DMX" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new Reboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	// NTP Client
	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	if (ntpClient.GetStatus() != ntpclient::Status::FAILED) {
		printf("Set RTC from System Clock\n");
		HwClock::Get()->SysToHc();
	}

	// mDNS
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_DDP, ddp::udp::PORT, "type=display");
	mDns.Print();

	// DDP
	DdpDisplayPixelConfiguration pixelConfiguration;

	DdpDisplayParams ddpDisplayParams(new StoreDdpDisplay);

	if (ddpDisplayParams.Load()) {
		ddpDisplayParams.Dump();
		ddpDisplayParams.Set(&pixelConfiguration);
	}

	DdpDisplay ddpDisplay(pixelConfiguration);

	ddpDisplay.Start();
	ddpDisplay.Print();

	uint8_t nTestPattern;
	PixelTestPattern *pPixelTestPattern = nullptr;

	if ((nTestPattern = ddpDisplayParams.GetTestPattern()) != 0) {
		pPixelTestPattern = new PixelTestPattern(static_cast<pixelpatterns::Pattern>(nTestPattern), ddpDisplay.GetActivePorts());
		hw.SetRebootHandler(new PixelReboot);
	}

	// RDMNet LLRP Only

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	char aDescription[RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "DDP Pixel %d-%s:%d", ddpDisplay.GetActivePorts(), PixelType::GetType(WS28xxMulti::Get()->GetType()), WS28xxMulti::Get()->GetCount());

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "Orange Pi Zero Pixel");

	RDMNetDevice llrpOnlyDevice(new RDMPersonality(aDescription, 0));
	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.SetRDMFactoryDefaults(new FactoryDefaults);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}

	llrpOnlyDevice.Init();
	llrpOnlyDevice.Start();
	llrpOnlyDevice.Print();

	display.SetTitle("DDP Pixel 8:%d", ddpDisplay.GetActivePorts());
	display.Set(2, displayudf::Labels::VERSION);
//	display.Set(3, displayudf::Labels::);
	display.Set(4, displayudf::Labels::IP);
	display.Set(5, displayudf::Labels::DEFAULT_GATEWAY);
//	display.Set(6, displayudf::Labels::);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show();

	RemoteConfig remoteConfig(remoteconfig::Node::DDP, remoteconfig::Output::PIXEL, ddpDisplay.GetActivePorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	if (pPixelTestPattern != nullptr) {
		display.TextStatus(PixelPatterns::GetName(static_cast<pixelpatterns::Pattern>(ddpDisplayParams.GetTestPattern())), ddpDisplayParams.GetTestPattern());
	} else {
		display.TextStatus("DDP Ready", Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);
	}

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		if (__builtin_expect((pPixelTestPattern == nullptr), 1)) {
			ddpDisplay.Run();
		} else {
			pPixelTestPattern->Run();
		}
		remoteConfig.Run();
		llrpOnlyDevice.Run();
		mDns.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
