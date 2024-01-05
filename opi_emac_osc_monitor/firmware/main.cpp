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

#include "hardware.h"
#include "network.h"
#include "networkconst.h"


#include "console.h"
#include "h3/showsystime.h"

#include "ntpclient.h"

#include "mdns.h"

#include "display.h"
#include "displayhandler.h"

#include "oscserver.h"
#include "oscserverparams.h"
#include "oscservermsgconst.h"

#include "dmxmonitor.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "flashcodeinstall.h"
#include "configstore.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"


void Hardware::RebootHandler() {
}

void main() {
	Hardware hw;
	Display display;
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	MDNS mDns;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, Display7SegmentMessage::INFO_NONE, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	console_clear();

	fw.Print();

	console_puts("OSC ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
	console_set_top_row(2);

	nw.Print();
	
	ShowSystime showSystime;

	display.TextStatus(OscServerMsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	OSCServerParams params;
	OscServer server;

	params.Load();
	params.Set(&server);

	mDns.ServiceRecordAdd(nullptr, mdns::Services::OSC, "type=monitor", server.GetPortIncoming());

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	DMXMonitor monitor;
	// There is support for HEX output only
	server.SetOutput(&monitor);
	monitor.Cls();
	console_set_top_row(20);
	console_clear_top_row();

	server.Print();

	uint8_t nHwTextLength;

	display.Printf(1, "OSC Monitor");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	RemoteConfig remoteConfig(remoteconfig::Node::OSC, remoteconfig::Output::MONITOR, 1);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	mDns.Print();

	display.TextStatus(OscServerMsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	server.Start();

	display.TextStatus(OscServerMsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
		remoteConfig.Run();
		configStore.Flash();
		ntpClient.Run();
		showSystime.Run();
		mDns.Run();
		display.Run();
		hw.Run();
	}
}
