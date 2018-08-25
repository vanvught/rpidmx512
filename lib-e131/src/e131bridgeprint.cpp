/**
 * @file e131bridgeprint.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <uuid/uuid.h>
#include <assert.h>

#include "e131bridge.h"

#include "network.h"

void E131Bridge::Print(void) {
	char uuid_str[UUID_STRING_LENGTH + 1];

	uuid_str[UUID_STRING_LENGTH] = '\0';
	uuid_unparse(GetCid(), uuid_str);

	const uint8_t *firmware_version = GetSoftwareVersion();

	printf("\nBridge configuration\n");
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" CID          : %s\n", uuid_str);
	printf(" Universe     : %d\n", GetUniverse());
	printf(" Merge mode   : %s\n", GetMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
	printf(" Multicast ip : " IPSTR "\n", IP2STR(GetMulticastIp()));
	printf(" Unicast ip   : " IPSTR "\n", IP2STR(Network::Get()->GetIp()));
}
