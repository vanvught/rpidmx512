/**
 * @file main.cpp
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

#include "dmxmonitor.h"

#include "network.h"

#include "software_version.h"

extern "C" {
extern int network_init(const char *);
}

int main(int argc, char **argv) {
	struct utsname os_info;
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

	memset(&os_info, 0, sizeof(struct utsname));
	uname(&os_info);

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, os_info.sysname, os_info.version[0] ==  '\0' ? "Linux" : os_info.version, __DATE__, __TIME__);
	puts("sACN E1.31 Real-time DMX Monitor");

	if (network_init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	if (e131params.isHaveCustomCid()) {
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		e131uuid.GetHardwareUuid(uuid);
		uuid_unparse(uuid, uuid_str);
	}

	network_begin(E131_DEFAULT_PORT);

	struct in_addr group_ip;
	(void)inet_aton("239.255.0.0", &group_ip);
	const uint16_t universe = e131params.GetUniverse();
	group_ip.s_addr = group_ip.s_addr | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF) << 24)) | ((uint32_t)(((uint32_t)universe & (uint32_t)0xFF00) << 8));
	network_joingroup(group_ip.s_addr);

	E131Bridge bridge;

	bridge.setCid(uuid);
	bridge.setUniverse(e131params.GetUniverse());
	bridge.setMergeMode(e131params.GetMergeMode());
	bridge.SetOutput(&monitor);

	printf("Running at : " IPSTR "\n", IP2STR(network_get_ip()));
	printf("Netmask : " IPSTR "\n", IP2STR(network_get_netmask()));
	printf("Hostname : %s\n", network_get_hostname());
	printf("DHCP : %s\n", network_is_dhcp_used() ? "Yes" : "No");

	printf("\nBridge configuration\n");
	const uint8_t *firmware_version = bridge.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" CID          : %s\n", uuid_str);
	printf(" Universe     : %d\n", bridge.getUniverse());
	printf(" Merge mode   : %s\n", bridge.getMergeMode() == E131_MERGE_HTP ? "HTP" : "LTP");
	printf(" Multicast ip : " IPSTR "\n", IP2STR(group_ip.s_addr));
	printf(" Unicast ip   : " IPSTR "\n\n", IP2STR(network_get_ip()));

	for (;;) {
		(void) bridge.Run();
	}

	return 0;
}
