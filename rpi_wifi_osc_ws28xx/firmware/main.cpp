/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "display.h"

#include "wifi.h"

#include "oscparams.h"
#include "oscws28xx.h"

#include "ws28xxstripeparams.h"

#include "software_version.h"

extern "C" {
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

void notmain(void) {
	HardwareBaremetal hw;
	NetworkBaremetal nw;
	LedBlinkBaremetal lb;
	WS28XXStripeParams deviceparms;
	OSCParams oscparms;
	Display display(0,8);
	struct ip_info ip_config;
	uint8_t nHwTextLength;

	(void) oscparms.Load();
	(void) deviceparms.Load();

	const uint16_t nIncomingPort = oscparms.GetIncomingPort();
	const uint16_t nOutgoingPort = oscparms.GetOutgoingPort();

	const bool IsOledConnected = display.isDetected();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);
	printf("WiFi OSC Pixel controller, Incoming port: %d, Outgoing port: %d", nIncomingPort, nOutgoingPort);

	hw.SetLed(HARDWARE_LED_ON);

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	nw.Init();

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Starting UDP ..."));

	nw.Begin(nIncomingPort);

	nw.SendTo((const uint8_t *)"osc", (const uint16_t) 3, ip_config.ip.addr | ~ip_config.netmask.addr, nOutgoingPort);

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Setting Node parameters ..."));

	const TWS28XXType tType = deviceparms.GetLedType();

	printf("Type  : %s [%d]\n", WS28XXStripeParams::GetLedTypeString(tType), tType);
	printf("Count : %d", (int) deviceparms.GetLedCount());

	if (IsOledConnected) {
		display.Write(1, "WiFi OSC Pixel");

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		(void) display.Printf(3, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		(void) display.Printf(4, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		(void) display.Printf(5, "I: %4d O: %4d", (int) nIncomingPort, (int) nOutgoingPort);
		(void) display.Printf(6, "Led type: %s", WS28XXStripeParams::GetLedTypeString(tType));
		(void) display.Printf(7, "Led count: %d", (int) deviceparms.GetLedCount());
	}

	console_set_top_row(16);

	hw.WatchdogInit();

	OSCWS28xx oscws28xx(nOutgoingPort, deviceparms.GetLedCount(), deviceparms.GetLedType(), WS28XXStripeParams::GetLedTypeString(tType));

	console_status(CONSOLE_GREEN, "Starting ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Starting ..."));

	oscws28xx.Start();

	console_status(CONSOLE_GREEN, "Controller started");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Controller started"));

	for (;;) {
		hw.WatchdogFeed();
		oscws28xx.Run();
		lb.Run();
	}
}

}
