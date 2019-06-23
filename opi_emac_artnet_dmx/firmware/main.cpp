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
#if defined (ORANGE_PI_ONE)
 // Monitor Output
 #include "dmxmonitor.h"
#endif
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
 #include "remoteconfig.h"
 #include "remoteconfigparams.h"
 #include "storeremoteconfig.h"
#endif

#include "firmwareversion.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	HardwareBaremetal hw;
	NetworkH3emac nw;
	LedBlinkBaremetal lb;
	Display display(DISPLAY_SSD1306);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

#if defined (ORANGE_PI)
	SpiFlashInstall spiFlashInstall;

	SpiFlashStore spiFlashStore;
	ArtNet4Params artnetparams((ArtNet4ParamsStore *)spiFlashStore.GetStoreArtNet4());
#else
	ArtNet4Params artnetparams;
#endif

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	const TLightSetOutputType tOutputType = artnetparams.GetOutputType();

	fw.Print();

	console_puts("Ethernet Art-Net 4 Node ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_DMX ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("DMX Output");
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(" / ");
	console_set_fg_color((artnetparams.IsRdm() && (tOutputType == LIGHTSET_OUTPUT_TYPE_DMX)) ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("RDM");
	console_set_fg_color(CONSOLE_WHITE);
#if defined (ORANGE_PI_ONE)
	console_puts(" / ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Real-time DMX Monitor");
	console_set_fg_color(CONSOLE_WHITE);
#endif
	console_puts(" / ");
	console_set_fg_color(tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? CONSOLE_GREEN : CONSOLE_WHITE);
	console_puts("Pixel controller {4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

#if defined (ORANGE_PI_ONE)
	DMXMonitor monitor;
#endif

	hw.SetLed(HARDWARE_LED_ON);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

#if defined (ORANGE_PI)
	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
#else
	nw.Init();
#endif
	nw.Print();

	ArtNet4Node node;
	ArtNetRdmController discovery;

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
	display.TextStatus(ArtNetConst::MSG_NODE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS);

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

			const bool bIsLedGrouping = ws28xxparms.IsLedGrouping() && (ws28xxparms.GetLedGroupCount() > 1);

			if (bIsLedGrouping) {
				WS28xxDmxGrouping *pWS28xxDmxGrouping = new WS28xxDmxGrouping;
				assert(pWS28xxDmxGrouping != 0);
				ws28xxparms.Set(pWS28xxDmxGrouping);
				pWS28xxDmxGrouping->SetLEDGroupCount(ws28xxparms.GetLedGroupCount());
				pSpi = pWS28xxDmxGrouping;
				display.Printf(7, "%s:%d G%d", ws28xxparms.GetLedTypeString(pWS28xxDmxGrouping->GetLEDType()), pWS28xxDmxGrouping->GetLEDCount(), pWS28xxDmxGrouping->GetLEDGroupCount());
			} else  {
				WS28xxDmx *pWS28xxDmx = new WS28xxDmx;
				assert(pWS28xxDmx != 0);
				ws28xxparms.Set(pWS28xxDmx);
				pSpi = pWS28xxDmx;
				display.Printf(7, "%s:%d", ws28xxparms.GetLedTypeString(pWS28xxDmx->GetLEDType()), pWS28xxDmx->GetLEDCount());

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
	}
#if defined(ORANGE_PI_ONE)
	else if (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		// There is support for HEX output only
		node.SetOutput(&monitor);
		monitor.Cls();
		console_set_top_row(20);
	}
#endif
	else {
#if defined (ORANGE_PI)
		DMXParams dmxparams((DMXParamsStore *)spiFlashStore.GetStoreDmxSend());
#else
		DMXParams dmxparams;
#endif
		if (dmxparams.Load()) {
			dmxparams.Set(&dmx);
			dmxparams.Dump();
		}

		node.SetOutput(&dmx);

		if(artnetparams.IsRdm()) {
			if (artnetparams.IsRdmDiscovery()) {
				console_status(CONSOLE_YELLOW, ArtNetConst::MSG_RDM_RUN);
				display.TextStatus(ArtNetConst::MSG_RDM_RUN, DISPLAY_7SEGMENT_MSG_INFO_RDM_RUN);
				discovery.Full();
			}
			node.SetRdmHandler(&discovery);
		}
	}

	node.Print();

	if (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI) {
		assert(pSpi != 0);
		pSpi->Print();
	} else if (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR) {
		// Nothing to print
	} else {
		dmx.Print();
	}

#if defined (ORANGE_PI)
	RemoteConfig remoteConfig(REMOTE_CONFIG_ARTNET, artnetparams.IsRdm() ? REMOTE_CONFIG_MODE_RDM : (tOutputType == LIGHTSET_OUTPUT_TYPE_SPI ? REMOTE_CONFIG_MODE_PIXEL : (tOutputType == LIGHTSET_OUTPUT_TYPE_MONITOR ? REMOTE_CONFIG_MODE_MONITOR : REMOTE_CONFIG_MODE_DMX)), node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;
#endif

	for (unsigned i = 0; i < 7; i++) {
		display.ClearLine(i);
	}
	
	display.Write(1, "Eth Art-Net 4 ");

	switch (tOutputType) {
	case LIGHTSET_OUTPUT_TYPE_SPI:
		display.PutString("Pixel");
		break;
#if defined(ORANGE_PI_ONE)
	case LIGHTSET_OUTPUT_TYPE_MONITOR:
		display.PutString("Monitor");
		break;
#endif
	default:
		if (artnetparams.IsRdm()) {
			display.PutString("RDM");
		} else {
			display.PutString("DMX");
		}
		break;
	}

	uint8_t nHwTextLength;

	display.Write(2, hw.GetBoardName(nHwTextLength));
	display.Printf(3, "IP: " IPSTR " %c", IP2STR(Network::Get()->GetIp()), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');
	display.Printf(4, "N: " IPSTR "", IP2STR(Network::Get()->GetNetmask()));
	display.Printf(5, "N: %d SubN: %d U: %d", node.GetNetSwitch(), node.GetSubnetSwitch(), nUniverse);
	display.Printf(6, "AP: %d", node.GetActiveOutputPorts());

	console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
	display.TextStatus(ArtNetConst::MSG_NODE_START, DISPLAY_7SEGMENT_MSG_INFO_NODE_START);

	node.Start();

	console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
	display.TextStatus(ArtNetConst::MSG_NODE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.HandlePacket();
#if defined (ORANGE_PI)
		remoteConfig.Run();
		spiFlashStore.Flash();
#endif
		lb.Run();
		display.Run();
	}
}

}
