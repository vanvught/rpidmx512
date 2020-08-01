/**
 * @file main.cpp
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

#include <stdio.h>

#include "hardware.h"
#include "networklinux.h"

#include "firmwareversion.h"
#include "software_version.h"

static const char Interfaces[][8] = { "eno1" , "en0", "eth0", "ens32" };

/*
 *
 */
#include "ptpclient.h"

int main(int argc, char **argv) {
	Hardware hw;
	NetworkLinux nw;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	fw.Print();

	if (argc < 2) {
		bool bIf = false;

		for (uint32_t i = 0; i < sizeof(Interfaces) / sizeof(Interfaces[0]); i++) {
			if (nw.Init(Interfaces[i]) >= 0) {
				bIf = true;
				break;
			}
		}

		if (!bIf) {
			printf("Usage: %s ip_address|interface_name\n", argv[0]);
			return -1;
		}
	} else if (nw.Init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -2;
	}

	Network::Get()->Print();

	/*
	 *
	 */
	PtpClient ptp;

	ptp.Start();
	ptp.Print();

	for (;;) {
		/*
		 *
		 */
		ptp.Run();
	}

	return 0;
}
