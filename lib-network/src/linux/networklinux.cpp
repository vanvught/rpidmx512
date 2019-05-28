/**
 * @file networklinux.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "networklinux.h"

#include "debug.h"

/**
 * BEGIN - needed H3 code compatibility
 */
#define MAX_PORTS_ALLOWED	4

static uint16_t sPortsAllowed[MAX_PORTS_ALLOWED];
static uint8_t snPortsUsed = 0;
static int snHandles[MAX_PORTS_ALLOWED];
/**
 * END
 */

NetworkLinux::NetworkLinux(void) {
	for (unsigned i = 0; i < sizeof(m_aIfName); i++) {
		m_aIfName[i] = '\0';
	}
}

NetworkLinux::~NetworkLinux(void) {
}

int NetworkLinux::Init(const char *s) {
	int result;
/**
 * BEGIN - needed H3 code compatibility
 */
	uint32_t i;

	for (i = 0; i < MAX_PORTS_ALLOWED; i++) {
		sPortsAllowed[i] = 0;
		snHandles[i] = -1;
	}

	snPortsUsed = 0;
/**
 * END
 */

	assert(s != NULL);

	if (IfGetByAddress(s, m_aIfName, sizeof(m_aIfName)) == 0) {
	} else {
		strncpy(m_aIfName, s, IFNAMSIZ);
	}

#ifndef NDEBUG
	printf("NetworkLinux::Init, _if_name = %s\n", m_aIfName);
#endif

	result = IfDetails(m_aIfName);

	if (result < 0) {
		fprintf(stderr, "Not able to start network on : %s\n", s);
	}
#if defined (__linux__)
	else {
		m_IsDhcpUsed = IsDhclient(m_aIfName);
	}
#endif

	gethostname(m_aHostName, sizeof(m_aHostName));

	return result;
}

int32_t NetworkLinux::Begin(uint16_t nPort) {
	DEBUG_ENTRY
	DEBUG_PRINTF("port = %d", nPort);

	int nSocket;
	struct sockaddr_in si_me;
	int true_flag = true;
	uint32_t i;


/**
 * BEGIN - needed H3 code compatibility
 */
	if (snPortsUsed == MAX_PORTS_ALLOWED) {
		perror("MAX_PORTS_ALLOWED");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < snPortsUsed; i++) {
		if (sPortsAllowed[i] == nPort) {
			DEBUG_EXIT
			return 0;
		}
	}
/**
 * END
 */

	if ((nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(nSocket, SOL_SOCKET, SO_BROADCAST, (char*) &true_flag, sizeof(int)) == -1) {
		perror("setsockopt(SO_BROADCAST)");
		exit(EXIT_FAILURE);
	}

	struct timeval recv_timeout;
	recv_timeout.tv_sec = 0;
	recv_timeout.tv_usec = 10;

	if (setsockopt(nSocket,SOL_SOCKET,SO_RCVTIMEO,(void *)&recv_timeout,sizeof(recv_timeout))== -1) {
		perror("setsockopt(SO_RCVTIMEO)");
		exit(EXIT_FAILURE);
	}

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(nPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(nSocket, (struct sockaddr*) &si_me, sizeof(si_me)) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

/**
 * BEGIN - needed H3 code compatibility
 */
		snHandles[snPortsUsed] = nSocket;
		sPortsAllowed[snPortsUsed++] = nPort;
/**
 * END
 */

	return nSocket;
}

void NetworkLinux::MacAddressCopyTo(uint8_t* pMacAddress) {
	for (unsigned i =  0; i < NETWORK_MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}
}

int32_t NetworkLinux::End(uint16_t nPort) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPort = %d", nPort);
/**
 * BEGIN - needed H3 code compatibility
 */
	DEBUG_PRINTF("s_ports_allowed_index=%d", snPortsUsed);

	if (snPortsUsed == 0) {
		perror("s_ports_used_index == 0");
		DEBUG_EXIT
		exit(EXIT_FAILURE);
	}

	DEBUG_PRINTF("s_ports_allowed[s_ports_allowed_index - 1]=%d", sPortsAllowed[snPortsUsed - 1]);

	if ((sPortsAllowed[snPortsUsed - 1]) == nPort) {
		sPortsAllowed[snPortsUsed - 1] = 0;
		if (close(snHandles[snPortsUsed - 1]) == -1) {
			perror("bind");
			exit(EXIT_FAILURE);
		}
		snPortsUsed--;
		DEBUG_EXIT
		return 0;
	}

	perror("port not in use");
	DEBUG_EXIT
	exit(EXIT_FAILURE);
/**
 * END
 */
}

void NetworkLinux::SetIp(uint32_t nIp) {
#if defined(__linux__)
    struct ifreq ifr;
    struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (fd == -1) {
    	perror("socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)");
    	return;
    }

    strncpy(ifr.ifr_name, m_aIfName, IFNAMSIZ);

    ifr.ifr_addr.sa_family = AF_INET;


    addr->sin_addr.s_addr = nIp;
    if (ioctl(fd, SIOCSIFADDR, &ifr) == -1) {
    	perror("ioctl-SIOCSIFADDR");
    	return;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1) {
    	perror("ioctl-SIOCGIFFLAGS");
    	return;
    }

    strncpy(ifr.ifr_name, m_aIfName, IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) == -1) {
    	perror("ioctl-SIOCGIFFLAGS");
    	return;
    }

    close(fd);

    m_IsDhcpUsed = false;
    m_nLocalIp = nIp;
#endif
}

