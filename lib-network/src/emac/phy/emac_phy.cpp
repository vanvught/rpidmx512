/**
 * net_phy.cpp
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_EMAC_PHY)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>

#include "emac/emac_phy.h"
#include "emac/mmi.h"
#include "hal_millis.h" // IWYU pragma: keep
#include "firmware/debug/debug_debug.h"

namespace emac::phy {
bool GetId(uint16_t address, Identifier& phy_identifier) {
    DEBUG_ENTRY();
    DEBUG_PRINTF("address=%.2x", address);

    uint16_t value;

    if (!phy::Read(address, mmi::REG_PHYSID1, value)) {
        DEBUG_EXIT();
        return false;
    }

    phy_identifier.oui = (static_cast<uint32_t>(value) << 14);

    if (!phy::Read(address, mmi::REG_PHYSID2, value)) {
        DEBUG_EXIT();
        return false;
    }

    phy_identifier.oui |= (((value & 0xfc00) >> 10));
    phy_identifier.vendor_model = ((value & 0x03f0) >> 4);
    phy_identifier.model_revision = value & 0x000f;

    DEBUG_PRINTF("%.8x %.4x %.4x", phy_identifier.oui, phy_identifier.vendor_model, phy_identifier.model_revision);
    DEBUG_EXIT();
    return true;
}

Link GetLink(uint16_t address) {
    uint16_t value = 0;
    phy::Read(address, mmi::REG_BMSR, value);

    if (mmi::BMSR_LINKED_STATUS == (value & mmi::BMSR_LINKED_STATUS)) {
        return emac::phy::Link::kStateUp;
    }

    return emac::phy::Link::kStateDown;
}

bool Powerdown(uint16_t address) {
    return phy::Write(address, mmi::REG_BMCR, mmi::BMCR_POWERDOWN);
}

static int32_t ConfigAdvertisement(uint16_t address, uint16_t advertisement) {
    DEBUG_ENTRY();

    uint16_t advertise;
    phy::Read(address, mmi::REG_ADVERTISE, advertise);

#ifndef NDEBUG
    debug::PrintBits(advertise);
#endif

    advertise &= static_cast<uint16_t>(mmi::ADVERTISE_ALL | mmi::ADVERTISE_100BASE4 | mmi::ADVERTISE_PAUSE_CAP | mmi::ADVERTISE_PAUSE_ASYM);
    advertise |= advertisement;

#ifndef NDEBUG
    debug::PrintBits(advertise);
    debug::PrintBits(advertisement);
#endif

    if (advertise != advertisement) {
        if (!phy::Write(address, mmi::REG_ADVERTISE, advertisement)) {
            DEBUG_EXIT();
            // error
            return -1;
        }
        // Changed
        return 1;
    }

    DEBUG_EXIT();
    // No change
    return 0;
}

static bool RestartAutonegotiation(uint16_t address) {
    uint16_t value;
    auto result = phy::Read(address, mmi::REG_BMCR, value);

    value |= (mmi::BMCR_AUTONEGOTIATION | mmi::BMCR_RESTART_AUTONEGOTIATION);
    /* Don't isolate the PHY if we're negotiating */
    value &= static_cast<uint16_t>(~(mmi::BMCR_ISOLATE));

    result = phy::Write(address, mmi::REG_BMCR, value);
    return result;
}

static bool ConfigAutonegotiation(uint16_t address, uint16_t advertisement) {
    DEBUG_ENTRY();

    auto result = ConfigAdvertisement(address, advertisement);

    if (result < 0) {
        DEBUG_EXIT();
        return false;
    }

    if (result == 0) {
        // Advertisement hasn't changed, but maybe aneg was never on to
        // begin with?  Or maybe phy was isolated?
        uint16_t bmcr;

        if (!phy::Read(address, mmi::REG_BMCR, bmcr)) {
            DEBUG_EXIT();
            return false;
        }

        if (!(bmcr & mmi::BMCR_AUTONEGOTIATION) || (bmcr & mmi::BMCR_ISOLATE)) {
            result = 1; /* do restart aneg */
        }
    }

    // Only restart autonegotiation if we are advertising something different
    // than we were before.

    if (result > 0) {
        const auto kResult = RestartAutonegotiation(address);
        DEBUG_EXIT();
        return kResult;
    }

    DEBUG_EXIT();
    return true;
}

