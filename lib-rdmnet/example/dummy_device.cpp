/**
 * @file dummy_device.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hardware.h"
#include "networklinux.h"
#include "ledblink.h"

#include "firmwareversion.h"

#include "software_version.h"

#include "rdmnetdevice.h"
#include "lightsetdebug.h"
#include "rdmpersonality.h"
#include "rdmdeviceparams.h"

#include "identify.h"

int main(int argc, char **argv) {
	Hardware hw;
	NetworkLinux nw;
	LedBlink lb;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	fw.Print();

	if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	nw.Print();

	lb.SetMode(LEDBLINK_MODE_NORMAL);

	Identify identify;

	LightSetDebug lighSetDebug;

	RDMPersonality personality("LLRP Dummy device", lighSetDebug.GetDmxFootprint());

	RDMNetDevice device(&personality);
	RDMDeviceParams rdmDeviceParams;

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&device);
		rdmDeviceParams.Dump();
	}

	device.Init();
	device.Print();
	device.Start();

	for (;;) {
		device.Run();
	}

	return 0;
}
