/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "led.h"

#include "console.h"
#include "display.h"

#include "oscparams.h"
#include "oscws28xx.h"

#include "deviceparams.h"

#include "wifi.h"
#include "network.h"

#include "software_version.h"

extern "C" {
extern void network_init(void);
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

void notmain(void) {
	struct ip_info ip_config;
	DeviceParams deviceparms;
	OSCParams oscparms;
	uint16_t incoming_port;
	uint16_t outgoing_port;
	Display display;
	bool oled_connected = false;

	oled_connected = display.isDetected();

	(void) oscparms.Load();
	(void) deviceparms.Load();

	incoming_port = oscparms.GetIncomingPort();
	outgoing_port = oscparms.GetOutgoingPort();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("WiFi OSC Pixel controller, Incoming port: %d, Outgoing port: %d", incoming_port, outgoing_port);

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	network_init();

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Starting UDP ..."));

	network_begin(incoming_port);

	network_sendto((const uint8_t *)"osc", (const uint16_t) 3, ip_config.ip.addr | ~ip_config.netmask.addr, outgoing_port);

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Setting Node parameters ..."));

	printf(" Type  : %s\n", deviceparms.GetLedTypeString());
	printf(" Count : %d", (int) deviceparms.GetLedCount());

	if (oled_connected) {
		display.Write(1, "WiFi OSC Pixel");

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		(void) display.Printf(3, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		(void) display.Printf(4, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		(void) display.Printf(5, "I: %4d O: %4d", (int) incoming_port, (int) outgoing_port);
		(void) display.Printf(6, "Led type: %s", deviceparms.GetLedTypeString());
		(void) display.Printf(7, "Led count: %d", (int) deviceparms.GetLedCount());
	}

	console_set_top_row(16);

	hardware_watchdog_init();

	OSCWS28xx oscws28xx(outgoing_port, deviceparms.GetLedCount(), deviceparms.GetLedType(), deviceparms.GetLedTypeString());

	console_status(CONSOLE_GREEN, "Starting ...");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Starting ..."));

	oscws28xx.Start();

	console_status(CONSOLE_GREEN, "Controller started");
	DISPLAY_CONNECTED(oled_connected, display.TextStatus("Controller started"));

	for (;;) {
		hardware_watchdog_feed();
		oscws28xx.Run();
		led_blink();
	}
}

}
