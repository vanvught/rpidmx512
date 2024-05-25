/**
 * @file phy.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef EMAC_PHY_H_
#define EMAC_PHY_H_

#include <cstdint>

namespace net {
enum class Link {
	STATE_DOWN, STATE_UP
};

enum class Duplex {
	DUPLEX_HALF, DUPLEX_FULL
};

enum class Speed {
	SPEED10, SPEED100, SPEED1000
};

struct PhyStatus {
	Link link;
	Duplex duplex;
	Speed speed;
	bool bAutonegotiation;
};

struct PhyIdentifier {
	uint32_t nOui;				///< 24-bit Organizationally Unique Identifier.
	uint16_t nVendorModel;		///< 6-bit Manufacturer’s model number.
	uint16_t nModelRevision;	///< 4-bit Manufacturer’s revision number.
};

/** \defgroup generic Generic implementation
  @{
*/

bool phy_get_id(const uint32_t nAddress, PhyIdentifier& phyIdentifier);
Link phy_get_link(const uint32_t nAddress);

/**
 *
 * @param nAddress PHY address
 * @return true for success, false for failure
 */
bool phy_powerdown(const uint32_t nAddress);

/**
 *
 * Called from \ref emac_start
 *
 * @param nAddress PHY address
 * @return true for success, false for failure
 */
bool phy_start(const uint32_t nAddress, PhyStatus& phyStatus);


const char *phy_string_get_link(const Link link);
const char *phy_string_get_duplex(const Duplex duplex);
const char *phy_string_get_speed(const Speed speed);
const char *phy_string_get_autonegotiation(const bool autonegotiation);

/** @} */

/** \defgroup platform Platform implementation
  @{
*/

/**
 * Reading a given PHY register
 * @param nAddress PHY address
 * @param nRegister PHY register number to read
 * @param nValue Returned value
 * @return
 */
bool phy_read(const uint32_t nAddress, const uint32_t nRegister, uint16_t& nValue);

/**
 *
 * @param nAddress PHY address
 * @param nRegister PHY register number to write
 * @param nValue Value to write
 * @return true for success, false for failure
 */
bool phy_write(const uint32_t nAddress, const uint32_t nRegister, uint16_t nValue);

/**
 * PHY interface configuration (configure SMI and reset PHY)
 * Called from \ref emac_config
 * @param nAddress true for success, false for failure
 * @return
 */
bool phy_config(const uint32_t nAddress);
/** @} */

/** \defgroup specific PHY specific
  @{
*/
void phy_customized_led();
void phy_customized_timing();
void phy_customized_status(PhyStatus& phyStatus);
/** @} */
}

#endif /* EMAC_PHY_H_ */
