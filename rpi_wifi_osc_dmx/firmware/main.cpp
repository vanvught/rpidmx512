/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"
#include "networkbaremetal.h"
#include "ledblinkbaremetal.h"

#include "console.h"

#include "wifi.h"

#include "oscserverparms.h"
#include "oscserver.h"

// DMX output
#include "dmxparams.h"
#include "dmxsend.h"
// DMX real-time monitor
#include "dmxmonitor.h"

#include "software_version.h"

extern "C" {
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

void notmain(void) {
	HardwareBaremetal hw;
	NetworkBaremetal nw;
	LedBlinkBaremetal lb;
	struct ip_info ip_config;
	OSCServerParams oscparms;
	OscServer server;
	DMXParams dmxparams;
	DMXSend dmx;
	DMXMonitor monitor;
	uint8_t nHwTextLength;

	if (oscparms.Load()) {
		oscparms.Dump();
		oscparms.Set(&server);
	}

	const TOutputType tOutputType = oscparms.GetOutputType();

	if (tOutputType == OUTPUT_TYPE_MONITOR) {
		//
	} else if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi OSC Server ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);

	hw.SetLed(HARDWARE_LED_ON);

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	nw.Init();

	if (tOutputType == OUTPUT_TYPE_MONITOR) {
		server.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	} else {
		server.SetOutput(&dmx);
	}

	nw.Print();
	server.Print();

	if (tOutputType == OUTPUT_TYPE_MONITOR) {
		printf(" Server ip-address    : " IPSTR "\n\n\n", IP2STR(nw.GetIp()));
	} else {
		dmx.Print();
		console_newline();
	}

	console_status(CONSOLE_CYAN, "Starting the server ...");
	server.Start();
	console_status(CONSOLE_GREEN, "Server is running");

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		(void) server.Run();
		lb.Run();
	}
}

}
