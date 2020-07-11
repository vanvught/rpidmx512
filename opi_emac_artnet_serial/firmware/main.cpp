/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "networkconst.h"

#include "artnet4node.h"
#include "artnet4params.h"
#include "storeartnet.h"
#include "storeartnet4.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

#include "ipprog.h"

#include "dmxserial.h"
#include "dmxserialparams.h"
#include "storedmxserial.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayudfhandler.h"
#include "displayhandler.h"

// RDMNet LLRP Device Only
#include "rdm_e120.h"
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"
#include "storerdmdevice.h"

#include "identify.h"
#include "factorydefaults.h"

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

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color (CONSOLE_GREEN);
	console_puts("Serial [UART/SPI/I2C]");
	console_set_fg_color (CONSOLE_WHITE);
	console_puts(" {1 Universe}\n");

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new ArtNetReboot);

	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(&displayUdfHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNet4Node node;

	StoreArtNet storeArtNet;
	StoreArtNet4 storeArtNet4;

	ArtNet4Params artnetparams(&storeArtNet4);

	if (artnetparams.Load()) {
		artnetparams.Set(&node);
		artnetparams.Dump();
	}

	node.SetIpProgHandler(new IpProg);
	node.SetArtNetDisplay(&displayUdfHandler);
	node.SetArtNetStore(StoreArtNet::Get());
	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	DmxSerial dmxSerial;
	DmxSerialParams dmxSerialParams(new StoreDmxSerial);

	if (dmxSerialParams.Load()) {
		dmxSerialParams.Set();
		dmxSerialParams.Dump();
	}

	node.SetOutput(&dmxSerial);
	node.Print();

	dmxSerial.Init();
	dmxSerial.Print();

	display.SetTitle("Art-Net 4 %s", dmxSerial.GetSerialType());
	display.Set(2, DISPLAY_UDF_LABEL_NODE_NAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_HOSTNAME);

	uint32_t nFilesCount = dmxSerial.GetFilesCount();
	if (nFilesCount == 0) {
		display.Printf(7, "No files [SDCard?]");
	} else {
		display.Printf(7, "Channel%s: %d", nFilesCount == 1 ?  "" : "s", nFilesCount);
	}

	DisplayUdfParams displayUdfParams(new StoreDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, REMOTE_CONFIG_MODE_SERIAL, node.GetActiveOutputPorts());
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	Identify identify;

	RDMNetDevice device(new RDMPersonality("RDMNet LLRP device only", 0));

	StoreRDMDevice storeRdmDevice;

	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	device.SetRDMDeviceStore(&storeRdmDevice);

	const char aLabel[] = "Art-Net 4 Serial [UART/SPI/I2C]";
	device.SetLabel(RDM_ROOT_DEVICE, aLabel, (sizeof(aLabel) / sizeof(aLabel[0])) - 1);

	device.SetRDMFactoryDefaults(new FactoryDefaults);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&device);
		rdmDeviceParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	device.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	device.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);

	device.Init();
	device.Print();

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();
	device.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		//
		node.Run();
		dmxSerial.Run();
		//
		device.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
