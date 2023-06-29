/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "mdns.h"

#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"

#include "dmxserial.h"
#include "dmxserialparams.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "storeartnet.h"
#include "storedisplayudf.h"
#include "storedmxserial.h"
#include "storenetwork.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

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

	fw.Print("Art-Net 4 Node Serial [UART/SPI/I2C] {1 Universe}");
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);

	MDNS mDns;
	mDns.AddServiceRecord(nullptr, mdns::Services::CONFIG);
	mDns.AddServiceRecord(nullptr, mdns::Services::TFTP);
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, mdns::Services::HTTP, "node=Art-Net 4 [UART/SPI/I2C]");
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
#endif

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	ArtNet4Node node;

	StoreArtNet storeArtNet(DMXPORT_OFFSET);
	node.SetArtNetStore(&storeArtNet);

	ArtNetParams artnetParams(&storeArtNet);

	if (artnetParams.Load()) {
		artnetParams.Dump();
		artnetParams.Set(DMXPORT_OFFSET);
	}

	bool isSet;
	const auto nAddress = static_cast<uint16_t>((artnetParams.GetNet() & 0x7F) << 8) | static_cast<uint16_t>((artnetParams.GetSubnet() & 0x0F) << 4);
	const auto nUniverse = artnetParams.GetUniverse(0, isSet);
	const auto portDirection = artnetParams.GetDirection(0);

	if (portDirection == lightset::PortDir::OUTPUT) {
		node.SetUniverse(0, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nAddress | nUniverse));
	}

	DmxSerial dmxSerial;
	DmxSerialParams dmxSerialParams(new StoreDmxSerial);

	if (dmxSerialParams.Load()) {
		dmxSerialParams.Dump();
		dmxSerialParams.Set();
	}

	node.SetOutput(&dmxSerial);
	node.Print();

	dmxSerial.Init();
	dmxSerial.Print();

	display.SetTitle("Art-Net 4 %s", dmxSerial.GetSerialType());
	display.Set(2, displayudf::Labels::NODE_NAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(6, displayudf::Labels::HOSTNAME);

	DisplayUdfParams displayUdfParams(new StoreDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Dump();
		displayUdfParams.Set(&display);
	}

	display.Show(&node);

	uint32_t nFilesCount = dmxSerial.GetFilesCount();
	if (nFilesCount == 0) {
		display.Printf(7, "No files [SDCard?]");
	} else {
		display.Printf(7, "Channel%s: %d", nFilesCount == 1 ?  "" : "s", nFilesCount);
	}

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::SERIAL, node.GetActiveOutputPorts());
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	RDMPersonality *pPersonalities[1] = { new RDMPersonality("RDMNet LLRP device only", static_cast<uint16_t>(0)) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	constexpr char aLabel[] = "Art-Net 4 Serial [UART/SPI/I2C]";

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, (sizeof(aLabel) / sizeof(aLabel[0])) - 1);
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
		dmxSerial.Run();
		llrpOnlyDevice.Run();
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
