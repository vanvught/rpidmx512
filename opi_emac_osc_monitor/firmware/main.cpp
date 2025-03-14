/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal.h"
#include "network.h"

#include "console.h"
#include "h3/showsystime.h"

#include "net/apps/mdns.h"

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

namespace hal {
void reboot_handler() {
}
}  // namespace hal

int main() {
	hal_init();
	Display display;
	ConfigStore configStore;
	Network nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	console_clear();

	fw.Print();

	console_puts("OSC ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
	console_set_top_row(2);

	ShowSystime showSystime;

	display.TextStatus(OscServerMsgConst::PARAMS, CONSOLE_YELLOW);

	OscServer oscServer;

	OSCServerParams oscServerParams;
	oscServerParams.Load();
	oscServerParams.Set();

	mdns_service_record_add(nullptr, mdns::Services::OSC, "type=monitor", oscServer.GetPortIncoming());

	DMXMonitor monitor;
	// There is support for HEX output only
	oscServer.SetOutput(&monitor);
	monitor.Cls();
	console_set_top_row(20);
	console_clear_top_row();

	oscServer.Print();

	uint8_t nHwTextLength;

	display.Printf(1, "OSC Monitor");
	display.Write(2, hal::board_name(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", oscServer.GetPortIncoming());
	display.Printf(5, "Out: %d", oscServer.GetPortOutgoing());

	RemoteConfig remoteConfig(remoteconfig::NodeType::OSC, remoteconfig::Output::MONITOR, 1);

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	display.TextStatus(OscServerMsgConst::START, CONSOLE_YELLOW);

	oscServer.Start();

	display.TextStatus(OscServerMsgConst::STARTED, CONSOLE_GREEN);

	hal::watchdog_init();

	for (;;) {
		hal::watchdog_feed();
		nw.Run();
		showSystime.Run();
		display.Run();
		hal::run();
	}
}
