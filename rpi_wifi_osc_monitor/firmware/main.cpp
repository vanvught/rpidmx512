/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>
#include <cstdint>
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "console.h"
#include "display.h"

#include "oscserverparams.h"
#include "oscserver.h"

// DMX output
#include "dmx.h"
#include "dmxparams.h"
#include "dmxsend.h"
#ifndef H3
// DMX real-time monitor
# include "dmxmonitor.h"
#endif
// Pixel Controller
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "lightset.h"
#include "pixeldmxparams.h"
#include "ws28xxdmx.h"

#include "handler.h"

#if defined(ORANGE_PI)
# include "flashcodeinstall.h"
# include "configstore.h"
#endif

#include "software_version.h"

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
constexpr char START_BRIDGE[] = "Starting the Bridge ...";
constexpr char BRIDGE_STARTED[] = "Bridge started";

int main() {
	Hardware hw;
	Network nw;
	Display display;

#ifndef H3
	DMXMonitor monitor;
#endif

#if defined (ORANGE_PI)
	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;
#endif
	OSCServerParams params;
	OscServer server;

	params.Load();
	params.Set(&server);

	const auto tOutputType = params.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi OSC Server ");
	console_set_fg_color(tOutputType == lightset::OutputType::DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(tOutputType == lightset::OutputType::MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(tOutputType == lightset::OutputType::SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {1 Universe}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif

#ifndef H3
	console_set_top_row(3);
#endif

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Print();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	Dmx dmx;
	DmxSend dmxSend;
	LightSet *pSpi = nullptr;

	if (tOutputType == lightset::OutputType::SPI) {
		PixelDmxConfiguration pixelDmxConfiguration;

		PixelDmxParams pixelDmxParams;
		pixelDmxParams.Load();
		pixelDmxParams.Set(&pixelDmxConfiguration);

		// For the time being, just 1 Universe
		if (pixelDmxConfiguration.GetType() == pixel::Type::SK6812W) {
			if (pixelDmxConfiguration.GetCount() > 128) {
				pixelDmxConfiguration.SetCount(128);
			}
		} else {
			if (pixelDmxConfiguration.GetCount() > 170) {
				pixelDmxConfiguration.SetCount(170);
			}
		}

		auto *pPixelDmx = new WS28xxDmx(&pixelDmxConfiguration);
		assert(pPixelDmx != nullptr);
		pSpi = pPixelDmx;

		display.Printf(7, "%s:%d G%d", pixel::pixel_get_type(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

		server.SetOutput(pSpi);
		server.SetOscServerHandler(new Handler(pPixelDmx));
	}
#ifndef H3
	else if (tOutputType == lightset::OutputType::MONITOR) {
		// There is support for HEX output only
		server.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {
		DmxParams dmxparams;
		dmxparams.Load();
		dmxparams.Set(&dmx);

		server.SetOutput(&dmxSend);
	}

	server.Print();

	if (tOutputType == lightset::OutputType::SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else 	if (tOutputType == lightset::OutputType::MONITOR) {
		printf(" Server ip-address    : " IPSTR "\n\n\n", IP2STR(nw.GetIp()));
	} else {
		dmxSend.Print();
	}

	for (uint32_t i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Printf(1, "WiFi OSC %s", tOutputType == lightset::OutputType::SPI ? "Pixel" : "DMX");

	if (nw.GetOpmode() == WIFI_STA) {
		display.Printf(2, "S: %s", nw.GetSsid());
	} else {
		display.Printf(2, "AP (%s)\n", nw.IsApOpen() ? "Open" : "WPA_WPA2_PSK");
	}

	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));

	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}

	display.Printf(4, "In: %d", server.GetPortIncoming());
	display.Printf(5, "Out: %d", server.GetPortOutgoing());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	server.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		hw.Run();
	}
}
