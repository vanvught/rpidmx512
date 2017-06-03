/**
 * @file artnet_output.cpp
 *
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

#include <stddef.h>
#include <stdint.h>

#include "midi.h"
#include "ltc_reader.h"

#include "artnetnode.h"

static ArtNetNode *_node = NULL;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param tc
 * @param type
 */
void artnet_output(const struct _midi_send_tc *tc) {
	struct TArtNetTimeCode ArtNetTimeCode;

	if (_node != NULL) {
		ArtNetTimeCode.Frames = tc->frame;
		ArtNetTimeCode.Hours = tc->hour;
		ArtNetTimeCode.Minutes = tc->minute;
		ArtNetTimeCode.Seconds = tc->second;
		ArtNetTimeCode.Type = (uint8_t) tc->rate;

		_node->SendTimeCode(&ArtNetTimeCode);
	}
}

/**
 *
 * @param node
 */
void artnet_output_set_node(const ArtNetNode *node) {
	_node = (ArtNetNode *) node;
}

#ifdef __cplusplus
}
#endif



