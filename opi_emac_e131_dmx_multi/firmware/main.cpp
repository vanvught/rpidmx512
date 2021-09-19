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
#include "network.h"
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
#include "dmxconfigudp.h"
// DMX Input
#include "dmxinput.h"

#include "spiflashinstall.h"
#include "spiflashstore.h"
#include "remoteconfig.h"
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"

#include "firmwareversion.h"
#include "software_version.h"

#include "displayhandler.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print();

	console_puts("Ethernet sACN E1.31 ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("DMX ");
	console_set_fg_color(CONSOLE_WHITE);
#if defined(ORANGE_PI)
	console_puts("{2 Universes}\n");
#else
	console_puts("{4 Universes}\n");
#endif

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new E131Reboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	StoreE131 storeE131;
	E131Params e131params(&storeE131);

	E131Bridge bridge;

	if (e131params.Load()) {
		e131params.Set(&bridge);
		e131params.Dump();
	}

	uint16_t nUniverse;
	bool bIsSet;

	nUniverse = e131params.GetUniverse(0, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(0, e131params.GetDirection(0), nUniverse);
	}

	nUniverse = e131params.GetUniverse(1, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(1, e131params.GetDirection(1), nUniverse);
	}
#if defined (ORANGE_PI_ONE)
	nUniverse = e131params.GetUniverse(2, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(2, e131params.GetDirection(2), nUniverse);
	}
#ifndef DO_NOT_USE_UART0
	nUniverse = e131params.GetUniverse(3, bIsSet);
	if (bIsSet) {
		bridge.SetUniverse(3, e131params.GetDirection(3), nUniverse);
	}
#endif
#endif

	StoreDmxSend storeDmxSend;
	DmxParams dmxparams(&storeDmxSend);

	Dmx dmx;

	if (dmxparams.Load()) {
		dmxparams.Dump();
		dmxparams.Set(&dmx);
	}

	DmxSend dmxSend;

	dmxSend.Print();

	DmxConfigUdp *pDmxConfigUdp = nullptr;

	if (bridge.GetActiveOutputPorts() != 0) {
		bridge.SetOutput(&dmxSend);
		pDmxConfigUdp = new DmxConfigUdp;
		assert(pDmxConfigUdp != nullptr);
	}

	DmxInput dmxInput;

	if (bridge.GetActiveInputPorts() != 0) {
		bridge.SetE131Dmx(&dmxInput);
	}

	bridge.Print();

	const uint32_t nActivePorts = bridge.GetActiveInputPorts() + bridge.GetActiveOutputPorts();

	display.SetTitle("sACN E1.31 DMX %u", nActivePorts);
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
		if (pDmxConfigUdp != nullptr) {
			pDmxConfigUdp->Run();
		}
	}
}

}
