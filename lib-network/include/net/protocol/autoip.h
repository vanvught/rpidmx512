/**
 * @file autoip.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PROTOCOL_AUTOIP_H_
#define NET_PROTOCOL_AUTOIP_H_

#include "ip4_address.h"

namespace net::autoip {
static constexpr auto AUTOIP_NET = network::convert_to_uint(169,254,0,0);
static constexpr auto AUTOIP_RANGE_START = network::convert_to_uint(169,254,1,0);
static constexpr auto AUTOIP_RANGE_END = network::convert_to_uint(169,254,254,255);

enum class State {
  AUTOIP_STATE_OFF,
  AUTOIP_STATE_CHECKING,
  AUTOIP_STATE_BOUND
};
}  // namespace autoip

#endif /* NET_PROTOCOL_AUTOIP_H_ */
