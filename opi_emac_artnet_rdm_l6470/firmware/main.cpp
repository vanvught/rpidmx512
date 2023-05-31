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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "storenetwork.h"

#include "mdns.h"

#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "ntpclient.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "display7segment.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "storeartnet.h"
#include "artnetmsgconst.h"

#include "rdmdeviceresponder.h"
#include "factorydefaults.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "rdmsubdevicesparams.h"
#endif

#include "artnetrdmresponder.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"
#include "storedisplayudf.h"
#include "storerdmdevice.h"
#include "storerdmsensors.h"
#if defined (ENABLE_RDM_SUBDEVICES)
# include "storerdmsubdevices.h"
#endif
#include "storetlc59711.h"
#include "storesparkfundmx.h"
#include "storemotors.h"

#define BOARD_NAME "Sparkfun"

#include "sparkfundmx.h"
#include "sparkfundmxconst.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

static constexpr uint32_t DMXPORT_OFFSET = 0;

void Hardware::RebootHandler() {
	ArtNet4Node::Get()->Stop();
}

void main() {
	Hardware hw;
	DisplayUdf display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	StoreNetwork storeNetwork;
	Network nw(&storeNetwork);
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("Art-Net 4 Stepper L6470");
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);

	MDNS mDns;
	mDns.AddServiceRecord(nullptr, mdns::Services::CONFIG);
	mDns.AddServiceRecord(nullptr, mdns::Services::TFTP);
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, mdns::Services::HTTP, "node=Art-Net 4 Stepper L6470");
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
#endif

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	LightSet *pBoard;
	uint32_t nMotorsConnected = 0;

	StoreSparkFunDmx storeSparkFunDmx;
	StoreMotors storeMotors;

	struct TSparkFunStores sparkFunStores;
	sparkFunStores.pSparkFunDmxParamsStore = &storeSparkFunDmx;
	sparkFunStores.pModeParamsStore = &storeMotors;
	sparkFunStores.pMotorParamsStore = &storeMotors;
	sparkFunStores.pL6470ParamsStore = &storeMotors;

	display.TextStatus(SparkFunDmxConst::MSG_INIT, Display7SegmentMessage::INFO_SPARKFUN, CONSOLE_YELLOW);

	auto *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != nullptr);

	pSparkFunDmx->ReadConfigFiles(&sparkFunStores);
	pSparkFunDmx->SetModeStore(&storeMotors);

	nMotorsConnected = pSparkFunDmx->GetMotorsConnected();

	pBoard = pSparkFunDmx;

	StoreTLC59711 storeTLC59711;
	TLC59711DmxParams pwmledparms(&storeTLC59711);

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			auto *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != nullptr);
			pTLC59711Dmx->SetTLC59711DmxStore(&storeTLC59711);
			pwmledparms.Dump();
			pwmledparms.Set(pTLC59711Dmx);

			display.Printf(7, "%s:%d", pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());

			auto *pChain = new LightSetChain;
			assert(pChain != nullptr);

			pChain->Add(pBoard, 0);
			pChain->Add(pTLC59711Dmx, 1);
			pChain->Dump();

			pBoard = pChain;
		}
	}

	char aDescription[64];
	if (isLedTypeSet) {
		snprintf(aDescription, sizeof(aDescription) - 1, "%s [%d] with %s [%d]", BOARD_NAME, nMotorsConnected, pwmledparms.GetType(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	} else {
		snprintf(aDescription, sizeof(aDescription) - 1, "%s [%d]", BOARD_NAME, nMotorsConnected);
	}

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNet4Node node;
	StoreArtNet storeArtNet(DMXPORT_OFFSET);

	ArtNetParams artnetparams(&storeArtNet);
	node.SetArtNetStore(&storeArtNet);

	node.SetLongName(aDescription);

	if (artnetparams.Load()) {
		artnetparams.Dump();
		artnetparams.Set(DMXPORT_OFFSET);
	}

	node.SetOutput(pBoard);
	bool isSet;
	node.SetUniverseSwitch(0, lightset::PortDir::OUTPUT, artnetparams.GetUniverse(0, isSet));

	RDMPersonality *pRDMPersonalities[1] = { new  RDMPersonality(aDescription, pBoard)};

	ArtNetRdmResponder rdmResponder(pRDMPersonalities, 1);

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
	rdmResponder.SetRDMDeviceStore(&storeRdmDevice);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);

# if defined (ENABLE_RDM_SUBDEVICES)
	StoreRDMSubDevices storeRdmSubDevices;
	RDMSubDevicesParams rdmSubDevicesParams(&storeRdmSubDevices);
# endif

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Dump();
		rdmSensorsParams.Set();
	}

#if defined (ENABLE_RDM_SUBDEVICES)
	if (rdmSubDevicesParams.Load()) {
		rdmSubDevicesParams.Dump();
		rdmSubDevicesParams.Set();
	}
#endif

	rdmResponder.Init();

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Dump();
		rdmDeviceParams.Set(&rdmResponder);
	}

	rdmResponder.Print();

	node.SetRdmHandler(&rdmResponder, true);
	node.Print();

	pBoard->Print();

	display.SetTitle("Art-Net 4 L6470");
	display.Set(2, displayudf::Labels::NODE_NAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(6, displayudf::Labels::DMX_START_ADDRESS);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);


	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::STEPPER, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (configStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		ntpClient.Run();
		remoteConfig.Run();
		configStore.Flash();
		mDns.Run();
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
		display.Run();
		hw.Run();
	}
}

