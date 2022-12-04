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

#include <cstdint>

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

#include "e131.h"
#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"

#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#include "dmxinput.h"

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
#include "storedmxsend.h"
#include "storee131.h"
#include "storenetwork.h"
#if defined (NODE_RDMNET_LLRP_ONLY)
# include "storerdmdevice.h"
#endif
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

void Hardware::RebootHandler() {
	Dmx::Get()->Blackout();
	E131Bridge::Get()->Stop();
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

	fw.Print("sACN E1.31 DMX");

	hw.SetLed(hardware::LedStatus::ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=sACN E1.31 DMX");
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
	httpDaemon.Start();
#endif

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	StoreE131 storeE131;
	E131Params e131params(&storeE131);

	E131Bridge bridge;

	if (e131params.Load()) {
		e131params.Dump();
		e131params.Set();
	}

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		const auto portDirection = e131params.GetDirection(nPortIndex);
		bool bIsSet;
		const auto nUniverse = e131params.GetUniverse(nPortIndex, bIsSet);
		bridge.SetUniverse(nPortIndex, portDirection, nUniverse);
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

	if (bridge.GetActiveOutputPorts() != 0) {
		bridge.SetOutput(&dmxSend);
		pDmxConfigUdp = new DmxConfigUdp;
		assert(pDmxConfigUdp != nullptr);
	}

	DmxInput dmxInput;

	if (bridge.GetActiveInputPorts() != 0) {
		bridge.SetE131Dmx(&dmxInput);
	}

	const auto nActivePorts = bridge.GetActiveInputPorts() + bridge.GetActiveOutputPorts();

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_CONFIG, Display7SegmentMessage::INFO_RDMNET_CONFIG, CONSOLE_YELLOW);
	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN E1.31 DMX %u", nActivePorts);

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, H3_BOARD_NAME " DMX");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
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

	bridge.Print();

	display.SetTitle("sACN E1.31 DMX %u", nActivePorts);
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::HOSTNAME);
	display.Set(4, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_B);
	display.Set(6, displayudf::Labels::BOARDNAME);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Dump();
		displayUdfParams.Set(&display);
	}

	display.Show(&bridge);

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	while (configStore.Flash())
		;

#if defined (NODE_RDMNET_LLRP_ONLY)
	display.TextStatus(RDMNetConst::MSG_START, Display7SegmentMessage::INFO_RDMNET_START, CONSOLE_YELLOW);

	llrpOnlyDevice.Start();

	display.TextStatus(RDMNetConst::MSG_STARTED, Display7SegmentMessage::INFO_RDMNET_STARTED, CONSOLE_GREEN);
#endif

	display.TextStatus(E131MsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	bridge.Start();

	display.TextStatus(E131MsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		remoteConfig.Run();
#if defined (NODE_RDMNET_LLRP_ONLY)
		llrpOnlyDevice.Run();
#endif
		configStore.Flash();
		lb.Run();
		display.Run();
		if (pDmxConfigUdp != nullptr) {
			pDmxConfigUdp->Run();
		}
		mDns.Run();
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
	}
}

}
