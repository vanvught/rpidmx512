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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <ui.h>

#include "artnetcontroller.h"

#include "hardwarelinux.h"
#include "networklinux.h"

#include "input.h"
#include "kb_linux.h"
#include "ir_linux.h"
#include "buttonsbw.h"
#include "buttonsadafruit.h"

#include "software_version.h"

static const char input_devices[4][3] = { "af", "kb", "ir", "bw" };

void usage(void){
  fprintf(stderr,
      " usage:\n"
      "    ./linux_rpi_artipprog -i input_device [if_name:ip]\n"
      "       -i input_device   supported options: af , kb , ir and bw\n"
      "       if_name:ip         interface name (examples: eth0, wan0) or ip address\n"
	  "\n"
	  "af = Adafruit Bonnet\n"
	  "kb = Keyboard\n"
	  "ir = Infrared remote control\n"
	  "bw = BitWizard User-Interface\n"
      "\n");
}

int main(int argc, char **argv) {
	HardwareLinux hw;
	NetworkLinux nw;
	uint8_t nTextLength;
	int i;
    int input_device = -1;
    bool ioption = false;
    char *if_name = NULL;
	struct utsname os_info;
	KbLinux input_kb;
	IrLinux input_ir;
	ButtonsBw input_bw;
	ButtonsAdafruit input_af;
	InputSet *input;

	while (1) {
		char c;

		c = getopt(argc, argv, "hi:");

		if (c == (char)-1) {
			break;
		}

		switch (c) {
		case 'i':
			ioption = true;
			for (i = 0; i < (int)(sizeof(input_devices) / sizeof(input_devices[0])); i++) {
				if (strcmp(input_devices[i], optarg) == 0) {
					input_device = i;
				}
			}
			if (input_device == -1) {
				input_device = 0;
				printf("Input device not recognized. Using default device : %s\n", input_devices[input_device]);
			}
			break;
		case 'h':
			usage();
			return (0);
		case ':':
		case '?':
			fprintf(stderr, "Try `%s -h' for more information.\n", argv[0]);
			return (-2);
		default:
			fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
			fprintf(stderr, "Try `%s -h' for more information.\n", argv[0]);
			return (-2);
		}
	}

    if (!ioption) {
    	fprintf(stderr,"Error: \"linux_rpi_artipprog\" must be run with command line option -i\n");
    	usage();
    	return (-1);
    }

    argc -= optind;
    argv += optind;

    if (argc > 0) {
        if_name = argv[0];
    }

	memset(&os_info, 0, sizeof(struct utsname));
	uname(&os_info);

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetSysName(nTextLength), hw.GetVersion(nTextLength), __DATE__, __TIME__);

	if (if_name == NULL) {
		// Just give some ifname's a try ...
		if (nw.Init("eth0") < 0) {
			if (nw.Init("ens32") < 0) {
				if (nw.Init("bond0") < 0) {
					nw.Init("wlan0");
				}
			}
		}
	} else {
		nw.Init(if_name);
	}
	
	switch (input_device) {
		case 0:
			input = &input_af;
			break;
		case 1:
			input = &input_kb;
			break;
		case 2:
			input = &input_ir;
			break;
		case 3:
			input = &input_bw;
			break;
		default:
			input = &input_af;
			break;
	}

	input->Start();

	ArtNetController controller;
	Ui ui(SOFTWARE_VERSION, &controller, input);

	if (!Display::Get()->isDetected()) {
		fprintf(stderr, "No display found\n");
    	return (-1);
	}

	nw.Print();

	controller.Start();

	for (;;) {
		controller.Run();
		ui.Run();
	}
}
