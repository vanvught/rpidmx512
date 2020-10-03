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

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "ntpclient.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "display7segment.h"

#include "networkconst.h"

#include "artnet4node.h"
#include "artnet4params.h"
#include "storeartnet.h"
#include "storeartnet4.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

#include "ipprog.h"

#include "identify.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "rdmdeviceparams.h"
#include "rdmsensorsparams.h"

#include "artnetrdmresponder.h"

#include "displayudfhandler.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "lightsetchain.h"

#if defined (ORANGE_PI)
# include "spiflashinstall.h"
# include "spiflashstore.h"
# include "remoteconfig.h"
# include "remoteconfigparams.h"
# include "storeremoteconfig.h"
# include "storedisplayudf.h"
# include "storerdmdevice.h"
# include "storerdmsensors.h"
# include "storetlc59711.h"
#endif

#if defined (ORANGE_PI_ONE)
# include "slushdmx.h"
# define BOARD_NAME	"Slushengine"
#else
# include "sparkfundmx.h"
# include "sparkfundmxconst.h"
# define BOARD_NAME "Sparkfun"
# include "storesparkfundmx.h"
# include "storemotors.h"
#endif

#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	DisplayUdfHandler displayUdfHandler;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

#if defined (ORANGE_PI)
	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;
#endif

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Stepper L6470");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new ArtNetReboot);

	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkDisplay(&displayUdfHandler);
#if defined (ORANGE_PI)
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
#else
	nw.Init();
#endif
	nw.Print();

	NtpClient ntpClient;
	ntpClient.SetNtpClientDisplay(&displayUdfHandler);
	ntpClient.Start();
	ntpClient.Print();

	if (ntpClient.GetStatus() != NtpClientStatus::FAILED) {
		printf("Set RTC from System Clock\n");
		HwClock::Get()->SysToHc();
	}

	LightSet *pBoard;
	uint32_t nMotorsConnected = 0;

#if defined (ORANGE_PI_ONE)
	SlushDmx *pSlushDmx = new SlushDmx(false);	// Do not use SPI busy check
	assert(pSlushDmx != 0);

	pSlushDmx->ReadConfigFiles();

	nMotorsConnected = pSlushDmx->GetMotorsConnected();

	pBoard = pSlushDmx;
#else
	StoreSparkFunDmx storeSparkFunDmx;
	StoreMotors storeMotors;

	struct TSparkFunStores sparkFunStores;
	sparkFunStores.pSparkFunDmxParamsStore = &storeSparkFunDmx;
	sparkFunStores.pModeParamsStore = &storeMotors;
	sparkFunStores.pMotorParamsStore = &storeMotors;
	sparkFunStores.pL6470ParamsStore = &storeMotors;

	display.TextStatus(SparkFunDmxConst::MSG_INIT, Display7SegmentMessage::INFO_SPARKFUN, CONSOLE_YELLOW);

	SparkFunDmx *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != 0);

	pSparkFunDmx->ReadConfigFiles(&sparkFunStores);
	pSparkFunDmx->SetModeStore(&storeMotors);

	nMotorsConnected = pSparkFunDmx->GetMotorsConnected();

	pBoard = pSparkFunDmx;
#endif

#if defined (ORANGE_PI)
	StoreTLC59711 storeTLC59711;
	TLC59711DmxParams pwmledparms(&storeTLC59711);
#else
	TLC59711DmxParams pwmledparms;
#endif

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != 0);
#if defined (ORANGE_PI)
			pTLC59711Dmx->SetTLC59711DmxStore(&storeTLC59711);
#endif
			pwmledparms.Dump();
			pwmledparms.Set(pTLC59711Dmx);

			display.Printf(7, "%s:%d", pwmledparms.GetLedTypeString(pwmledparms.GetLedType()), pwmledparms.GetLedCount());

			LightSetChain *pChain = new LightSetChain;
			assert(pChain != 0);

			pChain->Add(pBoard, 0);
			pChain->Add(pTLC59711Dmx, 1);
			pChain->Dump();

			pBoard = pChain;
		}
	}

	pBoard->SetLightSetDisplay(&displayUdfHandler);

	char aDescription[64];
	if (isLedTypeSet) {
		snprintf(aDescription, sizeof(aDescription) - 1, "%s [%d] with %s [%d]", BOARD_NAME, nMotorsConnected, pwmledparms.GetLedTypeString(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
	} else {
		snprintf(aDescription, sizeof(aDescription) - 1, "%s [%d]", BOARD_NAME, nMotorsConnected);
	}

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNet4Node node;
#if defined (ORANGE_PI)
	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnetparams(&storeArtNet4);
#else
	ArtNet4Params artnetparams;
#endif

	node.SetLongName(aDescription);

	if (artnetparams.Load()) {
		artnetparams.Set(&node);
		artnetparams.Dump();
	}

	Identify identify;

	node.SetIpProgHandler(new IpProg);
	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetDirectUpdate(false);
#if defined (ORANGE_PI)
	node.SetArtNetStore(StoreArtNet::Get());
#endif
	node.SetOutput(pBoard);
	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, pBoard);

#if defined (ORANGE_PI)
	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);
	RdmResponder.SetRDMDeviceStore(&storeRdmDevice);

	StoreRDMSensors storeRdmSensors;
	RDMSensorsParams rdmSensorsParams(&storeRdmSensors);
#else
	RDMDeviceParams rdmDeviceParams;
	RDMSensorsParams rdmSensorsParams;
#endif

	if(rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&RdmResponder);
		rdmDeviceParams.Dump();
	}

	if (rdmSensorsParams.Load()) {
		rdmSensorsParams.Set();
		rdmSensorsParams.Dump();
	}

	RdmResponder.Init();
	RdmResponder.Print();

	node.SetRdmHandler(&RdmResponder, true);
	node.Print();

	pBoard->Print();

	display.SetTitle("Eth Art-Net 4 L6470");
	display.Set(2, DISPLAY_UDF_LABEL_NODE_NAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_DMX_START_ADDRESS);

#if defined (ORANGE_PI)
	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);
#else
	DisplayUdfParams displayUdfParams;
#endif

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

#if defined (ORANGE_PI)
	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, REMOTE_CONFIG_MODE_STEPPER, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;
#endif

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		ntpClient.Run();
		identify.Run();
#if defined (ORANGE_PI)
		remoteConfig.Run();
		spiFlashStore.Flash();
#endif
		lb.Run();
		display.Run();
	}
}

}
