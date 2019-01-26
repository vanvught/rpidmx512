/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "hardwarebaremetal.h"
#include "networkh3emac.h"
#include "ledblinkbaremetal.h"

#include "console.h"
#include "display.h"

#include "e131bridge.h"
#include "e131params.h"

// DMX Out
#include "dmxparams.h"
#include "dmxsend.h"
// Pixel controller
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxgrouping.h"

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
	NetworkH3emac nw;
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

	Display display(DISPLAY_SSD1306);

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(tOutputType == E131_OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == E131_OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

#if defined (ORANGE_PI)
	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
#else
	nw.Init();
#endif
	nw.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	E131Bridge bridge;
	e131params.Set(&bridge);

	const uint8_t nUniverse = e131params.GetUniverse();

	bridge.SetUniverse(0, E131_OUTPUT_PORT, nUniverse);
	bridge.SetDirectUpdate(false);

	DMXSend dmx;
	LightSet *pSpi;

	if (tOutputType == E131_OUTPUT_TYPE_SPI) {
#if defined (ORANGE_PI)
		WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) spiFlashStore.GetStoreWS28xxDmx());
#else
		WS28xxDmxParams ws28xxparms;
#endif
		if (ws28xxparms.Load()) {
			ws28xxparms.Dump();
		}

		display.Printf(7, "%s:%d %c", ws28xxparms.GetLedTypeString(ws28xxparms.GetLedType()), ws28xxparms.GetLedCount(), ws28xxparms.IsLedGrouping() ? 'G' : ' ');

		if (ws28xxparms.IsLedGrouping()) {
			WS28xxDmxGrouping *pWS28xxDmxGrouping = new WS28xxDmxGrouping;
			assert(pWS28xxDmxGrouping != 0);
			ws28xxparms.Set(pWS28xxDmxGrouping);
			pSpi = pWS28xxDmxGrouping;
		} else  {
			WS28xxDmx *pWS28xxDmx = new WS28xxDmx;
			assert(pWS28xxDmx != 0);
			ws28xxparms.Set(pWS28xxDmx);
			pSpi = pWS28xxDmx;

			const uint16_t nLedCount = pWS28xxDmx->GetLEDCount();

			if (pWS28xxDmx->GetLEDType() == SK6812W) {
				if (nLedCount > 128) {
					bridge.SetDirectUpdate(true);
					bridge.SetUniverse(1, E131_OUTPUT_PORT, nUniverse + 1);
				}
				if (nLedCount > 256) {
					bridge.SetUniverse(2, E131_OUTPUT_PORT, nUniverse + 2);
				}
				if (nLedCount > 384) {
					bridge.SetUniverse(3, E131_OUTPUT_PORT, nUniverse + 3);
				}
			} else {
				if (nLedCount > 170) {
					bridge.SetDirectUpdate(true);
					bridge.SetUniverse(1, E131_OUTPUT_PORT, nUniverse + 1);
				}
				if (nLedCount > 340) {
					bridge.SetUniverse(2, E131_OUTPUT_PORT, nUniverse + 2);
				}
				if (nLedCount > 510) {
					bridge.SetUniverse(3, E131_OUTPUT_PORT, nUniverse + 3);
				}
			}
		}

		bridge.SetOutput(pSpi);
	} else {
#if defined (ORANGE_PI)
		DMXParams dmxparams((DMXParamsStore *)spiFlashStore.GetStoreDmxSend());
#else
		DMXParams dmxparams;
#endif
		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(&dmx);
		}

		bridge.SetOutput(&dmx);
	}

	bridge.Print();

	if (tOutputType == E131_OUTPUT_TYPE_SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else {
		dmx.Print();
	}

	display.Printf(1, "Eth sACN E1.31 %s", tOutputType == E131_OUTPUT_TYPE_SPI ? "Pixel" : "DMX");
	display.Printf(2, "%s", hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));
	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}
	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "U: %d", nUniverse);
	display.Printf(6, "Active ports: %d", bridge.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	bridge.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

#if defined (ORANGE_PI)
	while (spiFlashStore.Flash())
		;
#endif

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		lb.Run();
	}
}

}
