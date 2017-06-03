/**
 * @file main.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "hardware_uuid.h"

#include "led.h"

#include "console.h"
#include "oled.h"

#include "inet.h"
#include "uuid.h"

#include "e131.h"
#include "e131bridge.h"
#include "e131params.h"

#include "dmxsend.h"
#include "dmxparams.h"
#include "dmxmonitor.h"

#include "wifi.h"
#include "wifi_udp.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	_output_type output_type = OUTPUT_TYPE_MONITOR;
	E131Params e131params;
	DMXParams dmxparams;
	uuid_t uuid;
	char uuid_str[UUID_STRING_LENGTH + 1] ALIGNED;
	oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };
	bool oled_connected = false;
	struct ip_info ip_config;

	hardware_init();

	oled_connected = oled_start(&oled_info);

	(void) e131params.Load();

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		hardware_uuid(uuid);
		uuid_unparse(uuid, uuid_str);
	}

	output_type = e131params.GetOutputType();

	if (output_type == OUTPUT_TYPE_MONITOR) {
		//
	} else {
		output_type = OUTPUT_TYPE_DMX;
		(void) dmxparams.Load();
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("WiFi sACN E1.31 DMX Out / Real-time DMX Monitor");

	OLED_CONNECTED(oled_connected, oled_puts(&oled_info, "WiFi sACN E1.31"));

	console_set_top_row(3);

	if (!wifi(&ip_config)) {
		for (;;)
			;
	}

	console_status(CONSOLE_YELLOW, "Starting UDP ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Starting UDP ..."));

	wifi_udp_begin(E131_DEFAULT_PORT);

	console_status(CONSOLE_YELLOW, "Join group ...");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Join group ..."));

	uint32_t group_ip;
	(void)inet_aton("239.255.0.0", &group_ip);
	const uint16_t universe = e131params.GetUniverse();
	group_ip = group_ip | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF) << 24)) | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF00) << 8));
	wifi_udp_joingroup(group_ip);

	E131Bridge bridge;
	DMXSend dmx;
	DMXMonitor monitor;

	bridge.setCid(uuid);
	bridge.setUniverse(universe);
	bridge.setMergeMode(e131params.GetMergeMode());

	if (output_type == OUTPUT_TYPE_MONITOR) {
		bridge.SetOutput(&monitor);
		console_set_top_row(20);
	} else {
		bridge.SetOutput(&dmx);

		dmx.SetBreakTime(dmxparams.GetBreakTime());
		dmx.SetMabTime(dmxparams.GetMabTime());

		const uint8_t refresh_rate = dmxparams.GetRefreshRate();
		uint32_t period = (uint32_t) 0;

		if (refresh_rate != (uint8_t) 0) {
			period = (uint32_t) (1E6 / refresh_rate);
		}
		dmx.SetPeriodTime(period);
	}

	printf("\nBridge configuration\n");
	const uint8_t *firmware_version = bridge.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" CID          : %s\n", uuid_str);
	printf(" Universe     : %d\n", bridge.getUniverse());
	printf(" Merge mode   : %s\n", bridge.getMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
	printf(" Multicast ip : " IPSTR "\n", IP2STR(group_ip));
	printf(" Unicast ip   : " IPSTR "\n\n", IP2STR(ip_config.ip.addr));

	if (output_type == OUTPUT_TYPE_DMX) {
		printf("DMX Send parameters\n");
		printf(" Break time   : %d\n", (int) dmx.GetBreakTime());
		printf(" MAB time     : %d\n", (int) dmx.GetMabTime());
		printf(" Refresh rate : %d\n", (int) (1E6 / dmx.GetPeriodTime()));
	}

	if (oled_connected) {
		oled_set_cursor(&oled_info, 0, 18);

		switch (output_type) {
		case OUTPUT_TYPE_DMX:
			oled_puts(&oled_info, "DMX");
			break;
		case OUTPUT_TYPE_MONITOR:
			oled_puts(&oled_info, "Mon");
			break;
		default:
			break;
		}

		oled_set_cursor(&oled_info, 1, 0);
		if (wifi_get_opmode() == WIFI_STA) {
			(void) oled_printf(&oled_info, "S: %s", wifi_get_ssid());
		} else {
			(void) oled_printf(&oled_info, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		oled_set_cursor(&oled_info, 2, 0);
		(void) oled_puts(&oled_info, "CID: ");
		(void) oled_puts(&oled_info, uuid_str);
		oled_set_cursor(&oled_info, 4, 0);
		(void) oled_printf(&oled_info, "U: %d M: %s", bridge.getUniverse(), bridge.getMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
		oled_set_cursor(&oled_info, 5, 0);
		(void) oled_printf(&oled_info, "M: " IPSTR "", IP2STR(group_ip));
		oled_set_cursor(&oled_info, 6, 0);
		(void) oled_printf(&oled_info, "U: " IPSTR "", IP2STR(ip_config.ip.addr));
	}

	hardware_watchdog_init();

	console_status(CONSOLE_GREEN, "Bridge is running");
	OLED_CONNECTED(oled_connected, oled_status(&oled_info, "Bridge is running"));

	for (;;) {
		hardware_watchdog_feed();
		(void) bridge.Run();
		led_blink();
	}
}

}
