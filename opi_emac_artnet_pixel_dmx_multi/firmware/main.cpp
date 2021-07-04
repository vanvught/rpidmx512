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

#include <stdio.h>
#include <stdint.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "artnet4node.h"
#include "artnet4params.h"
#include "storeartnet.h"
#include "storeartnet4.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

// Pixel
#include "pixeltestpattern.h"
#include "pixeltype.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"
#include "h3/ws28xxdmxstartstop.h"
#include "handleroled.h"
#include "storews28xxdmx.h"
#include "pixelreboot.h"
// DMX Output
#include "dmxparams.h"
#include "dmxsendmulti.h"
#include "storedmxsend.h"
// DMX Input
#include "dmxinput.h"
//
#include "lightset32with4.h"
// RDMNet LLRP Only
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "artnet/displayudfhandler.h"
#include "displayhandler.h"

using namespace artnet;

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	DisplayUdfHandler displayUdfHandler;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Ethernet Art-Net 4 Node " "\x1b[32m" "Pixel controller {8x 4 Universes}" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&displayUdfHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnetparams(&storeArtNet4);

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	// LightSet A - Pixel - 32 Universes

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}

	WS28xxDmxMulti pixelDmxMulti(pixelDmxConfiguration);
	pixelDmxMulti.SetPixelDmxHandler(new PixelDmxStartStop);
	WS28xxMulti::Get()->SetJamSTAPLDisplay(new HandlerOled);

	const auto nActivePorts = pixelDmxMulti.GetOutputPorts();

	auto nPages = static_cast<uint8_t>(nActivePorts);

	auto bIsSet = false;
	artnetparams.GetUniverse(0, bIsSet);

	if (bIsSet) {
		nPages = 9;
	} else {
		artnetparams.GetUniverse(1, bIsSet);
		if (bIsSet) {
			nPages = 9;
		}
	}

	ArtNet4Node node(nPages);
	artnetparams.Set(&node);

	const auto nUniverses = pixelDmxMulti.GetUniverses();

	uint8_t nPortProtocolIndex = 0;

	for (uint8_t nOutportIndex = 0; nOutportIndex < nActivePorts; nOutportIndex++) {
		auto isSet = false;
		const auto nStartUniversePort = ws28xxparms.GetStartUniversePort(nOutportIndex, isSet);
		if (isSet) {
			for (uint16_t u = 0; u < nUniverses; u++) {
				node.SetUniverse(nPortProtocolIndex, PortDir::OUTPUT, static_cast<uint16_t>(nStartUniversePort + u));
				nPortProtocolIndex++;
			}
			nPortProtocolIndex = static_cast<uint8_t>(nPortProtocolIndex + ArtNet::PORTS - nUniverses);
		} else {
			nPortProtocolIndex = static_cast<uint8_t>(nPortProtocolIndex + ArtNet::PORTS);
		}
	}

	uint8_t nTestPattern;
	PixelTestPattern *pPixelTestPattern = nullptr;

	if ((nTestPattern = ws28xxparms.GetTestPattern()) != 0) {
		pPixelTestPattern = new PixelTestPattern(static_cast<pixelpatterns::Pattern>(nTestPattern), nActivePorts);
		hw.SetRebootHandler(new PixelReboot);
	}

	// LightSet B - DMX - 2 Universes

	const auto portDir = artnetparams.GetDirection();
	const auto nAddress = static_cast<uint16_t>((artnetparams.GetNet() & 0x7F) << 8) | static_cast<uint16_t>((artnetparams.GetSubnet() & 0x0F) << 4);
	bIsSet = false;
	auto nUniverse = artnetparams.GetUniverse(0, bIsSet);
	uint32_t nDmxPorts = 0;

	if (bIsSet) {
		node.SetUniverse(32, portDir, static_cast<uint16_t>(nAddress | nUniverse));
		nDmxPorts++;
	}

	nUniverse = artnetparams.GetUniverse(1, bIsSet);

	if (bIsSet) {
		node.SetUniverse(33, portDir, static_cast<uint16_t>(nAddress | nUniverse));
		nDmxPorts++;
	}

	StoreDmxSend storeDmxSend;
	DMXSendMulti *pDmxOutput = nullptr;

	if (nDmxPorts != 0) {
		if (portDir == PortDir::INPUT) {
			auto *pDmxInput = new DmxInput;
			assert(pDmxInput != nullptr);

			node.SetArtNetDmx(pDmxInput);
			display.SetDmxInfo(displayudf::dmx::PortDir::INPUT , nDmxPorts);
		} else {
			pDmxOutput = new DMXSendMulti;
			assert(pDmxOutput != nullptr);

			DMXParams dmxparams(&storeDmxSend);

			if (dmxparams.Load()) {
				dmxparams.Dump();
				dmxparams.Set(pDmxOutput);
			}

			display.SetDmxInfo(displayudf::dmx::PortDir::OUTPUT , nDmxPorts);
		}
	}
	
	LightSet32with4 lightSet((pPixelTestPattern != nullptr) ? nullptr : &pixelDmxMulti, pDmxOutput);
	lightSet.Print();

	node.SetOutput(&lightSet);
	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(&storeArtNet);
	node.Print();

	// RDMNet LLRP Only

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	char aDescription[RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "Art-Net Pixel %d-%s:%d", nActivePorts, PixelType::GetType(WS28xxMulti::Get()->GetType()), WS28xxMulti::Get()->GetCount());

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

	node.SetRdmUID(llrpOnlyDevice.GetUID(), true);

	llrpOnlyDevice.Init();
	llrpOnlyDevice.Start();
	llrpOnlyDevice.Print();

	display.SetTitle("ArtNet Pixel 8:%dx%d", nActivePorts, WS28xxMulti::Get()->GetCount());
	display.Set(2, displayudf::Labels::VERSION);
	display.Set(3, displayudf::Labels::NODE_NAME);
	display.Set(4, displayudf::Labels::IP);
	display.Set(5, displayudf::Labels::DEFAULT_GATEWAY);
	display.Set(6, displayudf::Labels::DMX_DIRECTION);
	display.Printf(7, "%d-%s:%d G%d", nActivePorts, PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::PIXEL, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	if (pPixelTestPattern != nullptr) {
		display.TextStatus(PixelPatterns::GetName(static_cast<pixelpatterns::Pattern>(ws28xxparms.GetTestPattern())), ws28xxparms.GetTestPattern());
	} else {
		display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);
	}

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
		if (__builtin_expect((pPixelTestPattern != nullptr), 0)) {
			pPixelTestPattern->Run();
		}
	}
}

}
