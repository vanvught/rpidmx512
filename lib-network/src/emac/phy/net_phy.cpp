/**
 * net_phy.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>
#include <cstdio>

#include "emac/phy.h"
#include "emac/mmi.h"

#include "hardware.h"
#include "debug.h"

#if !defined(PHY_ADDRESS)
# define PHY_ADDRESS	1
#endif

namespace net {
bool phy_get_id(const uint32_t nAddress, PhyIdentifier& phyIdentifier) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nAddress=%.2x", nAddress);

	uint16_t nValue;

	if (!phy_read(nAddress, mmi::REG_PHYSID1, nValue)) {
		DEBUG_EXIT
		return false;
	}

	phyIdentifier.nOui =(static_cast<uint32_t>(nValue) << 14);

	if (!phy_read(nAddress, mmi::REG_PHYSID2, nValue)) {
		DEBUG_EXIT
		return false;
	}

	phyIdentifier.nOui |=       (((nValue & 0xfc00) >> 10));
	phyIdentifier.nVendorModel = ((nValue & 0x03f0) >> 4) ;
	phyIdentifier.nModelRevision = nValue & 0x000f;

	DEBUG_PRINTF("%.8x %.4x %.4x", phyIdentifier.nOui, phyIdentifier.nVendorModel, phyIdentifier.nModelRevision);
	DEBUG_EXIT
	return true;
}

Link phy_get_link(const uint32_t nAddress) {
	uint16_t nValue = 0;
	phy_read(nAddress, mmi::REG_BMSR, nValue);

	if (mmi::BMSR_LINKED_STATUS == (nValue & mmi::BMSR_LINKED_STATUS)) {
		return net::Link::STATE_UP;
	}

	return net::Link::STATE_DOWN;
}

bool phy_powerdown(const uint32_t nAddress) {
	return phy_write(nAddress, mmi::REG_BMCR, mmi::BMCR_POWERDOWN);
}

static int32_t phy_config_advertise(const uint32_t nAddress, const uint16_t nAdvertisement) {
	DEBUG_ENTRY

	uint16_t nAdvertise;
	phy_read(nAddress, mmi::REG_ADVERTISE, nAdvertise);

#ifndef NDEBUG
	debug_print_bits(nAdvertise);
#endif

	nAdvertise &= static_cast<uint16_t>(mmi::ADVERTISE_ALL | mmi::ADVERTISE_100BASE4 | mmi::ADVERTISE_PAUSE_CAP | mmi::ADVERTISE_PAUSE_ASYM);
	nAdvertise |= nAdvertisement;

#ifndef NDEBUG
	debug_print_bits(nAdvertise);
	debug_print_bits(nAdvertisement);
#endif

	if (nAdvertise != nAdvertisement) {
		if (!phy_write(nAddress, mmi::REG_ADVERTISE, nAdvertisement)) {
			DEBUG_EXIT
			/* error */
			return -1;
		}
		/* Changed */
		return 1;
	}

	DEBUG_EXIT
	/* No change */
	return 0;
}

static bool phy_restart_autonegotiation(const uint32_t nAddress) {
	uint16_t nValue;
	auto nResult = phy_read(nAddress, mmi::REG_BMCR, nValue);

	nValue |= (mmi::BMCR_AUTONEGOTIATION | mmi::BMCR_RESTART_AUTONEGOTIATION);
	/* Don't isolate the PHY if we're negotiating */
	nValue &= static_cast<uint16_t>(~(mmi::BMCR_ISOLATE));

	nResult = phy_write(nAddress, mmi::REG_BMCR, nValue);
	return nResult;
}

static bool phy_config_autonegotiation(const uint32_t nAddress, const uint16_t nAdvertisement) {
	DEBUG_ENTRY

	auto nResult = phy_config_advertise(nAddress, nAdvertisement);

	if (nResult < 0) {
		DEBUG_EXIT
		return false;
	}

	if (nResult == 0) {
		/*
		 * Advertisement hasn't changed, but maybe aneg was never on to
		 * begin with?  Or maybe phy was isolated?
		 */

		uint16_t nCR;

		if (!phy_read(nAddress, mmi::REG_BMCR, nCR)) {
			DEBUG_EXIT
			return false;
		}

		if (!(nCR & mmi::BMCR_AUTONEGOTIATION) || (nCR & mmi::BMCR_ISOLATE)) {
			nResult = 1; /* do restart aneg */
		}
	}

	/*
	 * Only restart autonegotiation if we are advertising something different
	 * than we were before.
	 */

	if (nResult > 0) {
		const auto bResult = phy_restart_autonegotiation(nAddress);
		DEBUG_EXIT
		return bResult;
	}


	DEBUG_EXIT
	return true;
}

