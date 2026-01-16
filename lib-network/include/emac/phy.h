/**
 * @file phy.h
 *
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

#ifndef EMAC_PHY_H_
#define EMAC_PHY_H_

#include <cstdint>

namespace net::phy
{
enum class Link
{
    kStateDown,
    kStateUp
};

enum class Duplex
{
    kDuplexHalf,
    kDuplexFull
};

enum class Speed
{
    kSpeed10,
    kSpeed100,
    kSpeed1000
};

struct Status
{
    Link link;
    Duplex duplex;
    Speed speed;
    bool autonegotiation;
};

struct Identifier
{
    uint32_t oui;            ///< 24-bit Organizationally Unique Identifier.
    uint16_t vendor_model;   ///< 6-bit Manufacturer’s model number.
    uint16_t model_revision; ///< 4-bit Manufacturer’s revision number.
};

/** \defgroup generic Generic implementation
  @{
*/

bool GetId(uint32_t address, Identifier& phy_identifier);
Link GetLink(uint32_t address);

/**
 *
 * @param address PHY address
 * @return true for success, false for failure
 */
bool Powerdown(uint32_t address);

/**
 *
 * Called from \ref Start
 *
 * @param address PHY address
 * @return true for success, false for failure
 */
bool Start(uint32_t address, Status& phy_status);
/** @} */

/** \defgroup platform Platform implementation
  @{
*/

/**
 * Reading a given PHY register
 * @param address PHY address
 * @param reg PHY register number to read
 * @param value Returned value
 * @return
 */
bool Read(uint32_t address, uint32_t reg, uint16_t& value);

/**
 *
 * @param address PHY address
 * @param reg PHY register number to write
 * @param value Value to write
 * @return true for success, false for failure
 */
bool Write(uint32_t address, uint32_t reg, uint16_t value);

/**
 * PHY interface configuration (configure SMI and reset PHY)
 * Called from \ref EmacConfig
 * @param address true for success, false for failure
 * @return
 */
bool Config(uint32_t address);
/** @} */

/** \defgroup specific PHY specific
  @{
*/
void CustomizedLed();
void CustomizedTiming();
void CustomizedStatus(Status& phy_status);
/** @} */

const char* ToString(Link link);
const char* ToString(Duplex duplex);
const char* ToString(Speed speed);
const char* ToStringAutonegotiation(bool autonegotiation);
} // namespace net::phy

#endif  // EMAC_PHY_H_
