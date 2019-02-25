/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "handler.h"

#include "oscserver.h"
#include "oscserverparms.h"

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"

#if defined (RASPPI)
 #include "spiflashstore.h"
#endif

#include "software_version.h"

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

#if defined (RASPPI)
	SpiFlashStore spiFlashStore;
	OSCServerParams oscparms((OSCServerParamsStore *)spiFlashStore.GetStoreOscServer());
	spiFlashStore.Dump();
#else
	OSCServerParams oscparms;
#endif

	OscServer server;

	if (oscparms.Load()) {
		oscparms.Dump();
		oscparms.Set(&server);
	}

	uint8_t nTextLength;
	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);
	puts("OSC Real-time DMX Monitor");

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	Handler handler;
	server.SetOscServerHandler(&handler);

	DMXMonitor monitor;
	DMXMonitorParams params;

	if (params.Load()) {
		params.Dump();
		params.Set(&monitor);
	}

	server.SetOutput(&monitor);

	nw.Print();
	server.Print();

	server.Start();

#if defined (RASPPI)
	while (spiFlashStore.Flash())
		;

	spiFlashStore.Dump();
#endif

	for (;;) {
		server.Run();
	}

	return 0;
}
