/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2016-2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "midimonitor.h"

#include "midi.h"
#include "midi_params.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	LedBlink lb;

	MidiMonitor monitor;

	midi_params_init();
	midi_set_baudrate(midi_params_get_baudrate());
	midi_active_set_sense(true);
	midi_init(MIDI_DIRECTION_INPUT);

	printf("[V%s] Orange Pi One Compiled on %s at %s\n", SOFTWARE_VERSION, __DATE__, __TIME__);
	printf("MIDI Monitor, baudrate : %d, interface : %s", (int) midi_get_baudrate(), midi_get_interface_description());

	monitor.Init();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		monitor.Run();
		lb.Run();
	}
}

}
