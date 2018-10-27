/**
 * @file main.c
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
#include <netinet/in.h>
#include <uuid/uuid.h>

#include "hardwarebaremetal.h"
#include "networkh3emac.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

// DMX output
#include "dmxparams.h"
#include "dmxsend.h"
// WS28xx output
#include "ws28xxstripeparams.h"
#include "ws28xxstripedmx.h"

#include "util.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	E131Params e131params;
	E131Uuid e131uuid;
	uuid_t uuid;
	char uuid_str[UUID_STRING_LENGTH + 1];
	uint8_t nHwTextLength;

	if (e131params.Load()) {
		e131params.Dump();
	}

	const TOutputType tOutputType = e131params.GetOutputType();

	Display display(0,8);
	const bool IsOledConnected = display.isDetected();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {1 Universe}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);

	console_set_top_row(3);

	console_status(CONSOLE_YELLOW, "Network init ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Network init ..."));

	nw.Init();

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		e131uuid.GetHardwareUuid(uuid);
		uuid_unparse(uuid, uuid_str);
	}

	E131Bridge bridge;
	DMXSend dmx;
	SPISend spi;

	bridge.SetCid(uuid);
	bridge.SetUniverse(e131params.GetUniverse());
	bridge.SetMergeMode(e131params.GetMergeMode());

	if (tOutputType == OUTPUT_TYPE_DMX) {
		DMXParams dmxparams;
		if (dmxparams.Load()) {
			dmxparams.Dump();
		}
		dmxparams.Set(&dmx);
		bridge.SetOutput(&dmx);
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		WS28XXStripeParams deviceparms;
		if (deviceparms.Load()) {
			deviceparms.Dump();
		}
		deviceparms.Set(&spi);
		bridge.SetOutput(&spi);
	}

	bridge.Print();

	if (tOutputType == OUTPUT_TYPE_DMX) {
		dmx.Print();
	} else if (tOutputType == OUTPUT_TYPE_SPI) {
		spi.Print();
	}

	if (IsOledConnected) {
		display.Write(1, "Eth sACN E1.31 ");

		switch (tOutputType) {
		case OUTPUT_TYPE_DMX:
			display.PutString("DMX");
			break;
		case OUTPUT_TYPE_SPI:
			display.PutString("Pixel");
			break;
		default:
			display.PutString("-E-");
			break;
		}

		(void) display.Printf(2, "%s", hw.GetBoardName(nHwTextLength));
		(void) display.Printf(3, "CID: ");
		(void) display.PutString(uuid_str);
		(void) display.Printf(5, "U: %d M: %s", bridge.GetUniverse(), bridge.GetMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
		(void) display.Printf(6, "M: " IPSTR "", IP2STR(bridge.GetMulticastIp()));
		(void) display.Printf(7, "U: " IPSTR "", IP2STR(Network::Get()->GetIp()));
	}

	console_status(CONSOLE_YELLOW, "Starting the Bridge ...");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Starting the Bridge ..."));

	bridge.Start();

	console_status(CONSOLE_GREEN, "Bridge is running");
	DISPLAY_CONNECTED(IsOledConnected, display.TextStatus("Bridge is running"));

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		(void) bridge.Run();
		lb.Run();
	}
}

}
