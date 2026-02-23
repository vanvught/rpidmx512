/**
 * @file net_link_check.h
 *
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef EMAC_NET_LINK_CHECK_H_
#define EMAC_NET_LINK_CHECK_H_

#include "phy.h"

namespace net::link
{
net::phy::Link StatusRead();
void HandleChange(net::phy::Link state);
// Platform defined implementations
// #if defined(ENET_LINK_CHECK_USE_INT) || defined(ENET_LINK_CHECK_USE_PIN_POLL)
void GpioInit();
void PinEnable();
void PinRecovery();
// #endif
// #if defined(ENET_LINK_CHECK_USE_INT)
void ExtiInit();
void InterruptInit();
// #elif defined(ENET_LINK_CHECK_USE_PIN_POLL)
void PinPollInit();
void PinPoll();
// #endif
} // namespace net::link

#endif // EMAC_NET_LINK_CHECK_H_
