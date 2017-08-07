#if 0
#if !defined(__linux__)
#define __linux__
#endif
#endif
#if defined(__linux__) || defined (__CYGWIN__)
/**
 * @file linux.c
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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <limits.h>

#include "network.h"

static char _hostname[HOST_NAME_MAX + 1];
static uint8_t _net_macaddr[NETWORK_MAC_SIZE];
static uint32_t _local_ip;
static uint32_t _gw;
static uint32_t _netmask;
static uint32_t _broadcast_ip;
static bool _is_dhcp_used;

static int _socket;

#if defined(__linux__)
static bool is_dhclient(const char *if_name) {
	char cmd[255];
	char buf[1024];
	FILE *fp;

	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));

	snprintf(cmd, sizeof(cmd) -1, "ps -A -o cmd | grep -v grep | grep dhclient | grep %s", if_name);

	fp = popen(cmd, "r");

    if (fgets(buf, sizeof(buf), fp) == NULL) {
    	pclose(fp);
    	return false;
    }

    pclose(fp);

    if ((strlen(buf) != 0) && (strstr(buf, if_name) != NULL)){
    	return true;
    }

    return false;
}
#endif

static int if_get_by_address(const char *ip, char *name, size_t len) {
	struct ifaddrs *addrs, *iap;
	struct sockaddr_in *sa;
	char buf[32];

	getifaddrs(&addrs);

	for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
		if (iap->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) (iap->ifa_addr);
			inet_ntop(iap->ifa_addr->sa_family, (void *) &(sa->sin_addr), buf, sizeof(buf));
			if (!strcmp(ip, buf)) {
				strncpy(name,iap->ifa_name, len);
				freeifaddrs(addrs);
				return 0;
			}
		}
	}

	freeifaddrs(addrs);
	return -1;
}

static int if_details(const char *iface) {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
    	perror("socket");
    	return -1;
    }

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    	close(fd);
    	return -2;
    }

    _local_ip =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

	if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0) {
    	close(fd);
    	return -3;
    }

	_broadcast_ip =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
    	close(fd);
    	return -4;
    }

    _netmask =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    	close(fd);
    	return -5;
    }

	const uint8_t* mac = (uint8_t*) ifr.ifr_ifru.ifru_hwaddr.sa_data;
	memcpy(_net_macaddr, mac, NETWORK_MAC_SIZE);

    close(fd);

    return 0;
}

void network_init(const char *s) {
	assert(s != NULL);

	char if_name[IFNAMSIZ];
	int result;
	int true_flag = true;

	if (if_get_by_address(s, if_name, sizeof(if_name)) == 0) {
	} else {
		strncpy(if_name, s, IFNAMSIZ);
	}

	result = if_details(if_name);

	if (result < 0) {
		fprintf(stderr, "Not able to start network on : %s\n", s);
		exit(EXIT_FAILURE);
	}

	if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char*) &true_flag, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	gethostname(_hostname, HOST_NAME_MAX);

#if defined(__linux__)	
	_is_dhcp_used = is_dhclient(if_name);
#endif
}

void network_begin(const uint16_t port) {
	assert(_socket > 0);

	struct sockaddr_in si_me;

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_socket, (struct sockaddr*) &si_me, sizeof(si_me)) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
}

const bool network_get_macaddr(/*@out@*/const uint8_t *macaddr) {
	assert(macaddr != NULL);

	memcpy((void *)macaddr, _net_macaddr, NETWORK_MAC_SIZE);
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

const uint32_t network_get_gw(void) {
	return _gw;
}

const char *network_get_hostname(void) {
	return _hostname;
}

bool network_is_dhcp_used(void) {
	return _is_dhcp_used;
}

uint16_t network_recvfrom(const uint8_t *packet, const uint16_t size, uint32_t *from_ip, uint16_t *from_port) {
	assert(packet != NULL);
	assert(from_ip != NULL);
	assert(from_port != NULL);

	int recv_len;
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);


	if ((recv_len = recvfrom(_socket, (void *)packet, size, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}

	*from_ip = si_other.sin_addr.s_addr;
	*from_port = ntohs(si_other.sin_port);

	return recv_len;
}

void network_sendto(const uint8_t *packet, const uint16_t size, const uint32_t to_ip, const uint16_t remote_port) {
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

    si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr = to_ip;
	si_other.sin_port = htons(remote_port);

	if (sendto(_socket, packet, size, 0, (struct sockaddr*) &si_other, slen) == -1) {
		perror("sendto");
		exit(EXIT_FAILURE);
	}
}
#endif