void NetworkLinux::JoinGroup(uint32_t nHandle, uint32_t ip) {
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(nHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt(IP_ADD_MEMBERSHIP)");
	}
}

void NetworkLinux::LeaveGroup(uint32_t nHandle, uint32_t ip) {
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(nHandle, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt(IP_DROP_MEMBERSHIP)");
	}
}

uint16_t NetworkLinux::RecvFrom(uint32_t nHandle, uint8_t* pPacket, uint16_t nSize, uint32_t* pFromIp, uint16_t* pFromPort) {
	assert(pPacket != NULL);
	assert(pFromIp != NULL);
	assert(pFromPort != NULL);

	int recv_len;
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);


	if ((recv_len = recvfrom(nHandle, (void *)pPacket, nSize, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
		if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
			perror("recvfrom");
			//exit(EXIT_FAILURE);
		}
		return 0;
	}

	*pFromIp = si_other.sin_addr.s_addr;
	*pFromPort = ntohs(si_other.sin_port);

	return recv_len;
}

void NetworkLinux::SendTo(uint32_t nHandle, const uint8_t* pPacket, uint16_t nSize, uint32_t nToIp, uint16_t nRemotePort) {
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);

#ifndef NDEBUG
	struct in_addr in;
	in.s_addr = nToIp;
	printf("network_sendto(%p, %d, %s, %d)\n", pPacket, nSize, inet_ntoa(in), nRemotePort);
#endif

    si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr = nToIp;
	si_other.sin_port = htons(nRemotePort);

	if (sendto(nHandle, pPacket, nSize, 0, (struct sockaddr*) &si_other, slen) == -1) {
		perror("sendto");
	}
}

#if defined(__linux__)
bool NetworkLinux::IsDhclient(const char* if_name) {
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

int NetworkLinux::IfGetByAddress(const char* pIp, char* pName, size_t nLength) {
	struct ifaddrs *addrs, *iap;
	struct sockaddr_in *sa;
	char buf[32];

	getifaddrs(&addrs);

	for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
		if (iap->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) (iap->ifa_addr);
			inet_ntop(iap->ifa_addr->sa_family, (void *) &(sa->sin_addr), buf, sizeof(buf));
			if (!strcmp(pIp, buf)) {
				strncpy(pName,iap->ifa_name, nLength);
				freeifaddrs(addrs);
				return 0;
			}
		}
	}

	freeifaddrs(addrs);
	return -1;
}

int NetworkLinux::IfDetails(const char *pIfInterface) {
    int fd;
    struct ifreq ifr;

    assert(pIfInterface != NULL);

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
    	perror("socket");
    	return -1;
    }

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name , pIfInterface , IFNAMSIZ-1);

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
	if(!(OSxGetMacaddress(pIfInterface,m_aNetMacaddr))) {
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
