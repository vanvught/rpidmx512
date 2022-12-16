/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "networkconst.h"
#include "ledblink.h"

#include "mdns.h"
#include "mdnsservices.h"
#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"
#include "display_timeout.h"

#include "ddpdisplay.h"

#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "pixeldmxparams.h"
#include "ws28xxmulti.h"
#include "ws28xxdmxmulti.h"
#include "ws28xxdmxstartstop.h"
#include "handleroled.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmdeviceparams.h"
# include "rdmnetdevice.h"
# include "rdmnetconst.h"
# include "rdmpersonality.h"
# include "rdm_e120.h"
# include "factorydefaults.h"
#endif

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"

#include "storedisplayudf.h"
#include "storenetwork.h"
#if defined (NODE_RDMNET_LLRP_ONLY)
# include "storerdmdevice.h"
#endif
#include "storeremoteconfig.h"
#include "storepixeldmx.h"

#include "firmwareversion.h"
#include "software_version.h"

void Hardware::RebootHandler() {
	WS28xxMulti::Get()->Blackout();
	DdpDisplay::Get()->Stop();
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

	fw.Print("\x1b[32m" "DDP Pixel controller 8x 4U" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_DDP, ddp::UDP_PORT, mdns::Protocol::UDP, "type=display");
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=DDP Pixel");
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
	httpDaemon.Start();
#endif

	PixelDmxConfiguration pixelDmxConfiguration;

	StorePixelDmx storePixelDmx;
	PixelDmxParams pixelDmxParams(&storePixelDmx);

	if (pixelDmxParams.Load()) {
		pixelDmxParams.Dump();
		pixelDmxParams.Set(&pixelDmxConfiguration);
	}

	WS28xxDmxMulti pixelDmxMulti(pixelDmxConfiguration);
	WS28xxMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);
	pixelDmxMulti.SetPixelDmxHandler(new PixelDmxStartStop);

	DdpDisplay ddpDisplay;

	const auto nActivePorts = pixelDmxMulti.GetOutputPorts();

	ddpDisplay.SetCount(pixelDmxMulti.GetGroups(), pixelDmxMulti.GetChannelsPerPixel(), nActivePorts);

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(pixelDmxParams.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, nActivePorts);

	ddpDisplay.SetOutput(&pixelDmxMulti);
	ddpDisplay.Print();

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_CONFIG, Display7SegmentMessage::INFO_RDMNET_CONFIG, CONSOLE_YELLOW);

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "DDP Display %u-%s:%d", nActivePorts, PixelType::GetType(WS28xxMulti::Get()->GetType()), WS28xxMulti::Get()->GetCount());

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "Orange Pi Zero Pixel");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Dump();
		rdmDeviceParams.Set(&llrpOnlyDevice);
	}

	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();
#endif

	display.SetTitle("DDP Pixel %d", nActivePorts);
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(3, displayudf::Labels::HOSTNAME);
	display.Set(4, displayudf::Labels::IP);
	display.Set(5, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(6, displayudf::Labels::BOARDNAME);
	display.Printf(7, "%s:%d G%d %s",
		PixelType::GetType(pixelDmxConfiguration.GetType()),
		pixelDmxConfiguration.GetCount(),
		pixelDmxConfiguration.GetGroupingCount(),
		PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show();

	if (nTestPattern != pixelpatterns::Pattern::NONE) {
		display.ClearLine(6);
		display.Printf(6, "%s:%u", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
	}

	RemoteConfig remoteConfig(remoteconfig::Node::DDP, remoteconfig::Output::PIXEL, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (configStore.Flash())
		;

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_START, Display7SegmentMessage::INFO_RDMNET_START, CONSOLE_YELLOW);

	llrpOnlyDevice.Start();

	display.TextStatus(RDMNetConst::MSG_STARTED, Display7SegmentMessage::INFO_RDMNET_STARTED, CONSOLE_GREEN);
#endif

	display.TextStatus("DDP Display Start", Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	ddpDisplay.Start();

	display.TextStatus("DDP Display Started", Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		ddpDisplay.Run();
		remoteConfig.Run();
		configStore.Flash();
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
		mDns.Run();
#if defined (NODE_RDMNET_LLRP_ONLY)
		llrpOnlyDevice.Run();
#endif
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
		display.Run();
		lb.Run();
	}
}

}
