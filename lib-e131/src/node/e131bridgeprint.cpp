/**
 * @file e131bridgeprint.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdio>
#include <cstdint>
#include <uuid/uuid.h>

#include "e131bridge.h"
#include "e131const.h"
#include "e131.h"

#if defined (E131_HAVE_DMXIN)
static constexpr auto  UUID_STRING_LENGTH =	36;
#endif

void E131Bridge::Print() {
#if defined (E131_HAVE_DMXIN)
	char uuid_str[UUID_STRING_LENGTH + 1];
	uuid_str[UUID_STRING_LENGTH] = '\0';
	uuid_unparse(m_Cid, uuid_str);
#endif
	printf("sACN E1.31 V%d.%d\n", E131Const::VERSION[0], E131Const::VERSION[1]);
#if defined (E131_HAVE_DMXIN)
	printf(" CID      : %s\n", uuid_str);
#endif
	if (m_State.nEnableOutputPorts != 0) {
		printf(" Output\n");

		for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			uint16_t nUniverse;
			if (GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT)) {
				printf("  Port %-2u %-4u %s\n", static_cast<unsigned int>(nPortIndex), static_cast<unsigned int>(nUniverse), lightset::get_merge_mode(m_OutputPort[nPortIndex].mergeMode, true));
			}
		}
	}

#if defined (E131_HAVE_DMXIN)
	if (m_State.nEnabledInputPorts != 0) {
		printf(" Input\n");

		for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			uint16_t nUniverse;
			if (GetUniverse(nPortIndex, nUniverse, lightset::PortDir::INPUT)) {
				printf("  Port %-2u %-4u %-3u\n", static_cast<unsigned int>(nPortIndex), static_cast<unsigned int>(nUniverse), GetPriority(nPortIndex));
			}
		}
	}
#endif

	if (m_State.bDisableSynchronize) {
		printf(" Synchronize is disabled\n");
	}
}
