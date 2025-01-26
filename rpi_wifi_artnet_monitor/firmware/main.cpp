/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "hardware.h"
#include "network.h"

#include "console.h"
#include "display.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "dmxmonitor.h"
#include "timecode.h"

#if defined(ORANGE_PI)
# include "flashcodeinstall.h"
# include "configstore.h"
#endif

#include "software_version.h"

using namespace artnet;

constexpr char NETWORK_INIT[] = "Network init ...";
constexpr char NODE_PARMAS[] = "Setting Node parameters ...";
constexpr char RUN_RDM[] = "Running RDM Discovery ...";
constexpr char START_NODE[] = "Starting the Node ...";
constexpr char NODE_STARTED[] = "Node started";

namespace artnetnode {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace artnetnode

int main() {
	Hardware hw;
	Network nw;
	Display display;
#if defined (ORANGE_PI)
	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;
#endif

	ArtNetNode node;
	
	ArtNetParams artnetParams;
	artnetParams.Load();

	const auto outputType = artnetParams.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("WiFi Art-Net 3 Node ");
	console_set_fg_color(outputType == lightset::OutputType::DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetParams.IsRdm() && (outputType == lightset::OutputType::DMX)) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
#ifndef H3
	console_puts(" / ");
	console_set_fg_color(outputType == lightset::OutputType::MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(outputType == lightset::OutputType::SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
#ifdef H3
	console_putc('\n');
#endif
#ifndef H3
	console_set_top_row(3);
#endif

	console_status(CONSOLE_YELLOW, NETWORK_INIT);
	display.TextStatus(NETWORK_INIT);

	nw.Init();

#ifndef H3
	DMXMonitor monitor;
	TimeCode timecode;
#endif

	ArtNetRdmController artNetRdmController;

	console_status(CONSOLE_YELLOW, NODE_PARMAS);
	display.TextStatus(NODE_PARMAS);

	artnetParams.Set();

#ifndef H3
	if (outputType == lightset::OutputType::MONITOR) {
		timecode.Start();
		node.SetArtTimeCodeCallbackFunction(TimeCode::staticCallbackFunction);
	}
#endif

	const auto nStartUniverse = artnetParams.GetUniverse(0);

	node.SetUniverse(0, lightset::PortDir::OUTPUT, nStartUniverse);

	Dmx	dmx;
	DmxSend dmxSend;
	LightSet *pSpi = nullptr;

	if (outputType == lightset::OutputType::SPI) {
		PixelDmxConfiguration pixelDmxConfiguration;

		PixelDmxParams pixelDmxParams;
		pixelDmxParams.Load();
		pixelDmxParams.Set(&pixelDmxConfiguration);

		auto *pWS28xxDmx = new WS28xxDmx(&pixelDmxConfiguration);
		assert(pWS28xxDmx != nullptr);
		pSpi = pWS28xxDmx;

		display.Printf(7, "%s:%d G%d", pixel::pixel_get_type(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

		const auto nUniverses = pWS28xxDmx->GetUniverses();

		for (uint32_t nPortIndex = 1; nPortIndex < nUniverses; nPortIndex++) {
			node.SetUniverse(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint8_t>(nStartUniverse + nPortIndex));
		}

		node.SetOutput(pSpi);
	}
#ifndef H3
	else if (outputType == lightset::OutputType::MONITOR) {
		// There is support for HEX output only
		node.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {

		DmxParams dmxparams;

		dmxparams.Load();
		dmxparams.Set(&dmx);

		node.SetOutput(&dmxSend);

		if (artnetParams.IsRdm()) {
			RDMDeviceParams rdmDeviceParams;
			rdmDeviceParams.Load();
			rdmDeviceParams.Set(&artNetRdmController);

			artNetRdmController.Init();
			artNetRdmController.Print();

			node.SetRdmController(&artNetRdmController);
		}
	}

	node.Print();

	if (outputType == lightset::OutputType::SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else if (outputType == lightset::OutputType::MONITOR) {
		// Nothing
	} else {
		dmxSend.Print();
	}

	for (uint32_t i = 0; i < 7; i++) {
		display.ClearLine(i);
	}

	display.Write(1, "WiFi Art-Net 3 ");

	switch (outputType) {
	case lightset::OutputType::SPI:
		display.PutString("Pixel");
		break;
	case lightset::OutputType::MONITOR:
		display.PutString("Monitor");
		break;
	default:
		if (artnetParams.IsRdm()) {
			display.PutString("RDM");
		} else {
			display.PutString("DMX");
		}
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
	display.Printf(6, "Active ports: %d", node.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, START_NODE);
	display.TextStatus(START_NODE);

	node.Start();

	console_status(CONSOLE_GREEN, NODE_STARTED);
	display.TextStatus(NODE_STARTED);

	hw.WatchdogFeed();

	for (;;) {
		hw.WatchdogFeed();
		node.Run();
		hw.Run();
	}
}
