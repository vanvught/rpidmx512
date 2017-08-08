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

#include "oscparams.h"
#include "oscws28xx.h"

#include "deviceparams.h"

#include "oled.h"

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
	oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };
	bool oled_connected = false;

	oled_connected = oled_start(&oled_info);

	(void) oscparms.Load();
	(void) deviceparms.Load();

	incoming_port = oscparms.GetIncomingPort();
	outgoing_port = oscparms.GetOutgoingPort();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("WiFi OSC Pixel controller, Incoming port: %d, Outgoing port: %d", incoming_port, outgoing_port);

	OLED_CONNECTED(oled_connected, oled_puts(&oled_info, "WiFi OSC Pixel"));

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	network_init();

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting UDP ..."));

	network_begin(incoming_port);

	network_sendto((const uint8_t *)"osc", (const uint16_t) 3, ip_config.ip.addr | ~ip_config.netmask.addr, outgoing_port);

	console_status(CONSOLE_YELLOW, "Setting Node parameters ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Setting Node parameters ..."));

	printf(" Type  : %s\n", deviceparms.GetLedTypeString());
	printf(" Count : %d", (int) deviceparms.GetLedCount());

	if (oled_connected) {
		oled_set_cursor(&oled_info, 1, 0);
		if (wifi_get_opmode() == WIFI_STA) {
			(void) oled_printf(&oled_info, "S: %s", wifi_get_ssid());
		} else {
			(void) oled_printf(&oled_info, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		oled_set_cursor(&oled_info, 2, 0);
		(void) oled_printf(&oled_info, "IP: " IPSTR "", IP2STR(ip_config.ip.addr));
		oled_set_cursor(&oled_info, 3, 0);
		(void) oled_printf(&oled_info, "N: " IPSTR "", IP2STR(ip_config.netmask.addr));
		oled_set_cursor(&oled_info, 4, 0);
		(void) oled_printf(&oled_info, "I: %4d O: %4d", (int) incoming_port, (int) outgoing_port);
		oled_set_cursor(&oled_info, 5, 0);
		(void) oled_printf(&oled_info, "Led type: %s", deviceparms.GetLedTypeString());
		oled_set_cursor(&oled_info, 6, 0);
		(void) oled_printf(&oled_info, "Led count: %d", (int) deviceparms.GetLedCount());
	}

	console_set_top_row(16);

	hardware_watchdog_init();

	OSCWS28xx oscws28xx(outgoing_port, deviceparms.GetLedCount(), deviceparms.GetLedType(), deviceparms.GetLedTypeString());

	console_status(CONSOLE_GREEN, "Starting ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting ..."));

	oscws28xx.Start();

	console_status(CONSOLE_GREEN, "Controller started");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Controller started"));

	for (;;) {
		hardware_watchdog_feed();
		oscws28xx.Run();
		led_blink();
	}
}

}
