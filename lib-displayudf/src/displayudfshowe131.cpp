/**
 * @file displayudfshowe131.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "displayudf.h"

#include "e131bridge.h"

#include "debug.h"

#define MERGEMODE2STRING(m)		(m == E131_MERGE_HTP) ? "HTP" : "LTP"

void DisplayUdf::Show(E131Bridge *pE131Bridge) {
	DEBUG_ENTRY

	Show();

	uint16_t nUniverse;
	if (pE131Bridge->GetUniverse(0, nUniverse)) {
		Printf(m_aLabels[DISPLAY_UDF_LABEL_UNIVERSE], "U: %d", nUniverse);
	}
	Printf(m_aLabels[DISPLAY_UDF_LABEL_AP], "AP: %d", pE131Bridge->GetActiveOutputPorts());

	for (uint32_t i = 0; i < 4; i++) {
		uint16_t nUniverse;
		if (pE131Bridge->GetUniverse(i, nUniverse)) {
			Printf(m_aLabels[DISPLAY_UDF_LABEL_UNIVERSE_PORT_A + i], "Port %c: %d %s", (char) ('A' + i), nUniverse, MERGEMODE2STRING(pE131Bridge->GetMergeMode(i)));
		}
	}

	DEBUG_EXIT
}
