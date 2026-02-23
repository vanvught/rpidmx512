/**
 * @file ip4_helpers.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef IP4_IP4_HELPERS_H_
#define IP4_IP4_HELPERS_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ip4/ip4_address.h"

namespace net
{
inline constexpr size_t kIpBufferSize = 16; // For "255.255.255.255" + null

[[nodiscard]] inline const char* FormatIp(uint32_t ip, char (&buf)[kIpBufferSize])
{
    snprintf(buf, sizeof(buf), IPSTR, IP2STR(ip));
    return buf;
}

inline uint32_t ParseIpString(const char* val, uint32_t len)
{
    char tmp[kIpBufferSize]{};
    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
    memcpy(tmp, val, len);
    tmp[len] = '\0';

    struct in_addr addr;

    if (inet_aton(tmp, &addr) != 0)
    {
        return addr.s_addr;
    }

    return 0;
}
} // namespace net

#endif /* IP4_IP4_HELPERS_H_ */
