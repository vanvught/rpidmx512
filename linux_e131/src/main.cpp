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

#include "hardwarelinux.h"
#include "networklinux.h"
#include "ledblinklinux.h"

#include "e131bridge.h"
#include "e131uuid.h"
#include "e131params.h"

#include "dmxmonitor.h"

#if defined (RASPPI)
 #include "spiflashstore.h"
#endif

#include "software_version.h"

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	LedBlinkLinux lb;

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name [max_dmx_channels]\n", argv[0]);
		return -1;
	}

	uint8_t nTextLength;
	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);
	puts("sACN E1.31 Real-time DMX Monitor");

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	DMXMonitor monitor;

	if (argc == 3) {
		uint16_t max_channels = atoi(argv[2]);
		if (max_channels > 512) {
			max_channels = 512;
		}
		monitor.SetMaxDmxChannels(max_channels);
	}

#if defined (RASPPI)
	SpiFlashStore spiFlashStore;
	E131Params e131params((E131ParamsStore *)spiFlashStore.GetStoreE131());
	spiFlashStore.Dump();
#else
	E131Params e131params;
#endif

	if (e131params.Load()) {
		e131params.Dump();
	}

	uuid_t uuid;

	if (e131params.isHaveCustomCid()) {
		char uuid_str[UUID_STRING_LENGTH + 1];
		memcpy(uuid_str, e131params.GetCidString(), UUID_STRING_LENGTH);
		uuid_str[UUID_STRING_LENGTH] = '\0';
		uuid_parse((const char *)uuid_str, uuid);
	} else {
		E131Uuid e131uuid;
		e131uuid.GetHardwareUuid(uuid);
	}

	E131Bridge bridge;

	bridge.SetCid(uuid);
	bridge.SetUniverse(e131params.GetUniverse());
	bridge.SetMergeMode(e131params.GetMergeMode());
	bridge.SetOutput(&monitor);

	nw.Print();
	bridge.Print();

	bridge.Start();
	lb.SetMode(LEDBLINK_MODE_NORMAL);

	for (;;) {
		(void) bridge.Run();
#if defined (RASPPI)
		spiFlashStore.Flash();
#endif
	}

	return 0;
}
