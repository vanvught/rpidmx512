/**
 * @file httpd.h
 *  @brief HTTP daemon class for managing HTTP server tasks.
 *
 * This class handles HTTP requests and integrates with the network and mDNS subsystems.
 * It uses placement new to construct and destruct request handlers explicitly.
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

#ifndef HTTPD_HTTPD_H_
#define HTTPD_HTTPD_H_

#include <cstdint>

#include "core/protocol/iana.h"
#include "httpdhandlerequest.h"
#include "network_tcp.h"
#include "apps/mdns.h"
#include "../../lib-network/config/net_config.h"

namespace httpd {
inline constexpr auto kPort =
#if defined(LINUX)
    network::iana::Ports::kPortHttpAlt;
#else
    network::iana::Ports::kPortHttp;
#endif
inline constexpr auto kService =
#if defined(LINUX)
    network::apps::mdns::Services::kHttpAlt;
#else
    network::apps::mdns::Services::kHttp;
#endif
} // namespace httpd

class HttpDaemon {
   public:
    HttpDaemon();

    // The daemon and its handlers live for the lifetime of the firmware.
    ~HttpDaemon() = default;

    HttpDaemon(const HttpDaemon&) = delete;
    HttpDaemon& operator=(const HttpDaemon&) = delete;
    HttpDaemon(HttpDaemon&&) = delete;
    HttpDaemon& operator=(HttpDaemon&&) = delete;

   private:
    /**
     * Raw storage for one HttpDeamonHandleRequest.
     *
     * The storage itself has trivial initialization, so it can safely be
     * placed in a NOLOAD linker section. The actual handler object is created
     * explicitly with placement new in HttpDaemon::HttpDaemon().
     */
    struct HandlerStorage {
        alignas(HttpDeamonHandleRequest) unsigned char data[sizeof(HttpDeamonHandleRequest)];
    };

    [[nodiscard]]
    static HttpDeamonHandleRequest& GetHandler(uint32_t index);

    static void Data(network::tcp::ConnHandle conn_handle, const uint8_t* buffer, uint32_t size, void* context);

    static HandlerStorage s_handle_request_storage[TCP_MAX_TCBS_ALLOWED];

    bool is_listening_{false};
};

#endif // HTTPD_HTTPD_H_
