/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "console.h"
#include "display.h"

#include "e131bridge.h"
#include "e131params.h"

// DMX output
#include "dmx.h"
#include "dmxparams.h"
#include "dmxsend.h"
#ifndef H3
 // Monitor Output
 #include "dmxmonitor.h"
#endif
// Pixel Controller
#include "ws28xxdmx.h"
#include "pixeldmxconfiguration.h"
#include "pixeltype.h"
#include "lightset.h"
#include "pixeldmxparams.h"

#if defined(ORANGE_PI)
# include "flashcodeinstall.h"
# include "configstore.h"
#endif

#include "software_version.h"

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char BRIDGE_PARMAS[] = "Setting Bridge parameters ...";
constexpr char START_BRIDGE[] = "Starting the Bridge ...";
constexpr char BRIDGE_STARTED[] = "Bridge started";

namespace e131bridge {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace e131bridge

void main() {
	Hardware hw;
	Network nw;
	Display display;
#if defined (ORANGE_PI)
	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;
#endif

	E131Bridge bridge;

	E131Params e131params;
	e131params.Load();

	const auto tOutputType = e131params.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi sACN E1.31 ");
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
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif

#ifndef H3
	DMXMonitor monitor;

	console_set_top_row(3);
#endif

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Init();

	console_status(CONSOLE_YELLOW, BRIDGE_PARMAS);
	display.TextStatus(BRIDGE_PARMAS);

	e131params.Set();

	bool IsSet;
	const auto nStartUniverse = e131params.GetUniverse(0, IsSet);

	bridge.SetUniverse(0, lightset::PortDir::OUTPUT, nStartUniverse);

	Dmx	dmx;
	DmxSend dmxSend;
	LightSet *pSpi = nullptr;

	if (tOutputType == lightset::OutputType::SPI) {
		PixelDmxConfiguration pixelDmxConfiguration;

		PixelDmxParams pixelDmxParams;
		pixelDmxParams.Load();
		pixelDmxParams.Set(&pixelDmxConfiguration);

		auto *pWS28xxDmx = new WS28xxDmx(pixelDmxConfiguration);
		assert(pWS28xxDmx != nullptr);
		pSpi = pWS28xxDmx;

		display.Printf(7, "%s:%d G%d", PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

		const auto nUniverses = pWS28xxDmx->GetUniverses();

		for (uint32_t nPortIndex = 1; nPortIndex < nUniverses; nPortIndex++) {
			bridge.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nStartUniverse + nPortIndex));
		}

		bridge.SetOutput(pSpi);
	}
#ifndef H3
	else if (tOutputType == lightset::OutputType::MONITOR) {
		// There is support for HEX output only
		bridge.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {
		DmxParams dmxparams;
		dmxparams.Load();
		dmxparams.Set(&dmx);

		bridge.SetOutput(&dmxSend);
	}

	bridge.Print();

	if (tOutputType == lightset::OutputType::SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else if (tOutputType == lightset::OutputType::MONITOR) {
		// Nothing to do
	} else {
		dmxSend.Print();
	}

	for (uint32_t i = 0; i < 7 ; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "WiFi sACN E1.31 ");

	switch (tOutputType) {
	case lightset::OutputType::SPI:
		display.PutString("Pixel");
		break;
#ifndef H3
	case lightset::OutputType::MONITOR:
		display.PutString("Monitor");
		break;
#endif
	default:
		display.PutString("DMX");
		break;
	}

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

	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "U: %d", nStartUniverse);
	display.Printf(6, "Active ports: %d", bridge.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, START_BRIDGE);
	display.TextStatus(START_BRIDGE);

	bridge.Start();

	console_status(CONSOLE_GREEN, BRIDGE_STARTED);
	display.TextStatus(BRIDGE_STARTED);

#if defined (ORANGE_PI)
	while (configStore.Flash())
		;
#endif

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		bridge.Run();
		hw.Run();
	}
}
