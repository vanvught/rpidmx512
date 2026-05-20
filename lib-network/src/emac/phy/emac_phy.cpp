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
#include "timing.h"
#include "firmware/debug/debug_debug.h"
#include "firmware/debug/debug_printbits.h" // IWYU pragma: keep

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
    uint16_t value;

    phy::Read(address, mmi::REG_BMSR, value); // clear latch
    phy::Read(address, mmi::REG_BMSR, value); // current state

    return (value & mmi::BMSR_LINKED_STATUS) ? Link::kStateUp : Link::kStateDown;
}

bool Powerdown(uint16_t address) {
    return phy::Write(address, mmi::REG_BMCR, mmi::BMCR_POWERDOWN);
}

static int32_t ConfigAdvertisement(uint16_t address, uint16_t advertisement) {
    DEBUG_ENTRY();

    uint16_t current;

    if (!phy::Read(address, mmi::REG_ADVERTISE, current)) {
        DEBUG_EXIT();
        return -1;
    }

    if (current == advertisement) {
        DEBUG_EXIT();
        return 0;
    }

    if (!phy::Write(address, mmi::REG_ADVERTISE, advertisement)) {
        DEBUG_EXIT();
        return -1;
    }

    DEBUG_EXIT();
    return 1;
}

static bool RestartAutonegotiation(uint16_t address) {
    uint16_t value;
    auto result = phy::Read(address, mmi::REG_BMCR, value);

    value |= (mmi::BMCR_AUTONEGOTIATION | mmi::BMCR_RESTART_AUTONEGOTIATION);
    // Don't isolate the PHY if we're negotiating
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

        const auto kMillis = timing::Millis();
        while (!(bmsr & mmi::BMSR_AUTONEGO_COMPLETE)) {
            if ((timing::Millis() - kMillis) > 5000) {
                DEBUG_EXIT();
                return false;
            }
            phy::Read(address, mmi::REG_BMSR, bmsr);
        }

        phy_status.link = Link::kStateUp;

        DEBUG_PRINTF("%u", timing::Millis() - kMillis);
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
    if (phy_status.link != Link::kStateUp) {
        phy_status.duplex = Duplex::kUnknown;
        phy_status.speed = Speed::kUnknown;
        return;
    }

    phy_status.duplex = Duplex::kDuplexHalf;
    phy_status.speed = Speed::kSpeed10;

    uint16_t advertise;
    phy::Read(address, mmi::REG_ADVERTISE, advertise);
    debug::PrintBits(advertise);

    uint16_t lpa;
    phy::Read(address, mmi::REG_LPA, lpa);
    debug::PrintBits(lpa);

    lpa &= advertise;

    if (lpa & mmi::LPA_100FULL) {
        phy_status.speed = Speed::kSpeed100;
        phy_status.duplex = Duplex::kDuplexFull;
    } else if (lpa & mmi::LPA_100HALF) {
        phy_status.speed = Speed::kSpeed100;
        phy_status.duplex = Duplex::kDuplexHalf;
    } else if (lpa & mmi::LPA_10FULL) {
        phy_status.speed = Speed::kSpeed10;
        phy_status.duplex = Duplex::kDuplexFull;
    } else if (lpa & mmi::LPA_10HALF) {
        phy_status.speed = Speed::kSpeed10;
        phy_status.duplex = Duplex::kDuplexHalf;
    } else {
        phy_status.speed = Speed::kUnknown;
        phy_status.duplex = Duplex::kUnknown;
    }
}

bool Start(uint16_t address, Status& phy_status) {
    DEBUG_ENTRY();

    phy_status = {.link = Link::kStateDown, .duplex = Duplex::kUnknown, .speed = Speed::kUnknown, .autonegotiation = false};

    constexpr auto kAdvertisement = emac::mmi::ADVERTISE_ALL;

    if (!ConfigAutonegotiation(address, kAdvertisement)) {
        DEBUG_EXIT();
        return false;
    }

    if (!UpdateLink(address, phy_status)) {
        DEBUG_EXIT();
        return false;
    }

    uint16_t bmcr;
    phy::Read(address, mmi::REG_BMCR, bmcr);

    phy_status.autonegotiation = (bmcr & mmi::BMCR_AUTONEGOTIATION);

    phy_status.link = phy::GetLink(address);

    if (phy_status.link == Link::kStateUp) {
        ParseLink(address, phy_status);
    } else {
        phy_status.speed = Speed::kUnknown;
        phy_status.duplex = Duplex::kUnknown;
    }

    DEBUG_PRINTF("Link %s, %s, %s", ToString(phy_status.link), ToString(phy_status.speed), ToString(phy_status.duplex));
    DEBUG_EXIT();
    return true;
}

// Ensure order matches enum class Speed
constexpr const char* kSpeedNames[] = {
    "Unknown",
    "10baseT",   // Speed::SPEED10
    "100baseTX", // Speed::SPEED100
    "1000baseT"  // Speed::SPEED1000
};

static_assert(static_cast<size_t>(phy::Speed::kUnknown) == 0, "Enum ordering mismatch");
static_assert(static_cast<size_t>(phy::Speed::kSpeed1000) < (sizeof(kSpeedNames) / sizeof(kSpeedNames[0])), "Enum range mismatch");

const char* ToString(phy::Link link) {
    return link == phy::Link::kStateUp ? "up" : "down";
}

const char* ToString(phy::Duplex duplex) {
    switch (duplex) {
        case phy::Duplex::kUnknown:
            return "unknown";
        case phy::Duplex::kDuplexHalf:
            return "half";
        case phy::Duplex::kDuplexFull:
            return "full";
    }
    return "error";
}

const char* ToString(phy::Speed speed) {
    const auto kIndex = static_cast<size_t>(speed);
    if (kIndex < sizeof(kSpeedNames) / sizeof(kSpeedNames[0])) {
        return kSpeedNames[kIndex];
    }
    return "error";
}

const char* ToStringAutonegotiation(bool autonegotiation) {
    return autonegotiation ? "on" : "off";
}
} // namespace emac::phy
