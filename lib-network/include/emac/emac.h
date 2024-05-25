/**
 * @file emac.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef EMAC_EMAC_H_
#define EMAC_EMAC_H_

#include <cstdint>

/** \defgroup platform Platform implementation
  @{
*/
/**
 * Configure the PHY interface
 * - Call \ref net::phy_config
 */
void emac_config();

/**
 *
 * - Soft MAC reset
 * - Set the MAC address
 * - Initialize rx/tx descriptors
 * - PHY Start Up -> \ref net::phy_start
 * - Adjust the link with duplex and speed returned from \ref net::phy_start
 * - Start RX/TX DMA
 * - Enable RX/TX
 *
 * @param[out] macAddress
 *
 */
void emac_start(uint8_t macAddress[], net::Link& link);
/** @} */

#endif /* EMAC_EMAC_H_ */
