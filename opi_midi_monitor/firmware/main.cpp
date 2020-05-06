/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>

#include "hardware.h"
#include "ledblink.h"

#include "midi.h"
#include "midiparams.h"

#include "midimonitor.h"

#include "software_version.h"
#include "firmwareversion.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	LedBlink lb;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	Midi midi;

	midi.SetActiveSense(true);
	midi.Init(MIDI_DIRECTION_INPUT);

	fw.Print();
	printf("MIDI Monitor, baudrate : %d, interface : %s", midi.GetBaudrate(), midi.GetInterfaceDescription());

	MidiMonitor monitor;
	monitor.Init();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		monitor.Run();
		lb.Run();
	}
}

}
