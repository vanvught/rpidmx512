/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "console.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "networkconst.h"
#include "artnetconst.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "rdmdeviceparams.h"
#include "storerdmdevice.h"

#include "identify.h"
#include "artnetrdmresponder.h"

#include "ipprog.h"
#include "displayudfhandler.h"

#include "slushdmx.h"
#include "sparkfundmx.h"

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "storetlc59711.h"

#include "lightsetchain.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"

#include "software_version.h"

#if defined (ORANGE_PI_ONE)
 #define BOARD_NAME	"Slushengine"
#else
 #define BOARD_NAME "Sparkfun"
#endif

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
//	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;
	StoreTLC59711 storeTLC59711;

	LightSet *pBoard;

	fw.Print();

	console_puts("Ethernet Art-Net 3 Node ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Stepper L6470");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);

#if defined (ORANGE_PI_ONE)
	SlushDmx *pSlushDmx = new SlushDmx(false);	// Do not use SPI busy check
	assert(pSlushDmx != 0);
	pSlushDmx->ReadConfigFiles();
	pBoard = pSlushDmx;
#else
	SparkFunDmx *pSparkFunDmx = new SparkFunDmx;
	assert(pSparkFunDmx != 0);
	pSparkFunDmx->ReadConfigFiles();
	pBoard = pSparkFunDmx;
#endif

#if defined (ORANGE_PI)
	TLC59711DmxParams pwmledparms((TLC59711DmxParamsStore *) &storeTLC59711);
#else
	TLC59711DmxParams pwmledparms;
#endif

	bool isLedTypeSet = false;

	if (pwmledparms.Load()) {
		if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
			TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
			assert(pTLC59711Dmx != 0);
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

	char aDescription[64];
	snprintf(aDescription, sizeof(aDescription) - 1, "%s%s", BOARD_NAME, isLedTypeSet ? " with TLC59711" : "");

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
	display.TextStatus(ArtNetConst::MSG_NODE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS);

	ArtNetNode node;
	ArtNetParams artnetparams((ArtNetParamsStore *)spiFlashStore.GetStoreArtNet());

	if (artnetparams.Load()) {
		artnetparams.Set(&node);
		artnetparams.Dump();
	}

	Identify identify;

	IpProg ipprog;
	node.SetIpProgHandler(&ipprog);

	DisplayUdfHandler displayUdfHandler(&node);
	node.SetArtNetDisplay(&displayUdfHandler);

	node.SetDirectUpdate(false);;
	node.SetArtNetStore((ArtNetStore *)spiFlashStore.GetStoreArtNet());
	node.SetOutput(pBoard);
	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	RDMPersonality personality(aDescription, pBoard->GetDmxFootprint());
	ArtNetRdmResponder RdmResponder(&personality, pBoard);

	RDMDeviceParams rdmDeviceParams;

	if(rdmDeviceParams.Load()) {
		rdmDeviceParams.Set((RDMDevice *)&RdmResponder);
		rdmDeviceParams.Dump();
	}

	RdmResponder.Init();
	RdmResponder.Print();

	node.SetRdmHandler((ArtNetRdm *)&RdmResponder, true);
	node.Print();

	display.SetTitle("Eth Art-Net 3 L6470");
	display.Set(2, DISPLAY_UDF_LABEL_NODE_NAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_AP);

	StoreDisplayUdf storeDisplayUdf;
#if defined (ORANGE_PI)
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);
#else
	DisplayUdfParams displayUdfParams;
#endif

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

//	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET,  REMOTE_CONFIG_MODE_PIXEL, node.GetActiveOutputPorts());
//
//	StoreRemoteConfig storeRemoteConfig;
//	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);
//
//	if(remoteConfigParams.Load()) {
//		remoteConfigParams.Set(&remoteConfig);
//		remoteConfigParams.Dump();
//	}
//
//	while (spiFlashStore.Flash())
//		;


	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
	display.TextStatus(ArtNetConst::MSG_NODE_START, DISPLAY_7SEGMENT_MSG_INFO_NODE_START);

	node.Start();

	console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
	display.TextStatus(ArtNetConst::MSG_NODE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
		identify.Run();
//		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
