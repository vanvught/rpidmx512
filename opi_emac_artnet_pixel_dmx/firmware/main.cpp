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
#include <cassert>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "storenetwork.h"
#include "ledblink.h"

#if defined (ENABLE_HTTPD)
# include "mdns.h"
# include "mdnsservices.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "artnet/displayudfhandler.h"
#include "displayhandler.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"
#include "artnettriggerhandler.h"

#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "pixelreboot.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxstartstop.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#include "lightset4with4.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storeartnet.h"
#include "storedisplayudf.h"
#include "storedmxsend.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"
#include "storews28xxdmx.h"

#include "firmwareversion.h"
#include "software_version.h"

using namespace artnet;

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Ethernet Art-Net 4 Node " "\x1b[32m" "Pixel controller {1x 4 Universes} / DMX" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

#if defined (ENABLE_HTTPD)
	MDNS mDns;

	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=Art-Net Pixel DMX");
	mDns.Print();
#endif

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	StoreArtNet storeArtNet;
	ArtNetParams artnetParams(&storeArtNet);

	if (artnetParams.Load()) {
		artnetParams.Dump();
	}

	// LightSet A - Pixel - 4 Universes

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}

	WS28xxDmx pixelDmx(pixelDmxConfiguration);
	pixelDmx.SetPixelDmxHandler(new PixelDmxStartStop);

	auto isPixelUniverseSet = false;
	const auto nStartPixelUniverse = ws28xxparms.GetStartUniversePort(0, isPixelUniverseSet);

	auto isDmxUniverseSet = false;
	const auto nDmxUniverse = artnetParams.GetUniverse(0, isDmxUniverseSet);

	ArtNet4Node node(isDmxUniverseSet ? 2 : 1);
	artnetParams.Set(&node);

	if (isPixelUniverseSet) {
		const auto nUniverses = pixelDmx.GetUniverses();

		for (uint32_t nPortIndex = 0; nPortIndex < nUniverses; nPortIndex++) {
			node.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nStartPixelUniverse + nPortIndex));
		}
	}

	const auto nTestPattern = static_cast<pixelpatterns::Pattern>(ws28xxparms.GetTestPattern());
	PixelTestPattern pixelTestPattern(nTestPattern, 1);

	if (PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE) {
		hw.SetRebootHandler(new PixelReboot);
	}

	// LightSet B - DMX - 1 Universe

	if (isDmxUniverseSet) {
		const auto nAddress = static_cast<uint16_t>((artnetParams.GetNet() & 0x7F) << 8) | static_cast<uint16_t>((artnetParams.GetSubnet() & 0x0F) << 4);
		node.SetUniverse(4, artnetParams.GetDirection(0), static_cast<uint16_t>(nAddress | nDmxUniverse));
	}

	StoreDmxSend storeDmxSend;
	DmxParams dmxparams(&storeDmxSend);

	Dmx dmx;

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	DmxSend dmxSend;

	dmxSend.Print();

	DmxConfigUdp *pDmxConfigUdp = nullptr;

	if (isDmxUniverseSet) {
		pDmxConfigUdp = new DmxConfigUdp;
		assert(pDmxConfigUdp != nullptr);
	}

	display.SetDmxInfo(displayudf::dmx::PortDir::OUTPUT, isDmxUniverseSet ? 1 : 0);

	// Art-Net

	DisplayUdfHandler displayUdfHandler;
	node.SetArtNetDisplay(&displayUdfHandler);

	node.SetArtNetStore(&storeArtNet);

	// LightSet 4with4

	LightSet4with4 lightSet((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE) ? nullptr : &pixelDmx, isDmxUniverseSet ? &dmxSend : nullptr);
	lightSet.Print();

	ArtNetTriggerHandler triggerHandler(&lightSet, &pixelDmx);

	node.SetOutput(&lightSet);
	node.Print();

	// RDMNet LLRP Only

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	if (WS28xx::Get() == nullptr) {
		snprintf(aDescription, sizeof(aDescription) - 1, "Art-Net 4 Pixel-DMX");
	} else {
		snprintf(aDescription, sizeof(aDescription) - 1, "Art-Net 4 Pixel 1-%s:%d", PixelType::GetType(WS28xx::Get()->GetType()), WS28xx::Get()->GetCount());
	}

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "Orange Pi Zero Pixel");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.SetRDMFactoryDefaults(new FactoryDefaults);

	node.SetRdmUID(llrpOnlyDevice.GetUID(), true);

	llrpOnlyDevice.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}
	
	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();

	// Display

	display.SetTitle("Art-Net 4 Pixel 1 - DMX");
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(3, displayudf::Labels::NODE_NAME);
	display.Set(4, displayudf::Labels::IP);
	display.Set(5, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(6, displayudf::Labels::DMX_DIRECTION);
	display.Printf(7, "%s:%d G%d %s",
			PixelType::GetType(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	if (nTestPattern != pixelpatterns::Pattern::NONE) {
		display.ClearLine(6);
		display.Printf(6, "%s:%u", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
	}

	const auto nActivePorts = static_cast<uint32_t>(node.GetActiveInputPorts() + node.GetActiveOutputPorts());

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::PIXEL, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	llrpOnlyDevice.Start();
	node.Start();
	
	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		remoteConfig.Run();
		llrpOnlyDevice.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
		if (__builtin_expect((PixelTestPattern::GetPattern() != pixelpatterns::Pattern::NONE), 0)) {
			pixelTestPattern.Run();
		}
		if (pDmxConfigUdp != nullptr) {
			pDmxConfigUdp->Run();
		}
#if defined (ENABLE_HTTPD)
		mDns.Run();
#endif
	}
}

}
