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
#include <cassert>
#include <new>

#include "core/protocol/iana.h"
#include "httpdhandlerequest.h"
#include "network_tcp.h"
#include "apps/mdns.h"
#include "../../lib-network/config/net_config.h"
#include "firmware/debug/debug_debug.h"

/**
 * @class HttpDaemon
 * @brief A class that manages an HTTP server with support for multiple connections.
 *
 * The HttpDaemon class sets up an HTTP server, handles incoming requests, and integrates with mDNS.
 */

class HttpDaemon
{
   public:
    HttpDaemon()
    {
        DEBUG_ENTRY();
        assert(is_listening_ == false);

        is_listening_ = network::tcp::Listen(network::iana::Ports::kPortHttp, Input);
        assert(is_listening_ == true);

        // IMPORTANT:
        // Connection handles are now GLOBAL indices into s_Tcbs[].
        // Therefore the HTTP request handler table must also be global-sized.
        for (uint32_t i = 0; i < TCP_MAX_TCBS_ALLOWED; ++i)
        {
            // Each HttpDeamonHandleRequest corresponds to ONE possible TCB slot.
            // It can be addressed directly by conn_handle.
            new (&s_handle_request[i]) HttpDeamonHandleRequest(i);
        }

        network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kHttp);

        DEBUG_EXIT();
    }

    ~HttpDaemon() = default;

   private:
    static void Input(network::tcp::ConnHandle conn_handle, const uint8_t* buffer, uint32_t size)
    {
        // Defensive: bounds check in debug builds
        assert(conn_handle < TCP_MAX_TCBS_ALLOWED);

        // Route the request to the per-connection request handler instance.
        // This preserves your "one handler per connection slot" design.
        s_handle_request[conn_handle].HandleRequest(size, const_cast<char*>(reinterpret_cast<const char*>(buffer)));
    }

#if defined(GD32F207RG) || defined(GD32F450VE) || defined(GD32F470ZK)
#define SECTION_HTTPD __attribute__((section(".httpd")))
#else
#define SECTION_HTTPD
#endif

    // NOTE:
    // The array size must match the GLOBAL connection pool size.
    // Otherwise you'd index out of bounds when multiple listeners or client conns exist.
    static inline HttpDeamonHandleRequest s_handle_request[TCP_MAX_TCBS_ALLOWED] __attribute__((aligned(4))) SECTION_HTTPD;

    // In the new design, we typically don't store a "listener handle" at all.
    // But if you want to keep Begin() returning something for now, you can store bool.
    bool is_listening_{false};
};

#endif // HTTPD_HTTPD_H_
