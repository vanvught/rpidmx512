/**
 * @file main.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <netinet/in.h>
#include <uuid/uuid.h>

#include "hardware.h"
#include "console.h"
#include "display.h"

#include "ledblinktask.h"

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

#include "dmxsender.h"
#include "dmxparams.h"
#include "dmxmonitor.h"

#include "wifi.h"
#include "network.h"

#include "util.h"

#include "software_version.h"

extern "C" {
extern void network_init(void);
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

void notmain(void) {
	E131Params e131params;
	E131Uuid e131uuid;
	DMXParams dmxparams;
	DMXSender dmx;
	DMXMonitor monitor;
	Display display(0,8);
	LedBlinkTask ledblinktask;
	uuid_t uuid;
	char uuid_str[UUID_STRING_LENGTH + 1];
	struct ip_info ip_config;

	(void) e131params.Load();

	TOutputType tOutputType = e131params.GetOutputType();

	if (tOutputType == OUTPUT_TYPE_MONITOR) {
		//
	} else {
		tOutputType = OUTPUT_TYPE_DMX;
		(void) dmxparams.Load();
	}

	const bool IsOledConnected = display.isDetected();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);

	console_puts("WiFi sACN E1.31 ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	console_status(CONSOLE_YELLOW, "Network init ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Network init ..."));

	network_init();

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		e131uuid.GetHardwareUuid(uuid);
		uuid_unparse(uuid, uuid_str);
	}

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Starting UDP ..."));

	network_begin(E131_DEFAULT_PORT);

	console_status(CONSOLE_YELLOW, "Join group ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Join group ..."));

	struct in_addr group_ip;
	(void)inet_aton("239.255.0.0", &group_ip);
	const uint16_t universe = e131params.GetUniverse();
	group_ip.s_addr = group_ip.s_addr | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF) << 24)) | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF00) << 8));
	network_joingroup(group_ip.s_addr);

	E131Bridge bridge;

	bridge.setCid(uuid);
	bridge.setUniverse(universe);
	bridge.setMergeMode(e131params.GetMergeMode());

	if (tOutputType == OUTPUT_TYPE_MONITOR) {
		bridge.SetOutput(&monitor);
		console_set_top_row(20);
	} else {
		dmxparams.Set(&dmx);
		bridge.SetOutput(&dmx);
	}

	printf("\nBridge configuration\n");
	const uint8_t *firmware_version = bridge.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" CID          : %s\n", uuid_str);
	printf(" Universe     : %d\n", bridge.getUniverse());
	printf(" Merge mode   : %s\n", bridge.getMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
	printf(" Multicast ip : " IPSTR "\n", IP2STR(group_ip.s_addr));
	printf(" Unicast ip   : " IPSTR "\n\n", IP2STR(ip_config.ip.addr));

	if (tOutputType == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1E6 / dmx.GetPeriodTime()));
	}

	if (IsOledConnected) {
		display.Write(1, "WiFi sACN E1.31 ");

		switch (tOutputType) {
		case OUTPUT_TYPE_DMX:
			display.PutString("DMX");
			break;
		case OUTPUT_TYPE_MONITOR:
			display.PutString("Mon");
			break;
		default:
			break;
		}

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		(void) display.Printf(3, "CID: ");
		(void) display.PutString(uuid_str);
		(void) display.Printf(5, "U: %d M: %s", bridge.getUniverse(), bridge.getMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
		(void) display.Printf(6, "M: " IPSTR "", IP2STR(group_ip.s_addr));
		(void) display.Printf(7, "U: " IPSTR "", IP2STR(ip_config.ip.addr));
	}

	console_status(CONSOLE_GREEN, "Bridge is running");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Bridge is running"));

	hardware_watchdog_init();

	for (;;) {
		hardware_watchdog_feed();

		(void) bridge.Run();

		ledblinktask.Run();
	}
}

}
