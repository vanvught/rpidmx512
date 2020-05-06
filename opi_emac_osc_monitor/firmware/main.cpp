/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "console.h"
#include "h3/showsystime.h"

#include "ntpclient.h"

#include "display.h"

#include "networkconst.h"

#include "mdns.h"
#include "mdnsservices.h"

#include "oscserver.h"
#include "oscserverparms.h"

// Monitor Output
#include "dmxmonitor.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

constexpr char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
constexpr char START_BRIDGE[] = "Starting the Bridge ...";
constexpr char BRIDGE_STARTED[] = "Bridge started";

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(DISPLAY_SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	ShowSystime showSystime;

	console_puts("Ethernet OSC ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	nw.Init();
	nw.Print();

	NtpClient ntpClient;
	ntpClient.Init();
	ntpClient.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_PARMAMS);

	OSCServerParams params;
	OscServer server;

	if (params.Load()) {
		params.Dump();
		params.Set(&server);
	}

	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(0, MDNS_SERVICE_OSC, server.GetPortIncoming(), "type=monitor");
	mDns.Print();

	DMXMonitor monitor;
	// There is support for HEX output only
	server.SetOutput(&monitor);
	monitor.Cls();
	console_set_top_row(20);

	server.Print();

	for (unsigned i = 1; i < 7 ; i++) {
		display.ClearLine(i);
	}

	uint8_t nHwTextLength;

	display.Printf(1, "OSC Monitor");
	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_START);

	server.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		server.Run();
		ntpClient.Run();
		lb.Run();
		showSystime.Run();
		display.Run();
	}
}

}
