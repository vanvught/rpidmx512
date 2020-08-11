/**
 * @file e131bridgeprint.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdio.h>
#include <stdint.h>
#include <uuid/uuid.h>

#include "e131bridge.h"
#include "e131.h"

void E131Bridge::Print() {
	const uint8_t *pSoftwareVersion = GetSoftwareVersion();
	char uuid_str[UUID_STRING_LENGTH + 1];
	uuid_str[UUID_STRING_LENGTH] = '\0';
	uuid_unparse(m_Cid, uuid_str);

	printf("Bridge\n");
	printf(" Firmware : %d.%d\n", pSoftwareVersion[0], pSoftwareVersion[1]);

	if (m_State.nActiveOutputPorts != 0) {
		printf(" Output\n");

		for (uint32_t nPortIndex = 0; nPortIndex < E131_MAX_PORTS; nPortIndex++) {
			uint16_t nUniverse;
			if (GetUniverse(nPortIndex, nUniverse, E131_OUTPUT_PORT)) {
				printf("  Port %2d Universe %-3d [%s]\n", nPortIndex, nUniverse, E131::GetMergeMode(m_OutputPort[nPortIndex].mergeMode, true));
			}
		}
	}

	if (m_State.nActiveInputPorts != 0) {
		printf(" CID      : %s\n", uuid_str);
		printf(" Input\n");

		for (uint32_t nPortIndex = 0; nPortIndex < E131_MAX_UARTS; nPortIndex++) {
			uint16_t nUniverse;
			if (GetUniverse(nPortIndex, nUniverse, E131_INPUT_PORT)) {
				printf("  Port %2d Universe %-3d [%d]\n", nPortIndex, nUniverse, GetPriority(nPortIndex));
			}
		}
	}

	if (m_bDirectUpdate) {
		printf(" Direct update : Yes\n");
	}

	if (m_State.bDisableSynchronize) {
		printf(" Synchronize is disabled\n");
	}
}
