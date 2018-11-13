/**
 * @file static_ip.c
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
#include <ctype.h>

typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t s_addr;
};

extern int inet_aton(const char *, struct in_addr *);

#include "w5x00.h"
#include "socket.h"

static const uint8_t SPI_CS = 8; //BCM GPIO 8, SPI CS0
static const uint32_t SPI_SPEED_HZ = 4000000; // 4MHz

static const uint8_t MAC_ADDRESS[6] = {0x02, 0x08, 0xdc, 0xab, 0xcd, 0xef};

static const char IP[] = "192.168.2.250";
static const char NET_MASK[] = "255.255.255.0";
static const char GW[] = "192.168.2.1";

#define SOCKET_NUMBER	0
#define NET_LOCAL_PORT	6454 // 8000

#define IP2STR(addr) (uint8_t)(addr & 0xFF), (uint8_t)((addr >> 8) & 0xFF), (uint8_t)((addr >> 16) & 0xFF), (uint8_t)((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) (int)(mac[0]),(int)(mac[1]),(int)(mac[2]),(int)(mac[3]), (int)(mac[4]), (int)(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

void print_buffer(const uint8_t *buffer, uint16_t size);

int main(int argc, char **argv) {
	struct ip_info info;
	struct in_addr address;

	if (inet_aton(IP, &address) == 0) {
		fprintf(stderr, "Invalid local address\n");
        return -1;
	}

	info.ip.addr = address.s_addr;

	if (inet_aton(NET_MASK, &address) == 0) {
		fprintf(stderr, "Invalid netmask\n");
        return -1;
	}

	info.netmask.addr = address.s_addr;

	if (inet_aton(GW, &address) == 0) {
		fprintf(stderr, "Invalid gateway address\n");
        return -1;
	}

	info.gw.addr = address.s_addr;

	const int rc = w5x00_init(SPI_CS, SPI_SPEED_HZ, MAC_ADDRESS, &info);

	printf("w5x00_init()=%d\n", rc);

	if (rc < 0) {
		return rc;
	}

	printf("\%s configuration\n", w5x00_get_chip_name());
	printf(" Interface  : " IPSTR "\n", IP2STR(info.ip.addr));
	printf(" Netmask    : " IPSTR "\n", IP2STR(info.netmask.addr));
	printf(" Gateway    : " IPSTR "\n", IP2STR(info.gw.addr));
	printf(" MacAddress : " MACSTR "\n", MAC2STR(MAC_ADDRESS));

	uint8_t sd = _socket(SOCKET_NUMBER, Sn_MR_UDP, NET_LOCAL_PORT, 0);//SF_IO_NONBLOCK);

	if (sd != 0) {
		printf("socket(%d, Sn_MR_UDP, NET_LOCAL_PORT, 0)=%d\n", (int) SOCKET_NUMBER, (int) sd);
	}

	uint8_t sck_state;

	do {
		_getsockopt(sd , SO_STATUS, &sck_state);
	} while (sck_state != SOCK_UDP);

	uint8_t recv_buffer[2048];
	uint8_t remote_addres[4];
	uint16_t remote_port;

	for(;;) {
		const int32_t bytes_received = _recvfrom(SOCKET_NUMBER, recv_buffer, sizeof(recv_buffer), remote_addres, &remote_port);

		if (bytes_received < 0) {
			printf("Error recvfrom=%d\n", (int) bytes_received);
			break;
		} else if (bytes_received > 0) {
			printf("%d.%d.%d.%d:%d %d:", remote_addres[0],remote_addres[1],remote_addres[2],remote_addres[3], (int) remote_port, (int) bytes_received);
			print_buffer(recv_buffer, bytes_received);
			puts("");
		}
	}

	sd = _close(SOCKET_NUMBER);

	if (sd != 0) {
		printf("close(%d)=%d\n", (int) SOCKET_NUMBER, (int) sd);
	}

	return 0;
}

void print_buffer(const uint8_t *buffer, uint16_t size) {
	unsigned i;

	for (i = 0; i < 64 && i < size; i++) {
		const int c = (int) buffer[i];
		if (isprint(c)) {
			putchar(c);
		} else {
			putchar('.');
		}
	}
}
