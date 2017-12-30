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
#include <time.h>
#include <sys/time.h>
#include <sys/utsname.h>

#include "osc.h"
#include "oscsend.h"
#include "oscmessage.h"
#include "oscparams.h"

#include "network.h"

#include "software_version.h"

extern "C" {
extern int network_init(const char *);
}

int main(int argc, char **argv) {
	OSCParams oscparms;
	struct utsname os_info;
	uint8_t buffer[1600];
	uint32_t remote_ip;
	uint16_t incoming_port, outgoing_port, remote_port;

	if (argc < 2) {
		printf("Usage: %s ip_address|interface_name\n", argv[0]);
		return -1;
	}

	memset(&os_info, 0, sizeof(struct utsname));
	uname(&os_info);

	if (oscparms.Load()) {
		oscparms.Dump();
	}

	incoming_port = oscparms.GetIncomingPort();
	outgoing_port = oscparms.GetOutgoingPort();

	printf("[V%s] %s %s Compiled on %s at %s\n", SOFTWARE_VERSION, os_info.sysname, os_info.version[0] ==  '\0' ? "Linux" : os_info.version, __DATE__, __TIME__);
	printf("OSC, Incoming port: %d, Outgoing port: %d\n", incoming_port, outgoing_port);

	if (network_init(argv[1]) < 0) {
		fprintf(stderr, "Not able to start the network\n");
		return -1;
	}

	network_begin(incoming_port);

	printf("Running at : " IPSTR ":%d\n", IP2STR(network_get_ip()), incoming_port);
	printf("Netmask : " IPSTR "\n", IP2STR(network_get_netmask()));
	printf("Hostname : %s\n", network_get_hostname());
	printf("DHCP : %s\n", network_is_dhcp_used() ? "Yes" : "No");

	for (;;) {
		int bytes_received = network_recvfrom(buffer, sizeof buffer, &remote_ip, &remote_port);

		if (bytes_received > 0) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			struct tm ltm = *localtime(&tv.tv_sec);

			printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6ld ", ltm.tm_mday, ltm.tm_mon + 1, ltm.tm_year + 1900, ltm.tm_hour, ltm.tm_min, ltm.tm_sec, tv.tv_usec);

			if (OSC::isMatch((const char*) buffer, "/ping")) {
				OSCSend MsgSend(remote_ip, outgoing_port, "/pong", 0);
				printf("ping->pong\n");
			} else {
				OSCMessage Msg((char *) buffer, bytes_received);
				printf("path : %s\n", OSC::GetPath((char*) buffer, bytes_received));
				int argc = Msg.GetArgc();

				for (int i = 0; i < argc; i++) {
					int type = (int) Msg.GetType(i);

					printf("\targ %d, ", i);

					switch (type) {
					case OSC_INT32: {
						printf("int, %d", Msg.GetInt(i));
						break;
					}
					case OSC_FLOAT: {
						printf("float, %f", Msg.GetFloat(i));
						break;
					}
					case OSC_STRING: {
						printf("string, %s", Msg.GetString(i));
						break;
					}
					case OSC_BLOB: {
						OSCBlob blob = Msg.GetBlob(i);

						int size = (int) blob.GetDataSize();
						printf("blob, size %d, [", (int) size);

						if (size < 12) {
							for (int j = 0; j < size; j++) {
								printf("%02x", blob.GetByte(j));
								if (j + 1 < size) {
									printf(" ");
								}
							}
						}

						printf("]");
						break;
					}
					}

					const int result = Msg.GetResult();

					if (result) {
						printf(", result = %d\n", result);
					} else {
						puts("");
					}
				}
			}

			if (OSC::isMatch((const char*) buffer, "/2")) {
				OSCSend MsgSendModel(remote_ip, outgoing_port, "/info/model", "s", os_info.version[0] ==  '\0' ? "Linux" : os_info.version);
				OSCSend MsgSendInfo(remote_ip, outgoing_port, "/info/os", "s", os_info.sysname);
				OSCSend MsgSendSoc(remote_ip, outgoing_port, "/info/soc", "s", os_info.machine);
			}
		}
	}

	return 0;
}
