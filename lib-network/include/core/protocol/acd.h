/**
 * @file acd.h
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_PROTOCOL_ACD_H_
#define CORE_PROTOCOL_ACD_H_

#include <cstdint>

namespace network::acd
{
//  RFC 5227 and RFC 3927 Constants
inline constexpr uint32_t kProbeWait = 1;          ///< second  (initial random delay)
inline constexpr uint32_t kProbeMin = 1;           ///< second  (minimum delay till repeated probe)
inline constexpr uint32_t kProbeMax = 2;           ///< seconds (maximum delay till repeated probe)
inline constexpr uint32_t kProbeNum = 3;           ///<         (number of probe packets)
inline constexpr uint32_t kAnnounceNum = 2;        ///<         (number of announcement packets)
inline constexpr uint32_t kAnnounceInterval = 2;   ///< seconds (time between announcement packets)
inline constexpr uint32_t kAnnounceWait = 2;       ///< seconds (delay before announcing)
inline constexpr uint32_t kMaxConflicts = 10;      ///<         (max conflicts before rate limiting)
inline constexpr uint32_t kRateLimitInterval = 60; ///< seconds (delay between successive attempts)
inline constexpr uint32_t kDefendInterval = 10;    ///< seconds (minimum interval between defensive ARPs)

enum class State : uint8_t
{
    kAcdStateOff,
    kAcdStateProbeWait,
    kAcdStateProbing,
    kAcdStateAnnounceWait,
    kAcdStateAnnouncing,
    kAcdStateOngoing,
    kAcdStatePassiveOngoing,
    kAcdStateRateLimit
};

enum class Callback
{
    kAcdIpOk,          ///< IP address is good, no conflicts found in checking state
    kAcdRestartClient, ///< Conflict found -> the client should try again
    kAcdDecline        ///< Decline the received IP address (rate limiting)
};
} // namespace network::acd

#endif /* CORE_PROTOCOL_ACD_H_ */
