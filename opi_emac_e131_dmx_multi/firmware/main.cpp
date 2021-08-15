/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "hardware.h"
#include "networkemac.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "storedisplayudf.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131reboot.h"
#include "storee131.h"
#include "e131msgconst.h"

// DMX Output
#include "dmxparams.h"
#include "dmxsend.h"
#include "storedmxsend.h"
// DMX Input
#include "dmxinput.h"

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
	NetworkEmac nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreE131 storeE131;
	StoreDmxSend storeDmxSend;

	E131Params e131params(&storeE131);

	if (e131params.Load()) {
		e131params.Dump();
	}

	const auto portDir = e131params.GetDirection();

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(CONSOLE_GREEN);
	if (portDir == e131::PortDir::INPUT) {
		console_puts("DMX Input");
	} else {
		console_puts("DMX Output");
	}
	console_set_fg_color(CONSOLE_WHITE);
#if defined(ORANGE_PI)
	console_puts(" {2 Universes}\n");
#else
	console_puts(" {4 Universes}\n");
#endif

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new E131Reboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.SetNetworkDisplay(new DisplayUdfNetworkHandler);
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	E131Bridge bridge;

	e131params.Set(&bridge);

	uint16_t nUniverse;
	bool bIsSetIndividual = false;
	bool bIsSet;

	nUniverse = e131params.GetUniverse(0, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(0, portDir, nUniverse);
		bIsSetIndividual = true;
	}

	nUniverse = e131params.GetUniverse(1, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(1, portDir, nUniverse);
		bIsSetIndividual = true;
	}
#if defined (ORANGE_PI_ONE)
	nUniverse = e131params.GetUniverse(2, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(2, portDir, nUniverse);
		bIsSetIndividual = true;
	}
#ifndef DO_NOT_USE_UART0
	nUniverse = e131params.GetUniverse(3, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(3, portDir, nUniverse);
		bIsSetIndividual = true;
	}
#endif
#endif

	if (!bIsSetIndividual) { // Backwards compatibility
		nUniverse = e131params.GetUniverse();
		bridge.SetUniverse(0, portDir, 0 + nUniverse);
		bridge.SetUniverse(1, portDir, static_cast<uint16_t>(1 + nUniverse));
#if defined (ORANGE_PI_ONE)
		bridge.SetUniverse(2, portDir, static_cast<uint16_t>(2 + nUniverse));
#ifndef DO_NOT_USE_UART0
		bridge.SetUniverse(3, portDir, static_cast<uint16_t>(3 + nUniverse));
#endif
#endif
	}

	DmxSend *pDmxOutput;
	DmxInput *pDmxInput;

	if (portDir == e131::PortDir::INPUT) {
		pDmxInput = new DmxInput;
		assert(pDmxInput != nullptr);

		bridge.SetE131Dmx(pDmxInput);
	} else {
		pDmxOutput = new DmxSend;
		assert(pDmxOutput != nullptr);

		DmxParams dmxparams(&storeDmxSend);

		if (dmxparams.Load()) {
			dmxparams.Dump();
			dmxparams.Set(pDmxOutput);
		}

		bridge.SetOutput(pDmxOutput);

		pDmxOutput->Print();
	}

	bridge.Print();

	display.SetTitle("sACN E1.31 DMX %s", e131params.GetDirection() == e131::PortDir::INPUT ? "Input" : "Output");
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::NETMASK);
	display.Set(4, displayudf::Labels::UNIVERSE_PORT_A);
	display.Set(5, displayudf::Labels::UNIVERSE_PORT_B);
	display.Set(6, displayudf::Labels::BOARDNAME);

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	const uint32_t nActivePorts = (e131params.GetDirection() == e131::PortDir::INPUT ? bridge.GetActiveInputPorts() : bridge.GetActiveOutputPorts());

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::DMX, nActivePorts);

	StoreRemoteConfig storeRemoteConfig;

	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if (remoteConfigParams.Load()) {
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
