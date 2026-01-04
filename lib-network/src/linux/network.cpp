#if !defined(CONFIG_NETWORK_USE_MINIMUM)
/**
 * @file network.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef DEBUG_NETWORK
#undef NDEBUG
#endif

#include <cstdio>
#include <cstdint>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <cassert>

#include "linux/network.h"
#include "net/netif.h"
#include "net/udp.h"
#if !defined(CONFIG_NET_APPS_NO_MDNS)
#include "net/apps/mdns.h"
#endif
#include "network_store.h"
#include "json/networkparams.h"
#include "../../config/net_config.h"
#include "net/protocol/udp.h"
#include "firmware/debug/debug_debug.h"

static int IfGetByAddress(const char*, char*, size_t);
#if defined(__linux__)
static bool IsDhclient(const char*);
#endif
#if defined(__APPLE__)
bool OSxGetMacaddress(const char*, uint8_t*);
#endif

/**
 * BEGIN - needed H3 code compatibility
 */
#define MAX_SEGMENT_LENGTH 1400

// static uint8_t s_ReadBuffer[MAX_SEGMENT_LENGTH];

namespace max
{
static constexpr auto ENTRIES = (1 << 2); // Must always be a power of 2
static constexpr auto ENTRIES_MASK [[maybe_unused]] = (ENTRIES - 1);
} // namespace max

struct PortInfo
{
    net::udp::UdpCallbackFunctionPtr callback;
    uint16_t nPort;
};

struct Port
{
    PortInfo info;
    int nSocket;
};

static Port s_Ports[UDP_MAX_PORTS_ALLOWED];

#include "net/netif.h"
#include "net/ip4_address.h"

/**
 * END
 */

extern char s_hostname[net::HOSTNAME_SIZE];
extern char s_domain_name[net::DOMAINNAME_SIZE];
char m_aIfName[IFNAMSIZ];
static int s_if_index;
extern uint32_t s_nameservers[net::NAMESERVERS_COUNT];

#if defined(__linux__)
static bool IsDhclient(const char* if_name)
{
    char cmd[255];
    char buf[1024];
    FILE* fp;

    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, sizeof(buf));

    snprintf(cmd, sizeof(cmd) - 1, "ps -A -o cmd | grep -v grep | grep dhclient | grep %s", if_name);

    fp = popen(cmd, "r");

    if (fgets(buf, sizeof(buf), fp) == nullptr)
    {
        pclose(fp);
        return false;
    }

    pclose(fp);

    if ((strlen(buf) != 0) && (strstr(buf, if_name) != nullptr))
    {
        return true;
    }

    return false;
}
#endif

static uint32_t GetDefaultGateway()
{
    char cmd[255];
    char buf[1024];
    FILE* fp;

    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, sizeof(buf));

    snprintf(cmd, sizeof(cmd) - 1, "ip route | grep default | awk '{print $3}'");

    fp = popen(cmd, "r");

    if (fgets(buf, sizeof(buf), fp) == nullptr)
    {
        pclose(fp);
        return 0;
    }

    pclose(fp);

    struct in_addr addr;

    inet_aton(buf, &addr);

    return addr.s_addr;
}

static int IfGetByAddress(const char* pIp, char* pName, size_t nLength)
{
    struct ifaddrs *addrs, *iap;
    struct sockaddr_in* sa;
    char buf[32];

    getifaddrs(&addrs);

    for (iap = addrs; iap != nullptr; iap = iap->ifa_next)
    {
        if (iap->ifa_addr->sa_family == AF_INET)
        {
            sa = reinterpret_cast<struct sockaddr_in*>((iap->ifa_addr));
            inet_ntop(iap->ifa_addr->sa_family, static_cast<void*>(&(sa->sin_addr)), buf, sizeof(buf));
            if (!strcmp(pIp, buf))
            {
                strncpy(pName, iap->ifa_name, nLength);
                freeifaddrs(addrs);
                return 0;
            }
        }
    }

    freeifaddrs(addrs);
    return -1;
}

static int IfDetails(const char* pIfInterface)
{
    int fd;
    struct ifreq ifr;

    assert(pIfInterface != nullptr);

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, pIfInterface, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        perror("ioctl(fd, SIOCGIFADDR, &ifr)");
        close(fd);
        return -2;
    }

    netif::globals::netif_default.ip.addr = (reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr))->sin_addr.s_addr;

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
    {
        perror("ioctl(fd, SIOCGIFNETMASK, &ifr)");
        close(fd);
        return -3;
    }

    net::ip4_addr_t netmask;
    netmask.addr = (reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr))->sin_addr.s_addr;
    netif::SetNetmask(netmask);

    net::ip4_addr_t gw;
    gw.addr = GetDefaultGateway();
    netif::SetGw(gw);

