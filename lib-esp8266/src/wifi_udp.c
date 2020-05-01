/**
 * @file wifi_udp.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "esp8266.h"
#include "esp8266_cmd.h"

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void wifi_udp_begin(uint16_t port) {
	esp8266_write_4bits((uint8_t) CMD_WIFI_UDP_BEGIN);

	esp8266_write_halfword(port);
}

void wifi_udp_joingroup(uint32_t ip_address) {
	esp8266_write_4bits((uint8_t) CMD_WIFI_UDP_JOIN_GROUP);
	esp8266_write_word(ip_address);
}

uint16_t wifi_udp_recvfrom(const uint8_t *buffer, uint16_t length, uint32_t *ip_address, uint16_t *port) {
	uint16_t bytes_received;

	assert(buffer != NULL);
	assert(ip_address != NULL);
	assert(port != NULL);

	esp8266_write_4bits((uint8_t) CMD_WIFI_UDP_RECEIVE);

	bytes_received = esp8266_read_halfword();

	if (bytes_received != 0) {
		*ip_address = esp8266_read_word();
		*port = esp8266_read_halfword();
		esp8266_read_bytes(buffer, MIN(bytes_received, length));
	} else {
		*ip_address = 0;
		*port = 0;
	}

	return bytes_received;
}

void wifi_udp_sendto(const uint8_t *buffer, uint16_t length, uint32_t ip_address, uint16_t port) {
	assert(buffer != NULL);

	esp8266_write_4bits((uint8_t) CMD_WIFI_UDP_SEND);

	esp8266_write_halfword(length);
	esp8266_write_word(ip_address);
	esp8266_write_halfword(port);
	esp8266_write_bytes(buffer, length);
}
