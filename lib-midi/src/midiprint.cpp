/**
 * @file midiprint.cpp
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "midi.h"

void Midi::Print() {
	const _midi_direction dir = GetDirection();
	const uint32_t nBaudrate = GetBaudrate();
	const uint8_t nChannel = GetChannel();

	printf("MIDI\n");
	printf(" Direction    : %s\n", dir == MIDI_DIRECTION_INPUT ? "Input" : "Output");
	if (dir == MIDI_DIRECTION_INPUT) {
		printf(" Channel      : %d %s\n", nChannel, nChannel == 0 ? "(OMNI mode)" : "");
	}
	printf(" Active sense : %s\n", GetActiveSense() ? "Enabled" : "Disabled");
	printf(" Baudrate     : %d %s\n", static_cast<int>(nBaudrate), nBaudrate == MIDI_BAUDRATE_DEFAULT ? "(Default)" : "");
	printf(" Interface    : %s\n", GetInterfaceDescription());
}

