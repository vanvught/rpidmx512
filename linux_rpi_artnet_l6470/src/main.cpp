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

#include "artnetnode.h"
#include "artnetparams.h"

#include "slushdmx.h"
#include "sparkfundmx.h"

#if defined (__linux__)
#include "ipprog.h"
#endif

#include "network.h"

#include "software_version.h"

enum TBoards {
	BOARD_SLUSH = 0,
	BOARD_SPARKFUN
};

static const char sLongName[] = "Raspberry Pi Art-Net 3 L6470 Stepper Motor Controller";

extern "C" {
extern int network_init(const char *);
}

int main(int argc, char **argv) {
	struct utsname os_info;
	ArtNetParams artnetparams;
	ArtNetNode node;
	LightSet *pBoard;
	TBoards board;
#if defined (__linux__)
	IpProg ipprog;
#endif

	if (argc < 3) {
		printf("Usage: %s ip_address|interface_name slush|sparkfun\n", argv[0]);
		return -1;
	}

	if (strcmp(argv[2], "slush") == 0) {
		board = BOARD_SLUSH;
	} else if (strcmp(argv[2], "sparkfun") == 0) {
		board = BOARD_SPARKFUN;
	} else {
		fprintf(stderr, "Option \'%s\' is not implemented\n", argv[2]);
		return -1;
	}

	memset(&os_info, 0, sizeof(struct utsname));
	uname(&os_info);

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, os_info.sysname, os_info.version[0] ==  '\0' ? "Linux" : os_info.version, __DATE__, __TIME__);
	puts(sLongName);

	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		return -1;
	}

	if (network_init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	(void) artnetparams.Load();
	artnetparams.Dump();
	artnetparams.Set(&node);

	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	if (board == BOARD_SLUSH) {
		SlushDmx *pSlushDmx;
		pSlushDmx = new SlushDmx;
		pSlushDmx->ReadConfigFiles();
		pBoard = pSlushDmx;
	} else {
		SparkFunDmx *pSparkFunDmx;
		pSparkFunDmx = new SparkFunDmx;
		pSparkFunDmx->ReadConfigFiles();
		pBoard = pSparkFunDmx;
	}

	node.SetOutput(pBoard);

#if defined (__linux__)
	if (getuid() == 0) {
		node.SetIpProgHandler(&ipprog);
	}
#endif

	char *params_long_name = (char *)artnetparams.GetLongName();
	if (*params_long_name == 0) {
		node.SetLongName(sLongName);
	}

	printf("Running at : " IPSTR "\n", IP2STR(network_get_ip()));
	printf("Netmask : " IPSTR "\n", IP2STR(network_get_netmask()));
	printf("Hostname : %s\n", network_get_hostname());
#if defined (__linux__)
	printf("DHCP : %s\n", network_is_dhcp_used() ? "Yes" : "No");
#endif

	printf("\nNode configuration\n");
	const uint8_t *firmware_version = node.GetSoftwareVersion();
	printf(" Firmware     : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" Short name   : %s\n", node.GetShortName());
	printf(" Long name    : %s\n", node.GetLongName());
	printf(" Net          : %d\n", node.GetNetSwitch());
	printf(" Sub-Net      : %d\n", node.GetSubnetSwitch());
	printf(" Universe     : %d\n", node.GetUniverseSwitch(0));
	printf(" Active ports : %d\n\n", node.GetActiveOutputPorts());

	node.Start();

	for (;;) {
		int bytes = node.HandlePacket();
		if (bytes > 0) {
#ifndef NDEBUG
			printf("(%d)\n", bytes);
#endif
		}
	}

	return 0;
}
