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

#include "h3/hal_watchdog.h"
#include "hal_boardinfo.h"
#include "emac/network.h"
#include "display.h"
#include "oscserver.h"
#include "json/oscserverparams.h"
#include "oscservermsgconst.h"
#include "json/dmxsendparams.h"
#include "dmxsend.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#include "firmwareversion.h"
#include "software_version.h"

namespace hal {
void RebootHandler() {
	Dmx::Get()->Blackout();
}
}  // namespace hal

int main() {
	hal::Init();
	Display display;
	ConfigStore config_store;
	network::Init();
	FirmwareVersion fw(kSoftwareVersion, __DATE__, __TIME__);
	FlashCodeInstall spiflash_install;

	fw.Print("OSC Server DMX");

	OscServer oscserver;

	json::OscServerParams oscserver_params;
	oscserver_params.Load();
	oscserver_params.Set();

	Dmx dmx;

	json::DmxSendParams dmxparams;
	dmxparams.Load();
	dmxparams.Set();

	DmxSend dmx_send;
	dmx_send.Print();

	oscserver.SetOutput(&dmx_send);
	oscserver.Print();

	RemoteConfig remote_config(remoteconfig::Output::DMX, 1);

	uint8_t text_length;

	display.Printf(1, "OSC DMX 1");
	display.Write(2, hal::BoardName(text_length));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(network::GetPrimaryIp()), network::iface::IsDhcpKnown() ? ( network::iface::Dhcp() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", oscserver.GetPortIncoming());
	display.Printf(5, "Out: %d", oscserver.GetPortOutgoing());

	display.TextStatus(OscServerMsgConst::START, console::Colours::kConsoleYellow);

	oscserver.Start();

	display.TextStatus(OscServerMsgConst::STARTED, console::Colours::kConsoleGreen);

	hal::WatchdogInit();

	for (;;) {
		hal::WatchdogFeed();
		network::Run();
		display.Run();
		hal::Run();
	}
}
