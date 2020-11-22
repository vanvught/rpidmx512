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

#include <cassert>
#include <stdio.h>
#include <stdint.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "showfileparams.h"
#include "storeshowfile.h"
#include "showfileosc.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

// LLRP Only Device
#include "rdmnetllrponly.h"
#include "rdm_e120.h"

// LLRP Handlers
#include "factorydefaults.h"
#include "displayudfnetworkhandler.h"

// Reboot handler
#include "reboot.h"

// Display
#include "displayhandler.h"

// Format handlers
#include "olashowfile.h"

// Protocol handlers
#include "showfileprotocole131.h"
#include "showfileprotocolartnet.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Showfile player");

	hw.SetRebootHandler(new Reboot);
	hw.SetLed(HARDWARE_LED_ON);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(new DisplayUdfNetworkHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	StoreShowFile storeShowFile;
	ShowFileParams showFileParams(&storeShowFile);

	if (showFileParams.Load()) {
		showFileParams.Dump();
	}

	ShowFile *pShowFile = nullptr;

	switch (showFileParams.GetFormat()) {
//		case SHOWFILE_FORMAT_:
//			break;
		default:
			pShowFile = new OlaShowFile;
			break;
	}

	assert(pShowFile != nullptr);

	DisplayHandler displayHandler;
	pShowFile->SetShowFileDisplay(&displayHandler);

	ShowFileProtocolHandler *pShowFileProtocolHandler = nullptr;

	switch (showFileParams.GetProtocol()) {
	case ShowFileProtocols::ARTNET:
		pShowFileProtocolHandler = new ShowFileProtocolArtNet;
		break;
	default:
		pShowFileProtocolHandler = new ShowFileProtocolE131;
		break;
	}

	assert(pShowFileProtocolHandler != nullptr);
	pShowFile->SetProtocolHandler(pShowFileProtocolHandler);

	ShowFileOSC oscServer;

	showFileParams.Set();

	oscServer.Start();
	oscServer.Print();

	pShowFileProtocolHandler->Start();
	pShowFileProtocolHandler->Print();

	RDMNetLLRPOnly rdmNetLLRPOnly("Showfile player");

	rdmNetLLRPOnly.GetRDMNetDevice()->SetRDMFactoryDefaults(new FactoryDefaults);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Print();
	rdmNetLLRPOnly.Start();

	RemoteConfig remoteConfig(REMOTE_CONFIG_SHOWFILE, REMOTE_CONFIG_MODE_PLAYER, 0);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.SetTitle("Showfile player");
	display.Set(2, DISPLAY_UDF_LABEL_HOSTNAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);

	DisplayUdfParams displayUdfParams(new StoreDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show();

	pShowFile->SetShowFile(showFileParams.GetShow());
	pShowFile->Print();

	if (showFileParams.IsAutoStart()) {
		pShowFile->Start();
	}

	// Fixed row 5, 6, 7
	display.Printf(5, showFileParams.GetProtocol() == ShowFileProtocols::ARTNET ? "Art-Net" : "sACN E1.31");
	if (showFileParams.GetProtocol() == ShowFileProtocols::ARTNET) {
		if (showFileParams.IsArtNetBroadcast()) {
			Display::Get()->PutString(" <Broadcast>");
		}
	}
	if (pShowFileProtocolHandler->IsSyncDisabled()) {
		display.Printf(6, "<No synchronization>");
	}
	displayHandler.ShowShowFileStatus();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		//
		pShowFile->Run();
		pShowFileProtocolHandler->Run();
		oscServer.Run();
		//
		rdmNetLLRPOnly.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
