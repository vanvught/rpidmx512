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

#include "networkconst.h"
#include "artnetconst.h"

#include "artnet4node.h"
#include "artnet4params.h"

#include "ipprog.h"

// DMX Out, RDM Controller
#include "dmxparams.h"
#include "dmxsend.h"
#include "artnetdiscovery.h"
// Pixel Controller
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxgrouping.h"
// PWM Led
#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#if defined(ORANGE_PI)
 #include "spiflashinstall.h"
 #include "spiflashstore.h"
#endif

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	Display display(DISPLAY_SSD1306);

#if defined (ORANGE_PI)
	if (hw.GetBootDevice() == BOOT_DEVICE_MMC0) {
		SpiFlashInstall spiFlashInstall;
	}

	SpiFlashStore spiFlashStore;
	ArtNet4Params artnetparams((ArtNet4ParamsStore *)spiFlashStore.GetStoreArtNet4());
#else
	ArtNet4Params artnetparams;
#endif

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	const TLightSetOutputType tOutputType = artnetparams.GetOutputType();

	uint8_t nHwTextLength;
	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetparams.IsRdm() && (tOutputType == LIGHTSET_OUTPUT_TYPE_DMX)) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT);

#if defined (ORANGE_PI)
	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
#else
	nw.Init();
#endif
	nw.Print();

	ArtNet4Node node;
	ArtNetRdmController discovery;

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
	display.TextStatus(ArtNetConst::MSG_NODE_PARAMS);

	artnetparams.Set(&node);

	IpProg ipprog;

	node.SetIpProgHandler(&ipprog);

#if defined (ORANGE_PI)
	node.SetArtNetStore((ArtNetStore *)spiFlashStore.GetStoreArtNet());
#endif

	const uint8_t nUniverse = artnetparams.GetUniverse();

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, nUniverse);
	node.SetDirectUpdate(false);

	DMXSend dmx;
	LightSet *pSpi;

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		bool isLedTypeSet = false;

#if defined (ORANGE_PI)
		TLC59711DmxParams pwmledparms((TLC59711DmxParamsStore *) spiFlashStore.GetStoreTLC59711());
#else
		TLC59711DmxParams pwmledparms;
#endif

		if (pwmledparms.Load()) {
			if ((isLedTypeSet = pwmledparms.IsSetLedType()) == true) {
				TLC59711Dmx *pTLC59711Dmx = new TLC59711Dmx;
				assert(pTLC59711Dmx != 0);
				pwmledparms.Dump();
				pwmledparms.Set(pTLC59711Dmx);
				pSpi = pTLC59711Dmx;

				display.Printf(7, "%s:%d", pwmledparms.GetLedTypeString(pwmledparms.GetLedType()), pwmledparms.GetLedCount());
			}
		}

		if (!isLedTypeSet) {
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
						node.SetDirectUpdate(true);
						node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
					}
					if (nLedCount > 256) {
						node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
					}
					if (nLedCount > 384) {
						node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
					}
				} else {
					if (nLedCount > 170) {
						node.SetDirectUpdate(true);
						node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, nUniverse + 1);
					}
					if (nLedCount > 340) {
						node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, nUniverse + 2);
					}
					if (nLedCount > 510) {
						node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, nUniverse + 3);
					}
				}
			}
		}

		node.SetOutput(pSpi);
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

		node.SetOutput(&dmx);

		if(artnetparams.IsRdm()) {
			if (artnetparams.IsRdmDiscovery()) {
				console_status(CONSOLE_YELLOW, ArtNetConst::MSG_RDM_RUN);
				display.TextStatus(ArtNetConst::MSG_RDM_RUN);
				discovery.Full();
			}
			node.SetRdmHandler(&discovery);
		}
	}

	node.Print();

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else {
		dmx.Print();
	}

	for (unsigned i = 0; i < 7; i++) {
		display.ClearLine(i);
	}
	
	display.Write(1, "Eth Art-Net 4 ");

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		display.PutString("Pixel");
	} else {
		if (artnetparams.IsRdm()) {
			display.PutString("RDM");
		} else {
			display.PutString("DMX");
		}
	}

	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR "", IP2STR(Network::Get()->GetIp()));
	
	if (nw.IsDhcpKnown()) {
		if (nw.IsDhcpUsed()) {
			display.PutString(" D");
		} else {
			display.PutString(" S");
		}
	}
	
	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "N: %d SubN: %d U: %d", node.GetNetSwitch(), node.GetSubnetSwitch(), nUniverse);
	display.Printf(6, "Active ports: %d", node.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
	display.TextStatus(ArtNetConst::MSG_NODE_START);

	node.Start();

	console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
	display.TextStatus(ArtNetConst::MSG_NODE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.HandlePacket();
		lb.Run();
#if defined (ORANGE_PI)
		spiFlashStore.Flash();
#endif
	}
}

}
