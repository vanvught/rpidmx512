/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hardwarelinux.h"
#include "networklinux.h"

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

#include "dmxmonitor.h"

#include "software_version.h"

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	uint8_t nTextLength;
	E131Params e131params;
	DMXMonitor monitor;
	E131Uuid e131uuid;
	uuid_t uuid;
	char uuid_str[UUID_STRING_LENGTH + 1];

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name [max_dmx_channels]\n", argv[0]);
		return -1;
	}

	if (argc == 3) {
		uint16_t max_channels = atoi(argv[2]);
		if (max_channels > 512) {
			max_channels = 512;
		}
		monitor.SetMaxDmxChannels(max_channels);
	}

	if (e131params.Load()) {
		e131params.Dump();
	}

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);
	puts("sACN E1.31 Real-time DMX Monitor");

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		e131uuid.GetHardwareUuid(uuid);
	}

	nw.Begin(E131_DEFAULT_PORT);

	struct in_addr group_ip;
	(void)inet_aton("239.255.0.0", &group_ip);
	const uint16_t universe = e131params.GetUniverse();
	group_ip.s_addr = group_ip.s_addr | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF) << 24)) | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF00) << 8));
	nw.JoinGroup(group_ip.s_addr);

	E131Bridge bridge;

	bridge.setCid(uuid);
	bridge.setUniverse(e131params.GetUniverse());
	bridge.setMergeMode(e131params.GetMergeMode());
	bridge.SetOutput(&monitor);

	nw.Print();
	puts("-------------------------------------------------------------------------------------------");
	bridge.Print(group_ip.s_addr); //FIXME bridge.Print(group_ip.s_addr)
	puts("-------------------------------------------------------------------------------------------");

	for (;;) {
		(void) bridge.Run();
	}

	return 0;
}
