/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "ledblink.h"

#if defined (ENABLE_HTTPD)
# include "mdns.h"
# include "mdnsservices.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"
#include "display_timeout.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"
#include "artnetdiscovery.h"
#include "artnetreboot.h"
#include "artnet/displayudfhandler.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "rdmdeviceparams.h"
#include "dmxconfigudp.h"

#include "dmxinput.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "storeartnet.h"
#include "storedisplayudf.h"
#include "storedmxsend.h"
#include "storenetwork.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Art-Net 4 Node " "\x1b[32m" "DMX/RDM controller {1 Universe}" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

#if defined (ENABLE_HTTPD)
	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=Art-Net 4 DMX/RDM");
	mDns.Print();
#endif

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	StoreArtNet storeArtNet;
	ArtNetParams artnetParams(&storeArtNet);

	ArtNet4Node node;

	if (artnetParams.Load()) {
		artnetParams.Set(&node);
		artnetParams.Dump();
	}

	DisplayUdfHandler displayUdfHandler;
	node.SetArtNetDisplay(&displayUdfHandler);

	node.SetArtNetStore(&storeArtNet);

	bool isSet;
	node.SetUniverseSwitch(0, artnetParams.GetDirection(0), artnetParams.GetUniverse(0, isSet));

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

	if (node.GetActiveOutputPorts() != 0) {
		node.SetOutput(&dmxSend);
		pDmxConfigUdp = new DmxConfigUdp;
		assert(pDmxConfigUdp != nullptr);
	}

	DmxInput dmxInput;

	if (node.GetActiveInputPorts() != 0) {
		node.SetArtNetDmx(&dmxInput);
	}

	StoreRDMDevice storeRdmDevice;

	if (node.GetActiveOutputPorts() != 0) {
		if (artnetParams.IsRdm()) {
			auto pDiscovery = new ArtNetRdmController(1);
			assert(pDiscovery != nullptr);

			RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

			if (rdmDeviceParams.Load()) {
				rdmDeviceParams.Set(pDiscovery);
				rdmDeviceParams.Dump();
			}

			pDiscovery->Init();
			pDiscovery->Print();

			display.TextStatus(ArtNetMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);

			pDiscovery->Full(0);

			node.SetRdmHandler(pDiscovery);
		}
	}

	node.Print();

	const auto nActivePorts = static_cast<uint32_t>(node.GetActiveInputPorts() + node.GetActiveOutputPorts());

	display.SetTitle("Art-Net 4 %s", artnetParams.GetDirection(0) == lightset::PortDir::INPUT ? "DMX Input" : (artnetParams.IsRdm() ? "RDM" : "DMX Output"));
	display.Set(2, displayudf::Labels::NODE_NAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE);
	display.Set(6, displayudf::Labels::HOSTNAME);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, artnetParams.IsRdm() ? remoteconfig::Output::RDM : remoteconfig::Output::DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
		if (pDmxConfigUdp != nullptr) {
			pDmxConfigUdp->Run();
		}
#if defined (ENABLE_HTTPD)
		mDns.Run();
#endif
	}
}

}