/**
 * Update the value in \ref s_phyStatus to reflect the current link value.
 *
 * @param nAddress PHY address
 * @return true for success, false for failure
 */
static bool phy_update_link(const uint32_t nAddress, PhyStatus& phyStatus) {
	DEBUG_ENTRY

	uint16_t nBMSR;

	if (!phy_read(nAddress, mmi::REG_BMSR, nBMSR)) {
		DEBUG_EXIT
		return false;
	}

	/*
	 * If we already saw the link up, and it hasn't gone down, then
	 * we don't need to wait for autoneg again
	 */

	if ((phyStatus.link == Link::STATE_DOWN) && (nBMSR & mmi::BMSR_LINKED_STATUS)) {
		DEBUG_EXIT
		return true;
	}

	if (!(nBMSR & mmi::BMSR_AUTONEGO_COMPLETE)) {
		puts("Waiting for PHY auto negotiation to complete");

		const auto nMillis = Hardware::Get()->Millis();
		while (!(nBMSR & mmi::BMSR_AUTONEGO_COMPLETE)) {
			if ((Hardware::Get()->Millis() - nMillis) > 5000) {
				DEBUG_EXIT
				return false;
			}
			phy_read(nAddress, mmi::REG_BMSR, nBMSR);
		}

		phyStatus.link = Link::STATE_UP;

		DEBUG_PRINTF("%u", Hardware::Get()->Millis() - nMillis);
		DEBUG_EXIT
		return true;
	} else {
		phy_read(nAddress, mmi::REG_BMSR, nBMSR);
		phyStatus.link = nBMSR & mmi::BMSR_LINKED_STATUS ? Link::STATE_UP : Link::STATE_DOWN;

		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	assert(0);
	__builtin_unreachable();
	return true;
}

static void phy_parse_link(const uint32_t nAddress, PhyStatus& phyStatus) {

	phyStatus.duplex = Duplex::DUPLEX_HALF;
	phyStatus.speed = Speed::SPEED10;

	uint16_t nADVERTISE;
	phy_read(nAddress, mmi::REG_ADVERTISE, nADVERTISE);
	uint16_t nLPA;
	phy_read(nAddress, mmi::REG_LPA, nLPA);

	nLPA &= nADVERTISE;

	if (nLPA & (mmi::LPA_100FULL | mmi::LPA_100HALF)) {
		phyStatus.speed = Speed::SPEED100;

		if (nLPA & mmi::LPA_100FULL) {
			phyStatus.duplex = Duplex::DUPLEX_FULL;
		}
	} else if (nLPA & mmi::LPA_10FULL) {
		phyStatus.duplex = Duplex::DUPLEX_FULL;
	}
}

bool phy_start(const uint32_t nAddress, PhyStatus& phyStatus) {
	DEBUG_ENTRY

	constexpr auto nAdvertisement = net::mmi::ADVERTISE_FULL;

	if (!phy_config_autonegotiation(nAddress, nAdvertisement)) {
		DEBUG_EXIT
		return false;
	}

	if (!phy_update_link(nAddress, phyStatus)) {
		DEBUG_EXIT
		return false;
	}

	phy_parse_link(nAddress, phyStatus);

	phyStatus.link = phy_get_link(nAddress);

	DEBUG_PRINTF("Link %s, %d, %s",
			phyStatus.link == net::Link::STATE_UP ? "Up" : "Down",
			phyStatus.speed == net::Speed::SPEED10 ? 10 : 100,
			phyStatus.duplex == net::Duplex::DUPLEX_HALF ? "HALF" : "FULL");

	DEBUG_EXIT
	return true;
}

}  // namespace net
