/**
 * @file networklinux.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "networklinux.h"

#include "debug.h"

/**
 * BEGIN - needed H3 code compatibility
 */
#include "networkparams.h"

namespace max {
	static constexpr auto PORTS_ALLOWED = 16;
	static constexpr auto ENTRIES = (1 << 2); // Must always be a power of 2
	static constexpr auto ENTRIES_MASK __attribute__((unused)) = (ENTRIES - 1);
}

static int s_ports_allowed[max::PORTS_ALLOWED];
static int snHandles[max::PORTS_ALLOWED];

/**
 * END
 */

NetworkLinux::NetworkLinux() {
}

NetworkLinux::~NetworkLinux() {
}

int NetworkLinux::Init(const char *s) {
	int result;
/**
 * BEGIN - needed H3 code compatibility
 */
	uint32_t i;

	for (i = 0; i < max::PORTS_ALLOWED; i++) {
		s_ports_allowed[i] = 0;
		snHandles[i] = -1;
	}

	NetworkParams params;
	params.Load();
	params.Dump();

	m_nNtpServerIp = params.GetNtpServer();
/**
 * END
 */

	assert(s != nullptr);

	if (IfGetByAddress(s, m_aIfName, sizeof(m_aIfName)) == 0) {
	} else {
		strncpy(m_aIfName, s, IFNAMSIZ - 1);
	}

	DEBUG_PRINTF("m_aIfName=%s", m_aIfName);

	result = IfDetails(m_aIfName);

	if (result < 0) {
		fprintf(stderr, "Not able to start network on : %s\n", s);
		return result;
	}
#if defined (__linux__)
	else {
		m_IsDhcpUsed = IsDhclient(m_aIfName);
	}
#endif

	m_nIfIndex = if_nametoindex(const_cast<char*>(m_aIfName));

#if !defined ( __CYGWIN__ )
	if (m_nIfIndex == 0) {
		perror("if_nametoindex");
		exit(EXIT_FAILURE);
	}
#endif

	if (gethostname(m_aHostName, sizeof(m_aHostName)) < 0) {
		perror("gethostname");
	}

	m_aHostName[NETWORK_HOSTNAME_SIZE - 1] = '\0';

	i = 0;

	while(i < NETWORK_HOSTNAME_SIZE && m_aHostName[i] != '.') {
		i++;
	}

	m_aHostName[i++] = '\0';

	uint32_t j = 0;

	while (j < NETWORK_DOMAINNAME_SIZE && i < NETWORK_HOSTNAME_SIZE && m_aHostName[i] != '\0') {
		m_aDomainName[j++] = m_aHostName[i++];
	}

	m_aDomainName[j]  = '\0';

	return result;
}

int32_t NetworkLinux::Begin(uint16_t nPort) {
	DEBUG_ENTRY
	DEBUG_PRINTF("port = %d", nPort);

	int nSocket;
	struct sockaddr_in si_me;
	int true_flag = true;
	int32_t i;


/**
 * BEGIN - needed H3 code compatibility
 */

	for (i = 0; i < max::PORTS_ALLOWED; i++) {
		if (s_ports_allowed[i] == nPort) {
			return i;
		}

		if (s_ports_allowed[i] == 0) {
			break;
		}
	}

	if (i == max::PORTS_ALLOWED) {
		perror("i == max::PORTS_ALLOWED");
		exit(EXIT_FAILURE);
		//return -1;
	}

	DEBUG_PRINTF("i=%d, nPort=%d", i, nPort);

/**
 * END
 */

	if ((nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

//	if (setsockopt(nSocket, SOL_SOCKET, SO_TIMESTAMPING, reinterpret_cast<char*>(&true_flag), sizeof(int)) == -1) {
//		perror("setsockopt(SO_TIMESTAMP)");
//	}

	if (setsockopt(nSocket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&true_flag), sizeof(int)) == -1) {
		perror("setsockopt(SO_BROADCAST)");
		exit(EXIT_FAILURE);
	}

	struct timeval recv_timeout;
	recv_timeout.tv_sec = 0;
	recv_timeout.tv_usec = 10;

	if (setsockopt(nSocket, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&recv_timeout), sizeof(recv_timeout)) == -1) {
		perror("setsockopt(SO_RCVTIMEO)");
		exit(EXIT_FAILURE);
	}

    memset(&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(nPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(nSocket, reinterpret_cast<struct sockaddr*>(&si_me), sizeof(si_me)) == -1) {
		perror("bind");
		printf(IPSTR ":%d\n", IP2STR(si_me.sin_addr.s_addr), nPort);
		exit(EXIT_FAILURE);
	}

/**
 * BEGIN - needed H3 code compatibility
 */
	s_ports_allowed[i] = nPort;

	for (uint32_t i = 0; i < max::PORTS_ALLOWED; i++) {
		DEBUG_PRINTF("s_ports_allowed[%2u]=%4u", i, s_ports_allowed[i]);
	}
/**
 * END
 */

	snHandles[i] = nSocket;

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

	for (uint32_t i = 0; i < max::PORTS_ALLOWED; i++) {
		DEBUG_PRINTF("s_ports_allowed[%2u]=%4u", i, s_ports_allowed[i]);
	}

	uint32_t i;

	for (i = 0; i < max::PORTS_ALLOWED; i++) {
		if (s_ports_allowed[i] == nPort) {
			s_ports_allowed[i] = 0;
			printf("close");
			if (close(snHandles[i]) == -1) {
				perror("unbind");
				exit(EXIT_FAILURE);
			}
			snHandles[i] = -1;
			return 0;
		}
	}

	perror("unbind");
	return -1;

/**
 * END
 */
}

void NetworkLinux::SetIp(__attribute__((unused)) uint32_t nIp) {
#if defined(__linux__)
	if (nIp == m_nLocalIp) {
		return;
	}

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

void NetworkLinux::SetNetmask(__attribute__((unused)) uint32_t nNetmask) {
#if defined(__linux__)
	m_nNetmask = nNetmask;
#endif
}

void NetworkLinux::JoinGroup(int32_t nHandle, uint32_t ip) {
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(nHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt(IP_ADD_MEMBERSHIP)");
	}
}

void NetworkLinux::LeaveGroup(int32_t nHandle, uint32_t ip) {
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(nHandle, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt(IP_DROP_MEMBERSHIP)");
	}
}

uint16_t NetworkLinux::RecvFrom(int32_t nHandle, void *pPacket, uint16_t nSize, uint32_t *pFromIp, uint16_t *pFromPort) {
	assert(pPacket != nullptr);
	assert(pFromIp != nullptr);
	assert(pFromPort != nullptr);

	int recv_len;
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);


	if ((recv_len = recvfrom(nHandle, pPacket, nSize, 0, reinterpret_cast<struct sockaddr*>(&si_other), &slen)) == -1) {
		if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
			perror("recvfrom");
		}
		return 0;
	}

	*pFromIp = si_other.sin_addr.s_addr;
	*pFromPort = ntohs(si_other.sin_port);

	return recv_len;
}

