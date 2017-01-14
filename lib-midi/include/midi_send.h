/**
 * @file midi_send.h
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MIDI_SEND_H_
#define MIDI_SEND_H_

#include <stdint.h>

#include "midi.h"

struct _midi_send_tc {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t frame;
	_midi_timecode_type rate;
} ;

#ifdef __cplusplus
extern "C" {
#endif

extern void midi_send_init(void);

extern void midi_send_tc(const struct _midi_send_tc *);

extern void midi_send_raw(const uint8_t *, const int16_t);

#ifdef __cplusplus
}
#endif

#endif /* MIDI_SEND_H_ */
