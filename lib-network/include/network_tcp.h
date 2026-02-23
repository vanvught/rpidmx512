/**
 * @file network_tcp.h
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

#ifndef NETWORK_TCP_H_
#define NETWORK_TCP_H_

#include <cstdint>

namespace network::tcp
{
using ConnHandle = uint32_t;	
inline constexpr ConnHandle kInvalidConnHandle = UINT32_MAX;

using CallbackListen = void (*)(ConnHandle, const uint8_t*, uint32_t);

// Server
bool Listen(uint16_t local_port, CallbackListen cb_listen);

// Client
enum class Event : uint8_t
{
    kConnected, // client: connect handshake completed
    kClosed,    // connection closed
    kReset,     // connection reset/aborted
    kError      // generic error
};

using CallbackConnect = void (*)(uint32_t connection_handle, Event event);

int32_t Connect(uint32_t remote_ip, uint16_t remote_port, CallbackConnect cb_connect, CallbackListen cb_listen);
int32_t Close(ConnHandle connection_handle); // graceful FIN

// Common
void Abort(ConnHandle connection_handle); // RST
int32_t Send(ConnHandle connection_handle, const uint8_t* buffer, uint32_t length);
} // namespace network::tcp

#endif // NETWORK_TCP_H_
