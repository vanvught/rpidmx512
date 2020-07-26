/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "networkconst.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131msgconst.h"
#include "storee131.h"

#include "reboot.h"

// Addressable led
#include "lightset.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxgrouping.h"
#include "ws28xx.h"
#include "storews28xxdmx.h"
// PWM Led
#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"
#include "storetlc59711.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayudfnetworkhandler.h"
#include "displayhandler.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreE131 storeE131;
	StoreWS28xxDmx storeWS28xxDmx;
	StoreTLC59711 storeTLC59711;

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Pixel controller {1x 4 Universes}");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	hw.SetRebootHandler(new Reboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(new DisplayUdfNetworkHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	E131Bridge bridge;
	E131Params e131params(&storeE131);

	if (e131params.Load()) {
		e131params.Set(&bridge);
		e131params.Dump();
	}

	const uint8_t nUniverse = e131params.GetUniverse();

	bridge.SetUniverse(0, E131_OUTPUT_PORT, nUniverse);

	LightSet *pSpi;

	bool isLedTypeSet = false;

	TLC59711DmxParams pwmledparms(&storeTLC59711);

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
		WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

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
			display.Printf(7, "%s:%d G%d", WS28xx::GetLedTypeString(pWS28xxDmxGrouping->GetLEDType()), pWS28xxDmxGrouping->GetLEDCount(), pWS28xxDmxGrouping->GetLEDGroupCount());
		} else  {
			WS28xxDmx *pWS28xxDmx = new WS28xxDmx;
			assert(pWS28xxDmx != 0);
			ws28xxparms.Set(pWS28xxDmx);
			pSpi = pWS28xxDmx;
			display.Printf(7, "%s:%d", WS28xx::GetLedTypeString(pWS28xxDmx->GetLEDType()), pWS28xxDmx->GetLEDCount());

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
	}

	bridge.SetOutput(pSpi);
	bridge.Print();
	pSpi->Print();

	display.SetTitle("Eth sACN Pixel 1");
	display.Set(2, DISPLAY_UDF_LABEL_BOARDNAME);
	display.Set(3, DISPLAY_UDF_LABEL_IP);
	display.Set(4, DISPLAY_UDF_LABEL_VERSION);
	display.Set(5, DISPLAY_UDF_LABEL_UNIVERSE);
	display.Set(6, DISPLAY_UDF_LABEL_AP);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	RemoteConfig remoteConfig(REMOTE_CONFIG_E131, REMOTE_CONFIG_MODE_PIXEL, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(E131MsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	bridge.Start();

	display.TextStatus(E131MsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
	}
}

}
