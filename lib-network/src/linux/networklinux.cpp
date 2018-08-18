/**
 * @file networklinux.h
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <assert.h>

#if defined (__APPLE__)
 #include <sys/sysctl.h>
 #include <sys/socket.h>
 #include <net/if.h>
 #include <net/if_dl.h>

static bool osx_get_macaddress(const char *if_name, uint8_t *mac_address) {
	int mib[6] = { CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0 };
	size_t len;
	char *buf;

	if ((mib[5] = if_nametoindex(if_name)) == 0) {
		perror("if_nametoindex error");
		return false;
	}

	if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
		perror("sysctl 1 error");
		return false;
	}

	if ((buf = (char *)malloc(len)) == NULL) {
		perror("malloc error");
		return false;
	}

	if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
		perror("sysctl 2 error");
		free(buf);
		return false;
	}

	const struct if_msghdr *ifm = (struct if_msghdr *) buf;
	const struct sockaddr_dl *sdl = (struct sockaddr_dl *) (ifm + 1);
	const unsigned char *ptr = (unsigned char *) LLADDR(sdl);
#ifndef NDEBUG 
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5));
#endif

	for(unsigned i = 0; i < 6;i++) {
		mac_address[i] = ptr[i];
	}

	free(buf);

	return true;
} 
#endif

#include "networklinux.h"



NetworkLinux::NetworkLinux(void): _socket(-1) {
	for (unsigned i = 0; i < sizeof(_if_name); i++) {
		_if_name[i] = '\0';
	}

	for (unsigned i = 0; i < sizeof(_hostname); i++) {
		_hostname[i] = '\0';
	}
}

NetworkLinux::~NetworkLinux(void) {
#ifndef NDEBUG
	printf("NetworkLinux::~NetworkLinux, _socket = %d\n", _socket);
#endif

	End();
}

int NetworkLinux::Init(const char *s) {
	int result;

	assert(s != NULL);

	if (if_get_by_address(s, _if_name, sizeof(_if_name)) == 0) {
	} else {
		strncpy(_if_name, s, IFNAMSIZ);
	}

#ifndef NDEBUG
	printf("NetworkLinux::Init, _if_name = %s\n", _if_name);
#endif

	result = if_details(_if_name);

	if (result < 0) {
		fprintf(stderr, "Not able to start network on : %s\n", s);
	}
#if defined (__linux__)
	else {
		m_IsDhcpUsed = is_dhclient(_if_name);
	}
#endif

	gethostname(_hostname, HOST_NAME_MAX);

	return result;
}

void NetworkLinux::Begin(uint16_t nPort) {
	struct sockaddr_in si_me;
	int true_flag = true;

#ifndef NDEBUG
	printf("NetworkLinux::Begin, _socket = %d, port = %d\n", _socket, nPort);
#endif

	if (_socket > 0) {
		close(_socket);
	}

	if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char*) &true_flag, sizeof(int)) == -1) {
		perror("setsockopt(SO_BROADCAST)");
		exit(EXIT_FAILURE);
	}

	struct timeval recv_timeout;
	recv_timeout.tv_sec = 0;
	recv_timeout.tv_usec = 10;

	if (setsockopt(_socket,SOL_SOCKET,SO_RCVTIMEO,(void *)&recv_timeout,sizeof(recv_timeout))== -1) {
		perror("setsockopt(SO_RCVTIMEO)");
		exit(EXIT_FAILURE);
	}

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(nPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_socket, (struct sockaddr*) &si_me, sizeof(si_me)) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
}


const char* NetworkLinux::GetHostName(void) {
	return _hostname;
}
void NetworkLinux::MacAddressCopyTo(uint8_t* pMacAddress) {
	for (unsigned i =  0; i < NETWORK_MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}
}

void NetworkLinux::End(void) {
#ifndef NDEBUG
	printf("NetworkLinux::End, _socket = %d\n", _socket);
#endif

	if (_socket > 0) {
		close(_socket);
	}
	_socket = -1;

	m_nLocalIp = 0;
	m_nGatewayIp = 0;
	m_nNetmask = 0;
	m_nBroadcastIp = 0;
	m_IsDhcpUsed = false;
}


void NetworkLinux::SetIp(uint32_t ip) {
#if defined(__linux__)
    struct ifreq ifr;
    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (fd == -1) {
    	perror("socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)");
    	return;
    }

    strncpy(ifr.ifr_name, _if_name, IFNAMSIZ);

    ifr.ifr_addr.sa_family = AF_INET;


    addr->sin_addr.s_addr = ip;
    if (ioctl(fd, SIOCSIFADDR, &ifr) == -1) {
    	perror("ioctl-SIOCSIFADDR");
    	return;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1) {
    	perror("ioctl-SIOCGIFFLAGS");
    	return;
    }

    strncpy(ifr.ifr_name, _if_name, IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) == -1) {
    	perror("ioctl-SIOCGIFFLAGS");
    	return;
    }

    close(fd);

    m_IsDhcpUsed = false;
    m_nLocalIp = ip;
#endif
}

void NetworkLinux::JoinGroup(uint32_t ip) {
	struct ip_mreq mreq;

	assert(_socket != -1);

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt(IP_ADD_MEMBERSHIP)");
	}
}

uint16_t NetworkLinux::RecvFrom(uint8_t* packet, uint16_t size, uint32_t* from_ip, uint16_t* from_port) {
	assert(packet != NULL);
	assert(from_ip != NULL);
	assert(from_port != NULL);

	int recv_len;
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);


	if ((recv_len = recvfrom(_socket, (void *)packet, size, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
		if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
			perror("recvfrom");
			//exit(EXIT_FAILURE);
		}
		return 0;
	}

	*from_ip = si_other.sin_addr.s_addr;
	*from_port = ntohs(si_other.sin_port);

	return recv_len;
}

void NetworkLinux::SendTo(const uint8_t* packet, uint16_t size, uint32_t to_ip, uint16_t remote_port) {
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

	assert(_socket != -1);

#ifndef NDEBUG
	struct in_addr in;
	in.s_addr = to_ip;
	printf("network_sendto(%p, %d, %s, %d)\n", packet, size, inet_ntoa(in), remote_port);
#endif

    si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr = to_ip;
	si_other.sin_port = htons(remote_port);

	if (sendto(_socket, packet, size, 0, (struct sockaddr*) &si_other, slen) == -1) {
		perror("sendto");
	}
}

#if defined(__linux__)
bool NetworkLinux::is_dhclient(const char* if_name) {
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

int NetworkLinux::if_get_by_address(const char* ip, char* name, size_t len) {
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

int NetworkLinux::if_details(const char *iface) {
    int fd;
    struct ifreq ifr;

    assert(iface != NULL);

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
    	perror("socket");
    	return -1;
    }

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    	perror("ioctl(fd, SIOCGIFADDR, &ifr)");
    	close(fd);
    	return -2;
    }

    m_nLocalIp =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

	if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0) {
		perror("ioctl(fd, SIOCGIFBRDADDR, &ifr)");
    	close(fd);
    	return -3;
    }

	m_nBroadcastIp =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
    	perror("ioctl(fd, SIOCGIFNETMASK, &ifr)");
    	close(fd);
    	return -4;
    }

    m_nNetmask =  ((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr.s_addr;

#if defined (__APPLE__)
	if(!(osx_get_macaddress(iface,m_aNetMacaddr))) {
		return -5;
	}
#else
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    	perror("ioctl(fd, SIOCGIFHWADDR, &ifr)");
    	close(fd);
    	return -5;
    }

	const uint8_t* mac = (uint8_t*) ifr.ifr_ifru.ifru_hwaddr.sa_data;
	memcpy(m_aNetMacaddr, mac, NETWORK_MAC_SIZE);
#endif

    close(fd);

    return 0;
}
