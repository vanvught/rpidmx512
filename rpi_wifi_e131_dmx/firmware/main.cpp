/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "console.h"
#include "display.h"

#include "e131bridge.h"
#include "e131params.h"

#include "dmx.h"
#include "dmxparams.h"
#include "dmxsend.h"

#if defined(ORANGE_PI)
# include "flashcodeinstall.h"
# include "configstore.h"
#endif

#include "software_version.h"

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
constexpr char START_BRIDGE[] = "Starting the Bridge ...";
constexpr char BRIDGE_STARTED[] = "Bridge started";

namespace e131bridge {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace e131bridge

int main() {
	Hardware hw;
	Network nw;
	Display display;
#if defined (ORANGE_PI)
	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;
#endif

	E131Bridge bridge;

	E131Params e131params;
	e131params.Load();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi sACN E1.31 ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("DMX Output");

#ifdef H3
	console_putc('\n');
#endif

#ifndef H3
	DMXMonitor monitor;

	console_set_top_row(3);
#endif

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Init();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	e131params.Set();

	bool IsSet;
	const auto nStartUniverse = e131params.GetUniverse(0, IsSet);

	bridge.SetUniverse(0, lightset::PortDir::OUTPUT, nStartUniverse);

	Dmx	dmx;
	DmxSend dmxSend;

	DmxParams dmxparams;
	dmxparams.Load();
	dmxparams.Set(&dmx);

	bridge.SetOutput(&dmxSend);

	bridge.Print();
	dmxSend.Print();

	for (uint32_t i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "WiFi sACN E1.31 DMX");

	if (nw.GetOpmode() == WIFI_STA) {
		display.Printf(2, "S: %s", nw.GetSsid());
	} else {
		display.Printf(2, "AP (%s)\n", nw.IsApOpen() ? "Open" : "WPA_WPA2_PSK");
	}

	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));

	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}

	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "U: %d", nStartUniverse);
	display.Printf(6, "Active ports: %d", bridge.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	bridge.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		bridge.Run();
		hw.Run();
	}
}
