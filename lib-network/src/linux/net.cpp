/*
 * net.cpp
 */

#ifdef DEBUG_NET
#undef NDEBUG
#endif

#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include "linux/network.h"
#include "network_store.h"
#include "firmware/debug/debug_debug.h"

extern char m_aIfName[IFNAMSIZ];

namespace net
{
void SetPrimaryIp([[maybe_unused]] uint32_t np_in)
{
    DEBUG_ENTRY();

#if defined(__linux__)
    auto& netif = netif::globals::netif_default;

    if (np_in == netif.ip.addr)
    {
        DEBUG_EXIT();
        return;
    }

    struct ifreq ifr;
    auto* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (fd == -1)
    {
        perror("socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)");
        return;
    }

    strncpy(ifr.ifr_name, m_aIfName, IFNAMSIZ);

    ifr.ifr_addr.sa_family = AF_INET;

    addr->sin_addr.s_addr = np_in;
    if (ioctl(fd, SIOCSIFADDR, &ifr) == -1)
    {
        perror("ioctl-SIOCSIFADDR");
        return;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1)
    {
        perror("ioctl-SIOCGIFFLAGS");
        return;
    }

    strncpy(ifr.ifr_name, m_aIfName, IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) == -1)
    {
        perror("ioctl-SIOCGIFFLAGS");
        return;
    }

    close(fd);

    netif.ip.addr = np_in;

    network::store::SaveDhcp(false);
    network::store::SaveIp(np_in);
#endif

    DEBUG_EXIT();
}

void SetNetmask(uint32_t netmask_in)
{
    DEBUG_ENTRY();

    if (netmask_in == netif::Netmask())
    {
        DEBUG_EXIT();
        return;
    }

    net::ip4_addr_t netmask;
    netmask.addr = netmask_in;

    netif::SetNetmask(netmask);

    network::store::SaveNetmask(netmask_in);

    DEBUG_EXIT();
}

void SetGatewayIp(uint32_t gateway_ip)
{
    DEBUG_ENTRY();

    if (gateway_ip == netif::Gw())
    {
        DEBUG_EXIT();
        return;
    }

    net::ip4_addr_t gw;
    gw.addr = gateway_ip;

    netif::SetGw(gw);

    network::store::SaveGatewayIp(gateway_ip);

    DEBUG_EXIT();
}

} // namespace net
