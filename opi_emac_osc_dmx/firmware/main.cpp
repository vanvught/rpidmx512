/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "display.h"

#include "mdns.h"

#include "oscserver.h"
#include "oscserverparams.h"
#include "oscservermsgconst.h"
// DMX Out
#include "dmxparams.h"
#include "dmxsend.h"
#include "dmxconfigudp.h"

#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"


#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

void Hardware::RebootHandler() {
	Dmx::Get()->Blackout();
}

int main() {
	Hardware hw;
	Display display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	MDNS mDns;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("OSC Server DMX");
	nw.Print();

	OSCServerParams params;
	OscServer server;

	params.Load();
	params.Set(&server);

	mDns.ServiceRecordAdd(nullptr, mdns::Services::OSC, "type=server", server.GetPortIncoming());

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	Dmx dmx;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	DmxSend dmxSend;
	dmxSend.Print();

	server.SetOutput(&dmxSend);
	server.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::OSC, remoteconfig::Output::DMX, 1);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	for (uint32_t i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "OSC DMX 1");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	display.TextStatus(OscServerMsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	server.Start();

	display.TextStatus(OscServerMsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	while (configStore.Flash())
		;

	mDns.Print();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
		remoteConfig.Run();
		configStore.Flash();
		mDns.Run();
		display.Run();
		hw.Run();
	}
}