void NetworkLinux::SendTo(int32_t nHandle, const void *pPacket, uint16_t nSize, uint32_t nToIp, uint16_t nRemotePort) {
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);

#ifndef NDEBUG
	struct in_addr in;
	in.s_addr = nToIp;
	printf("network_sendto(%p, %d, %s, %d)\n", pPacket, nSize, inet_ntoa(in), nRemotePort);
#endif

    si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr = nToIp;
	si_other.sin_port = htons(nRemotePort);

	if (sendto(nHandle, pPacket, nSize, 0, reinterpret_cast<struct sockaddr*>(&si_other), slen) == -1) {
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

    if (fgets(buf, sizeof(buf), fp) == nullptr) {
    	pclose(fp);
    	return false;
    }

    pclose(fp);

    if ((strlen(buf) != 0) && (strstr(buf, if_name) != nullptr)){
    	return true;
    }

    return false;
}
#endif

uint32_t NetworkLinux::GetDefaultGateway() {
	char cmd[255];
	char buf[1024];
	FILE *fp;

	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));

	snprintf(cmd, sizeof(cmd) -1, "ip route | grep default | awk '{print $3}'");

	fp = popen(cmd, "r");

    if (fgets(buf, sizeof(buf), fp) == nullptr) {
    	pclose(fp);
    	return 0;
    }

    pclose(fp);

	struct in_addr addr;

	inet_aton(buf, &addr);

	return addr.s_addr;
}

int NetworkLinux::IfGetByAddress(const char* pIp, char* pName, size_t nLength) {
	struct ifaddrs *addrs, *iap;
	struct sockaddr_in *sa;
	char buf[32];

	getifaddrs(&addrs);

	for (iap = addrs; iap != nullptr; iap = iap->ifa_next) {
		if (iap->ifa_addr->sa_family == AF_INET) {
			sa = reinterpret_cast<struct sockaddr_in*>((iap->ifa_addr));
			inet_ntop(iap->ifa_addr->sa_family,	static_cast<void*>(&(sa->sin_addr)), buf, sizeof(buf));
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

    assert(pIfInterface != nullptr);

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

	m_nLocalIp = (reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr))->sin_addr.s_addr;

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
    	perror("ioctl(fd, SIOCGIFNETMASK, &ifr)");
    	close(fd);
    	return -3;
    }

	m_nNetmask = (reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr))->sin_addr.s_addr;

    m_nGatewayIp =  GetDefaultGateway();

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

	const uint8_t* mac = reinterpret_cast<uint8_t*>(ifr.ifr_ifru.ifru_hwaddr.sa_data);
	memcpy(m_aNetMacaddr, mac, NETWORK_MAC_SIZE);
#endif

    close(fd);

    return 0;
}

void NetworkLinux::SetHostName(const char *pHostName) {
	if(sethostname(pHostName, strlen(pHostName)) < 0) {
		perror("sethostname");
	}

	if (gethostname(m_aHostName, sizeof(m_aHostName)) < 0) {
		perror("gethostname");
	}

	m_aHostName[NETWORK_HOSTNAME_SIZE - 1] = '\0';

}
