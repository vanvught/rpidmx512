/**
 * @file midiqf.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "midi.h"

void Midi::SendQf(const struct _midi_send_tc *tMidiTimeCode, uint32_t& nMidiQuarterFramePiece) {
	uint8_t data = nMidiQuarterFramePiece << 4;

	switch (nMidiQuarterFramePiece) {
	case 0:
		data = data | (tMidiTimeCode->nFrames & 0x0F);
		break;
	case 1:
		data = data | ((tMidiTimeCode->nFrames & 0x10) >> 4);
		break;
	case 2:
		data = data | (tMidiTimeCode->nSeconds & 0x0F);
		break;
	case 3:
		data = data | ((tMidiTimeCode->nSeconds & 0x30) >> 4);
		break;
	case 4:
		data = data | (tMidiTimeCode->nMinutes & 0x0F);
		break;
	case 5:
		data = data | ((tMidiTimeCode->nMinutes & 0x30) >> 4);
		break;
	case 6:
		data = data | (tMidiTimeCode->nHours & 0x0F);
		break;
	case 7:
		data = data | (tMidiTimeCode->nType << 1) | ((tMidiTimeCode->nHours & 0x10) >> 4);
		break;
	default:
		break;
	}

	midi_send_qf(data);

	nMidiQuarterFramePiece = (nMidiQuarterFramePiece + 1) & 0x07;
}
