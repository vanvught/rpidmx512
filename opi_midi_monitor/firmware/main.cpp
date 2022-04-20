/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "ledblink.h"
#include "display.h"
#include "console.h"

#include "spiflashinstall.h"

#if !defined(NO_EMAC)
# include "networkh3emac.h"
# include "remoteconfig.h"
# include "remoteconfigparams.h"
# include "storeremoteconfig.h"
# include "spiflashstore.h"
# include "storenetwork.h"
#endif

#include "midi.h"
#include "midiparams.h"
#include "midimonitor.h"

#include "software_version.h"
#include "firmwareversion.h"

extern "C" {

using namespace midi;

void notmain(void) {
	Hardware hw;
#if !defined(NO_EMAC)
	NetworkH3emac nw;
#endif
	LedBlink lb;
	Display display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
#if !defined(NO_EMAC)
	SpiFlashStore spiFlashStore;
#endif

	Midi midi;

	midi.SetActiveSense(true);
	midi.Init(Direction::INPUT);

#if !defined(NO_EMAC)
	nw.SetNetworkStore(StoreNetwork::Get());
	nw.Init(StoreNetwork::Get());
	nw.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::MIDI, remoteconfig::Output::MONITOR);
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;
#endif

	console_clear();
	fw.Print();
	printf("MIDI Monitor, baudrate : %d, interface : %s", midi.GetBaudrate(), EXT_MIDI_UART_NAME);

	MidiMonitor monitor;
	monitor.Init();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		monitor.Run();
#if !defined(NO_EMAC)
		nw.Run();
		remoteConfig.Run();
#endif
		lb.Run();
	}
}

}
