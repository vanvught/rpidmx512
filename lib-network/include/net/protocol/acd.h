/**
 * @file acd.h
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

#ifndef NET_PROTOCOL_ACD_H_
#define NET_PROTOCOL_ACD_H_

#include <cstdint>

namespace net::acd {
/**
 *  RFC 5227 and RFC 3927 Constants
 */
#define PROBE_WAIT           1   ///< second  (initial random delay)
#define PROBE_MIN            1   ///< second  (minimum delay till repeated probe)
#define PROBE_MAX            2   ///< seconds (maximum delay till repeated probe)
#define PROBE_NUM            3   ///<         (number of probe packets)
#define ANNOUNCE_NUM         2   ///<         (number of announcement packets)
#define ANNOUNCE_INTERVAL    2   ///< seconds (time between announcement packets)
#define ANNOUNCE_WAIT        2   ///< seconds (delay before announcing)
#define MAX_CONFLICTS        10  ///<         (max conflicts before rate limiting)
#define RATE_LIMIT_INTERVAL  60  ///< seconds (delay between successive attempts)
#define DEFEND_INTERVAL      10  ///< seconds (minimum interval between defensive ARPs)

enum class State: uint8_t {
  ACD_STATE_OFF,
  ACD_STATE_PROBE_WAIT,
  ACD_STATE_PROBING,
  ACD_STATE_ANNOUNCE_WAIT,
  ACD_STATE_ANNOUNCING,
  ACD_STATE_ONGOING,
  ACD_STATE_PASSIVE_ONGOING,
  ACD_STATE_RATE_LIMIT
};

enum class Callback {
  ACD_IP_OK,            ///< IP address is good, no conflicts found in checking state
  ACD_RESTART_CLIENT,   ///< Conflict found -> the client should try again
  ACD_DECLINE           ///< Decline the received IP address (rate limiting)
};
}  // namespace acd

#endif /* NET_PROTOCOL_ACD_H_ */
