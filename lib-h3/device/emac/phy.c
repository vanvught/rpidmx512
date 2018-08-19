/**
 * @file phy.c
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

#include <stdint.h>

#include "h3.h"
#include "h3_hs_timer.h"

#include "mii.h"

#define CONFIG_SYS_HZ       1000
#define CONFIG_MDIO_TIMEOUT	(3 * CONFIG_SYS_HZ)

#define MDIO_CMD_MII_BUSY	(1 << 0)
#define MDIO_CMD_MII_WRITE	(1 << 1)

#define MDIO_CMD_MII_PHY_REG_ADDR_MASK	0x000001f0
#define MDIO_CMD_MII_PHY_REG_ADDR_SHIFT	4
#define MDIO_CMD_MII_PHY_ADDR_MASK	0x0001f000
#define MDIO_CMD_MII_PHY_ADDR_SHIFT	12

static int _phy_read(int addr, int reg) {
	uint32_t miiaddr = 0;
	int timeout = CONFIG_MDIO_TIMEOUT;

	miiaddr &= ~MDIO_CMD_MII_WRITE;
	miiaddr &= ~MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	miiaddr |= (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) & MDIO_CMD_MII_PHY_REG_ADDR_MASK;

	miiaddr &= ~MDIO_CMD_MII_PHY_ADDR_MASK;
	miiaddr |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) & MDIO_CMD_MII_PHY_ADDR_MASK;

	miiaddr |= MDIO_CMD_MII_BUSY;

	H3_EMAC->MII_CMD = miiaddr;

	uint32_t start = h3_hs_timer_lo_us();
	while (h3_hs_timer_lo_us() - start < timeout) {
		if (!(H3_EMAC->MII_CMD & MDIO_CMD_MII_BUSY)) {
			return H3_EMAC->MII_DATA;
		}
		udelay(10);
	};

	return -1;
}

int phy_read(int addr, int reg) {
	(void) _phy_read(addr, reg);
	return _phy_read(addr, reg);
}

int phy_write(int addr, int reg, uint16_t val) {
	uint32_t miiaddr = 0;
	int timeout = CONFIG_MDIO_TIMEOUT;

	miiaddr &= ~MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	miiaddr |= (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) & MDIO_CMD_MII_PHY_REG_ADDR_MASK;

	miiaddr &= ~MDIO_CMD_MII_PHY_ADDR_MASK;
	miiaddr |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) & MDIO_CMD_MII_PHY_ADDR_MASK;

	miiaddr |= MDIO_CMD_MII_WRITE;
	miiaddr |= MDIO_CMD_MII_BUSY;

	H3_EMAC->MII_DATA = val;
	H3_EMAC->MII_CMD = miiaddr;

	uint32_t start = h3_hs_timer_lo_us();
	while (h3_hs_timer_lo_us() - start < timeout) {
		if (!(H3_EMAC->MII_CMD & MDIO_CMD_MII_BUSY)) {
			return 0;
		}
		udelay(10);
	};

	return -1;
}

uint32_t phy_get_id(int addr) {
	int phy_reg;
	uint32_t phy_id;

	phy_reg = phy_read(addr, MII_PHYSID1);

	if (phy_reg < 0) {
		return 0;
	}

	phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = phy_read(addr, MII_PHYSID2);

	if (phy_reg < 0) {
		return 0;
	}

	phy_id |= (phy_reg & 0xffff);

	return phy_id;
}

void phy_shutdown(int addr) {
	phy_write(addr, MII_BMCR, BMCR_PDOWN);
}
