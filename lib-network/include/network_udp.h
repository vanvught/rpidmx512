/**
 * @file network_udp.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NETWORK_UDP_H_
#define NETWORK_UDP_H_

#include <cstdint>

namespace network::udp
{
typedef void (*UdpCallbackFunctionPtr)(const uint8_t*, uint32_t, uint32_t, uint16_t);

int32_t Begin(uint16_t, UdpCallbackFunctionPtr callback);
int32_t End(uint16_t);
uint32_t Recv(const int32_t, const uint8_t**, uint32_t*, uint16_t*);
void Send(int32_t, const uint8_t*, uint32_t, uint32_t, uint16_t);
void SendWithTimestamp(int32_t, const uint8_t*, uint32_t, uint32_t, uint16_t);
} // namespace net::udp

#endif // NETWORK_UDP_H_