static bool UpdateLink(uint16_t address, Status& phy_status) {
    DEBUG_ENTRY();

    uint16_t bmsr;

    if (!phy::Read(address, mmi::REG_BMSR, bmsr)) {
        DEBUG_EXIT();
        return false;
    }

    // If we already saw the link up, and it hasn't gone down, then
    // we don't need to wait for autoneg again

    if ((phy_status.link == Link::kStateDown) && (bmsr & mmi::BMSR_LINKED_STATUS)) {
        DEBUG_EXIT();
        return true;
    }

    if (!(bmsr & mmi::BMSR_AUTONEGO_COMPLETE)) {
        puts("Waiting for PHY auto negotiation to complete");

        const auto kMillis = hal::Millis();
        while (!(bmsr & mmi::BMSR_AUTONEGO_COMPLETE)) {
            if ((hal::Millis() - kMillis) > 5000) {
                DEBUG_EXIT();
                return false;
            }
            phy::Read(address, mmi::REG_BMSR, bmsr);
        }

        phy_status.link = Link::kStateUp;

        DEBUG_PRINTF("%u", hal::Millis() - kMillis);
        DEBUG_EXIT();
        return true;
    }

    // This path is only reached if autonegotiation is complete
    phy::Read(address, mmi::REG_BMSR, bmsr);
    phy_status.link = bmsr & mmi::BMSR_LINKED_STATUS ? Link::kStateUp : Link::kStateDown;

    DEBUG_EXIT();
    return true;
}

static void ParseLink(uint16_t address, Status& phy_status) {
    phy_status.duplex = Duplex::kDuplexHalf;
    phy_status.speed = Speed::kSpeed10;

    uint16_t advertise;
    phy::Read(address, mmi::REG_ADVERTISE, advertise);
    uint16_t lpa;
    phy::Read(address, mmi::REG_LPA, lpa);

    lpa &= advertise;

    if (lpa & (mmi::LPA_100FULL | mmi::LPA_100HALF)) {
        phy_status.speed = Speed::kSpeed100;

        if (lpa & mmi::LPA_100FULL) {
            phy_status.duplex = Duplex::kDuplexFull;
        }
    } else if (lpa & mmi::LPA_10FULL) {
        phy_status.duplex = Duplex::kDuplexFull;
    }
}

bool Start(uint16_t address, Status& phy_status) {
    DEBUG_ENTRY();

    constexpr auto kAdvertisement = emac::mmi::ADVERTISE_FULL;

    if (!ConfigAutonegotiation(address, kAdvertisement)) {
        DEBUG_EXIT();
        return false;
    }

    if (!UpdateLink(address, phy_status)) {
        DEBUG_EXIT();
        return false;
    }

    ParseLink(address, phy_status);

    phy_status.link = phy::GetLink(address);

    DEBUG_PRINTF("Link %s, %d, %s", phy_status.link == emac::phy::Link::kStateUp ? "Up" : "Down", phy_status.speed == emac::phy::Speed::kSpeed10 ? 10 : 100, phy_status.duplex == emac::phy::Duplex::kDuplexHalf ? "HALF" : "FULL");
    DEBUG_EXIT();
    return true;
}

// Ensure order matches enum class Speed
constexpr const char* kSpeedNames[] = {
    "10baseT",   // Speed::SPEED10
    "100baseTX", // Speed::SPEED100
    "1000baseT"  // Speed::SPEED1000
};

static_assert(static_cast<size_t>(phy::Speed::kSpeed10) == 0, "Enum ordering mismatch");
static_assert(static_cast<size_t>(phy::Speed::kSpeed1000) < (sizeof(kSpeedNames) / sizeof(kSpeedNames[0])), "Enum range mismatch");

const char* ToString(phy::Link link) {
    return link == phy::Link::kStateUp ? "up" : "down";
}

const char* ToString(phy::Duplex duplex) {
    return duplex == phy::Duplex::kDuplexHalf ? "half" : "full";
}

const char* ToString(phy::Speed speed) {
    const auto kIndex = static_cast<size_t>(speed);
    if (kIndex < sizeof(kSpeedNames) / sizeof(kSpeedNames[0])) {
        return kSpeedNames[kIndex];
    }
    return "unknown";
}

const char* ToStringAutonegotiation(bool autonegotiation) {
    return autonegotiation ? "on" : "off";
}
} // namespace emac::phy
