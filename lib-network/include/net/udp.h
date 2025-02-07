/**
 * @file udp.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_UDP_H_
#define NET_UDP_H_

#include <cstdint>

namespace net {
typedef void (*UdpCallbackFunctionPtr)(const uint8_t *, uint32_t, uint32_t, uint16_t);

int32_t udp_begin(uint16_t, UdpCallbackFunctionPtr callback = nullptr);
int32_t udp_end(uint16_t);
uint32_t udp_recv1(const int32_t, uint8_t *, uint32_t, uint32_t *, uint16_t *);
uint32_t udp_recv2(const int32_t, const uint8_t **, uint32_t *, uint16_t *);
void udp_send(int32_t, const uint8_t *, uint32_t, uint32_t, uint16_t);
void udp_send_timestamp(int32_t, const uint8_t *, uint32_t, uint32_t, uint16_t);
}  // namespace net

#endif /* NET_UDP_H_ */
