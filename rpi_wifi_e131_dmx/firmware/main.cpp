/**
 * @file main.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <netinet/in.h>
#include <uuid/uuid.h>

#include "hardwarebaremetal.h"
#include "networkesp8266.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "wifi.h"

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

// DMX output
#include "dmxparams.h"
#include "dmxsend.h"
#ifndef H3
 // Monitor Output
 #include "dmxmonitor.h"
#endif
// WS28xx output
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"

#if defined(ORANGE_PI)
 #include "spiflashinstall.h"
 #include "spiflashstore.h"
#endif

#include "software_version.h"

static const char NETWORK_INIT[] = "Network init ...";
static const char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
static const char START_BRIDGE[] = "Starting the Bridge ...";
static const char BRIDGE_STARTED[] = "Bridge started";

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkESP8266 nw;
	LedBlinkBaremetal lb;

#if defined (ORANGE_PI)
	if (hw.GetBootDevice() == BOOT_DEVICE_MMC0) {
		SpiFlashInstall spiFlashInstall;
	}

	SpiFlashStore spiFlashStore;
	E131Params e131params((E131ParamsStore *)spiFlashStore.GetStoreE131());
#else
	E131Params e131params;
#endif

	if (e131params.Load()) {
		e131params.Dump();
	}

	const TE131OutputType tOutputType = e131params.GetOutputType();

	Display display(0,8);

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi sACN E1.31 ");
	console_set_fg_color(tOutputType == E131_OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(tOutputType == E131_OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(tOutputType == E131_OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {1 Universe}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif

	console_set_top_row(3);

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Init();

	E131Uuid e131uuid;
	uuid_t uuid;
	char uuid_str[UUID_STRING_LENGTH + 1];

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		e131uuid.GetHardwareUuid(uuid);
		uuid_unparse(uuid, uuid_str);
	}

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	E131Bridge bridge;
	DMXSend dmx;
	WS28xxDmx spi;
#ifndef H3
	DMXMonitor monitor;
#endif

	bridge.SetCid(uuid);
	bridge.SetUniverse(e131params.GetUniverse());
	bridge.SetMergeMode(e131params.GetMergeMode());

	if (tOutputType == E131_OUTPUT_TYPE_DMX) {
		DMXParams dmxparams;
		if (dmxparams.Load()) {
			dmxparams.Dump();
		}
		dmxparams.Set(&dmx);
		bridge.SetOutput(&dmx);
	} else if (tOutputType == E131_OUTPUT_TYPE_SPI) {
		WS28xxDmxParams deviceparms;
		if (deviceparms.Load()) {
			deviceparms.Dump();
		}
		deviceparms.Set(&spi);
		bridge.SetOutput(&spi);
	}
#ifndef H3
	else if (tOutputType == E131_OUTPUT_TYPE_MONITOR) {
		bridge.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif

	bridge.Print();

	if (tOutputType == E131_OUTPUT_TYPE_SPI) {
		spi.Print();
	} else {
		dmx.Print();
	}

	if (display.isDetected()) {
		(void) display.Printf(1, "WiFi sACN E1.31 %s", tOutputType == E131_OUTPUT_TYPE_SPI ? "Pixel" : "DMX");

		if (wifi_get_opmode() == WIFI_STA) {
			(void) display.Printf(2, "S: %s", wifi_get_ssid());
		} else {
			(void) display.Printf(2, "AP (%s)\n", wifi_ap_is_open() ? "Open" : "WPA_WPA2_PSK");
		}

		(void) display.Printf(3, "CID: ");
		(void) display.PutString(uuid_str);
		(void) display.Printf(5, "U: %d M: %s", bridge.GetUniverse(), bridge.GetMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
		(void) display.Printf(6, "M: " IPSTR "", IP2STR(bridge.GetMulticastIp()));
		(void) display.Printf(7, "U: " IPSTR "", IP2STR(Network::Get()->GetIp()));
	}

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	bridge.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		(void) bridge.Run();
		lb.Run();
	}
}

}
