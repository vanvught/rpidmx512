/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "mdns.h"

#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif


#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "showfileparams.h"
#include "showfileosc.h"
// Protocol handlers
#include "showfileprotocole131.h"
#include "showfileprotocolartnet.h"
// Format handlers
#include "olashowfile.h"

#include "rdmnetllrponly.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "storedisplayudf.h"
#include "storenetwork.h"
#include "storeremoteconfig.h"
#include "storeshowfile.h"

#include "firmwareversion.h"
#include "software_version.h"

void Hardware::RebootHandler() {
	ShowFile::Get()->Stop();
	ShowFile::Get()->GetProtocolHandler()->Stop();

	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);

		while (ConfigStore::Get()->Flash())
			;

		Network::Get()->Shutdown();

		printf("Rebooting ...\n");

		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...", Display7SegmentMessage::INFO_REBOOTING);
	}
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

	fw.Print("Showfile player");
	nw.Print();

	display.TextStatus(NetworkConst::MSG_MDNS_CONFIG, Display7SegmentMessage::INFO_MDNS_CONFIG, CONSOLE_YELLOW);

	MDNS mDns;
	mDns.AddServiceRecord(nullptr, mdns::Services::CONFIG);
	mDns.AddServiceRecord(nullptr, mdns::Services::TFTP);
#if defined (ENABLE_HTTPD)
	mDns.AddServiceRecord(nullptr, mdns::Services::HTTP, "node=Showfile player");
#endif
	mDns.Print();

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
#endif

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
	case showfile::Protocols::ARTNET:
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

#if defined (NODE_RDMNET_LLRP_ONLY)
	RDMNetLLRPOnly rdmNetLLRPOnly("Showfile player");

	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Print();
#endif

	RemoteConfig remoteConfig(remoteconfig::Node::SHOWFILE, remoteconfig::Output::PLAYER, 0);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if (remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	while (configStore.Flash())
		;

	display.SetTitle("Showfile player");
	display.Set(2, displayudf::Labels::HOSTNAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);

	DisplayUdfParams displayUdfParams(new StoreDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Dump();
		displayUdfParams.Set(&display);
	}

	display.Show();

	pShowFile->SetShowFile(showFileParams.GetShow());
	pShowFile->Print();

	if (showFileParams.IsAutoStart()) {
		pShowFile->Start();
	}

	// Fixed row 5, 6, 7
	display.Printf(5, showFileParams.GetProtocol() == showfile::Protocols::ARTNET ? "Art-Net" : "sACN E1.31");

	if (showFileParams.GetProtocol() == showfile::Protocols::ARTNET) {
		if (showFileParams.IsArtNetBroadcast()) {
			Display::Get()->PutString(" <Broadcast>");
		}
	}

	if (pShowFileProtocolHandler->IsSyncDisabled()) {
		display.Printf(6, "<No synchronization>");
	}

	displayHandler.ShowShowFileStatus();

	hw.SetMode(hardware::ledblink::Mode::NORMAL);
	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		pShowFile->Run();
		pShowFileProtocolHandler->Run();
		oscServer.Run();
		remoteConfig.Run();
		configStore.Flash();
		mDns.Run();
#if defined (NODE_RDMNET_LLRP_ONLY)
		rdmNetLLRPOnly.Run();
#endif
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
		display.Run();
		hw.Run();
	}
}