#if defined(__APPLE__)
    if (!(OSxGetMacaddress(pIfInterface, netif::globals::netif_default.hwaddr)))
    {
        return -5;
    }
#else
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror("ioctl(fd, SIOCGIFHWADDR, &ifr)");
        close(fd);
        return -5;
    }

    const uint8_t* mac = reinterpret_cast<uint8_t*>(ifr.ifr_ifru.ifru_hwaddr.sa_data);
    memcpy(netif::globals::netif_default.hwaddr, mac, net::MAC_SIZE);
#endif

    close(fd);

    return 0;
}

void Network::Print()
{
    printf("Network\n");
    printf(" Hostname  : %s\n", s_hostname);
    printf(" Domain    : %s\n", s_domain_name);
    printf(" If        : %d: %s\n", s_if_index, m_aIfName);
    printf(" Inet      : " IPSTR "/%d\n", IP2STR(netif::globals::netif_default.ip.addr), net::GetNetmaskCIDR());
    printf(" Netmask   : " IPSTR "\n", IP2STR(netif::globals::netif_default.netmask.addr));
    printf(" Gateway   : " IPSTR "\n", IP2STR(netif::globals::netif_default.gw.addr));
    printf(" Broadcast : " IPSTR "\n", IP2STR(net::GetBroadcastIp()));
    printf(" Mac       : " MACSTR "\n", MAC2STR(netif::globals::netif_default.hwaddr));
    printf(" Mode      : %c\n", network::iface::AddressingMode());
}

//

namespace netif
{
void Init();

const char* GetIfName()
{
    return m_aIfName;
}
} // namespace netif

namespace net
{
namespace udp
{
int32_t Begin(uint16_t nPort, UdpCallbackFunctionPtr callback)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port = %d", nPort);

    int nSocket;
    struct sockaddr_in si_me;
    int true_flag = true;
    int32_t i;

    /**
     * BEGIN - needed H3 code compatibility
     */

    for (i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        auto& portInfo = s_Ports[i].info;

        if (portInfo.nPort == nPort)
        {
            return i;
        }

        if (portInfo.nPort == 0)
        {
            portInfo.callback = callback;
            portInfo.nPort = nPort;

            DEBUG_PRINTF("i=%d, local_port=%d[%x], callback=%p", i, nPort, nPort, reinterpret_cast<void*>(callback));
            break;
        }
    }

    if (i == UDP_MAX_PORTS_ALLOWED)
    {
        perror("i == UDP_MAX_PORTS_ALLOWED");
        exit(EXIT_FAILURE);
    }

    DEBUG_PRINTF("i=%d, nPort=%d", i, nPort);

    /**
     * END
     */

    if ((nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(nSocket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&true_flag), sizeof(int)) == -1)
    {
        perror("setsockopt(SO_BROADCAST)");
        exit(EXIT_FAILURE);
    }

    struct timeval recv_timeout;
    recv_timeout.tv_sec = 0;
    recv_timeout.tv_usec = 10;

    if (setsockopt(nSocket, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&recv_timeout), sizeof(recv_timeout)) == -1)
    {
        perror("setsockopt(SO_RCVTIMEO)");
        exit(EXIT_FAILURE);
    }

    int val = 1;
    if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
    {
        perror("setsockopt(SO_REUSEADDR)");
        exit(EXIT_FAILURE);
    }

    val = 1;
    if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == -1)
    {
        perror("setsockopt(SO_REUSEPORT)");
        exit(EXIT_FAILURE);
    }

    memset(&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(nPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(nSocket, reinterpret_cast<struct sockaddr*>(&si_me), sizeof(si_me)) == -1)
    {
        perror("bind");
        printf(IPSTR ":%d\n", IP2STR(si_me.sin_addr.s_addr), nPort);
        exit(EXIT_FAILURE);
    }

    /**
     * BEGIN - needed H3 code compatibility
     */
    for (uint32_t i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        DEBUG_PRINTF("s_Ports[%2u].info.nPort=%4u", i, s_Ports[i].info.nPort);
    }
    /**
     * END
     */

    s_Ports[i].nSocket = nSocket;

    DEBUG_PRINTF("nSocket=%d", nSocket);
    DEBUG_EXIT();
    return nSocket;
}

int32_t End(uint16_t nPort)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("nPort = %d", nPort);
    /**
     * BEGIN - needed H3 code compatibility
     */

    for (uint32_t i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        DEBUG_PRINTF("s_Ports[%2u].info.nPort=%4u", i, s_Ports[i].info.nPort);
    }

    for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        auto& portInfo = s_Ports[i].info;

        if (portInfo.nPort == nPort)
        {
            portInfo.callback = nullptr;
            portInfo.nPort = 0;

            s_Ports[i].nSocket = -1;
            return 0;
        }
    }

    perror("unbind");
    return -1;

    /**
     * END
     */
}

