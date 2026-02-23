/**
 * @file emac.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "emac/phy.h"

namespace net::emac
{
/** \defgroup platform Platform implementation
  @{
*/
/**
 * Configure the PHY interface
 * - Call \ref net::phy::Config
 */
void Config();

void AdjustLink(net::phy::Status phy_status);

/**
 *
 * - Soft MAC reset
 * - Set the MAC address
 * - Initialize rx/tx descriptors
 * - PHY Start Up -> \ref net::phy::Start
 * - Adjust the link with duplex and speed returned from \ref net::phy::Start
 * - Start RX/TX DMA
 * - Enable RX/TX
 *
 * @param[out] mac_address
 *
 */
void Start(uint8_t mac_address[], net::phy::Link& link);
/** @} */
namespace display
{
void Config();
void Start();
void Status(bool); // TODO (a) subject for removal
void Shutdown();
} // namespace display
} // namespace net::emac

#endif  // EMAC_EMAC_H_
