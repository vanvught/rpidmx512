#if !defined (CONFIG_NETWORK_USE_MINIMUM)
/**
 * net_phy.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#if !defined(__APPLE__)
# include <errno.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <net/if.h>
# include <linux/ethtool.h>
# include <linux/sockios.h>
#endif

#include "network.h"
#include "debug.h"

#if !defined(__APPLE__)
static struct ethtool_link_settings *ethtoolLinkSettings;

static int _phy_customized_status() {
	DEBUG_ENTRY

	int fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0) {
		perror("socket");
		DEBUG_EXIT
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, Network::Get()->GetIfName());

	char buffer[sizeof(struct ethtool_link_settings) + sizeof(uint32_t) * 3 * 128];
	ethtoolLinkSettings = (struct ethtool_link_settings *)buffer;
	memset(buffer, 0, sizeof(buffer));

	ethtoolLinkSettings->cmd = ETHTOOL_GLINKSETTINGS;
	ifr.ifr_data = (caddr_t)ethtoolLinkSettings;

	if (ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
		perror("ioctl");
		close(fd);
		DEBUG_EXIT
		return -1;
	}

	if (ethtoolLinkSettings->link_mode_masks_nwords >= 0) {
		close(fd);
		DEBUG_EXIT
		return -1;
	}

	ethtoolLinkSettings->cmd = ETHTOOL_GLINKSETTINGS;
	ethtoolLinkSettings->link_mode_masks_nwords = -ethtoolLinkSettings->link_mode_masks_nwords;

	if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)  {
		perror("ioctl");
		close(fd);
		DEBUG_EXIT
		return -1;
	}

	DEBUG_PRINTF("ethtool_link_settings.speed = %i", ethtoolLinkSettings->speed);
	DEBUG_PRINTF("ethtool_link_settings.duplex = %i",  ethtoolLinkSettings->duplex);
	DEBUG_PRINTF("ethtool_link_settings.autoneg = %i", ethtoolLinkSettings->autoneg);

	close(fd);
	DEBUG_EXIT
	return 0;
}

#undef DUPLEX_HALF
#undef DUPLEX_FULL
#endif

#include "emac/phy.h"

namespace net {
void phy_customized_status(PhyStatus& phyStatus) {
	phyStatus.link = Link::STATE_UP;
	phyStatus.duplex = Duplex::DUPLEX_FULL;
	phyStatus.speed = Speed::SPEED1000;
	phyStatus.bAutonegotiation = true;

#if !defined(__APPLE__)
	if (_phy_customized_status() < 0) {
		return;
	}

	switch (ethtoolLinkSettings->speed) {
	case 10:
		phyStatus.speed = Speed::SPEED10;
		break;
	case 100:
		phyStatus.speed = Speed::SPEED100;
		break;
	case 1000:
		phyStatus.speed = Speed::SPEED1000;
		break;
	default:
		break;
	}

	phyStatus.duplex = (ethtoolLinkSettings->duplex == 1) ? Duplex::DUPLEX_FULL : Duplex::DUPLEX_HALF;
	phyStatus.bAutonegotiation = (ethtoolLinkSettings->autoneg == 1);
#endif
}
}  // namespace net

namespace remoteconfig {
namespace dsa {
uint32_t json_get_phystatus(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"message\":\"Not implemented\"}\n"));
	return nLength;
}

uint32_t json_get_portstatus(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"message\":\"Not implemented\"}\n"));
	return nLength;
}

uint32_t json_get_vlantable(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize, "{\"message\":\"Not implemented\"}\n"));
	return nLength;
}
}  // namespace dsa
}  // namespace remoteconfig
#endif