void Send(int32_t handle, const uint8_t* pPacket, uint32_t nSize, uint32_t nToIp, uint16_t nRemotePort)
{
    struct sockaddr_in si_other;
    socklen_t slen = sizeof(si_other);

#ifndef NDEBUG
    struct in_addr in;
    in.s_addr = nToIp;
    printf("network_sendto(%d, %s, %d)\n", nSize, inet_ntoa(in), nRemotePort);
#endif

    si_other.sin_family = AF_INET;
    si_other.sin_addr.s_addr = nToIp;
    si_other.sin_port = htons(nRemotePort);

    if (sendto(handle, pPacket, nSize, 0, reinterpret_cast<struct sockaddr*>(&si_other), slen) == -1)
    {
        perror("sendto");
    }
}
} // namespace udp

namespace tcp
{
void Run();
}

namespace igmp
{
void JoinGroup(int32_t handle, uint32_t ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("handle=%d, ip=%x", handle, ip);

    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = ip;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt(IP_ADD_MEMBERSHIP)");
    }

    DEBUG_EXIT();
}

void LeaveGroup(int32_t handle, uint32_t ip)
{
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = ip;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt(IP_DROP_MEMBERSHIP)");
    }
}
} // namespace igmp
} // namespace net

namespace network
{
void Run()
{
    for (uint32_t nPortIndex = 0; nPortIndex < UDP_MAX_PORTS_ALLOWED; nPortIndex++)
    {
        struct sockaddr_in si_other;
        socklen_t slen = sizeof(si_other);

        const auto& portInfo = s_Ports[nPortIndex].info;

        if (portInfo.callback != nullptr)
        {
            uint8_t data[MAX_SEGMENT_LENGTH];
            int nDataLength;
            if ((nDataLength = recvfrom(s_Ports[nPortIndex].nSocket, data, MAX_SEGMENT_LENGTH, 0, reinterpret_cast<struct sockaddr*>(&si_other), &slen)) > 0)
            {
                portInfo.callback(data, nDataLength, si_other.sin_addr.s_addr, ntohs(si_other.sin_port));
            }
        }
    }

    net::tcp::Run();
}
} // namespace network

Network::Network(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s ip_address|interface_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    assert(s_this == nullptr);
    s_this = this;

    memset(&s_nameservers, 0, sizeof(s_nameservers));

    /**
     * BEGIN - needed H3 code compatibility
     */
    for (uint32_t i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        s_Ports[i].nSocket = -1;
    }

    json::NetworkParams params;
    params.Load();

    /**
     * END
     */

    netif::Init();

    if (IfGetByAddress(argv[1], m_aIfName, sizeof(m_aIfName)) == 0)
    {
    }
    else
    {
        strncpy(m_aIfName, argv[1], IFNAMSIZ - 1);
    }

    DEBUG_PRINTF("m_aIfName=%s", m_aIfName);

    auto result = IfDetails(m_aIfName);

    if (result < 0)
    {
        fprintf(stderr, "Not able to start network on : %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
#if defined(__linux__)
    else
    {
        if (IsDhclient(m_aIfName))
        {
            netif::SetFlags(netif::Netif::kNetifFlagDhcpOk);
        }
        else
        {
            netif::ClearFlags(netif::Netif::kNetifFlagDhcpOk);
        }
    }
#endif

    s_if_index = if_nametoindex(const_cast<char*>(m_aIfName));

    if (s_if_index == 0)
    {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }

    memset(s_hostname, 0, sizeof(s_hostname));

    if (gethostname(s_hostname, sizeof(s_hostname)) < 0)
    {
        perror("gethostname");
    }

    uint32_t i = 0;

    while ((s_hostname[i] != '\0') && (i < net::HOSTNAME_SIZE) && (s_hostname[i] != '.'))
    {
        i++;
    }

    s_hostname[i++] = '\0';

    uint32_t j = 0;

    while (j < net::DOMAINNAME_SIZE && i < net::HOSTNAME_SIZE && s_hostname[i] != '\0')
    {
        s_domain_name[j++] = s_hostname[i++];
    }

    s_domain_name[j] = '\0';

      network::iface::SetHostname(s_hostname);

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    mdns::Init();
#endif
}

Network::~Network()
{
    for (uint32_t i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        if (s_Ports[i].info.nPort != 0)
        {
            net::udp::End(s_Ports[i].info.nPort);
        }
    }
}

#endif
