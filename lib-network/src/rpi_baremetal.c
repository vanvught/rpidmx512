#if defined (BARE_METAL)
/**
 * @file rpi_baremetal.c
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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "util.h"

#include "wifi.h"
#include "wifi_udp.h"

#include "network.h"

static const char *_hostname;
static uint8_t _net_macaddr[NETWORK_MAC_SIZE];
static uint32_t _local_ip;
static uint32_t _netmask;
static uint32_t _broadcast_ip;
static bool _is_dhcp_used;

void network_init(void) {
	struct ip_info info;;

	_hostname = wifi_get_hostname();
	(void) wifi_get_macaddr(_net_macaddr);
	(void) wifi_get_ip_info(&info);

	_local_ip = info.ip.addr;
	_netmask = info.netmask.addr;

	_is_dhcp_used = wifi_station_is_dhcp_used();
}

void network_begin(const uint16_t port) {
	wifi_udp_begin(port);
}

const bool network_get_macaddr(/*@out@*/const uint8_t *macaddr) {
	assert(macaddr != 0);

	memcpy((void *)macaddr, _net_macaddr , NETWORK_MAC_SIZE);
	return true;
}

const uint32_t network_get_ip(void) {
	return _local_ip;
}

const uint32_t network_get_netmask(void) {
	return _netmask;
}

const uint32_t network_get_bcast(void) {
	return _broadcast_ip;
}

const char *network_get_hostname(void) {
	return _hostname;
}

bool network_is_dhcp_used(void) {
	return _is_dhcp_used;
}

uint16_t network_recvfrom(const uint8_t *packet, const uint16_t size, uint32_t *from_ip, uint16_t *from_port) {
	return wifi_udp_recvfrom(packet, size, from_ip, from_port);
}

void network_sendto(const uint8_t *packet, const uint16_t size, const uint32_t to_ip, const uint16_t remote_port) {
	wifi_udp_sendto(packet, size, to_ip, remote_port);
}

void network_joingroup(const uint32_t ip) {
	wifi_udp_joingroup(ip);
}

void network_end(void) {

}

#endif
