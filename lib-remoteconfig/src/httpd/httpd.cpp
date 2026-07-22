/**
 * @file httpd.cpp
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

#include <cassert>
#include <new>

#include "firmware/debug/debug_debug.h"
#include "httpd/httpd.h"
#include "network_tcp.h"
#include "apps/mdns.h"
#include "../../lib-network/config/net_config.h"

/*
 * Define the storage exactly once, outside the header.
 *
 * gnu::used:
 *   Ensures that the compiler emits the object.
 *
 * .httpd:
 *   Is collected by the linker script into the dedicated HTTP RAM section.
 *
 * The linker script should contain:
 *
 *   .http (NOLOAD) :
 *   {
 *       . = ALIGN(4);
 *       _shttp = .;
 *       KEEP(*(.http))
 *       KEEP(*(.http.*))
 *       KEEP(*(.httpd))
 *       KEEP(*(.httpd.*))
 *       . = ALIGN(4);
 *       _ehttp = .;
 *   } > RAM2
 */
[[gnu::section(".httpd"), gnu::aligned(alignof(HttpDeamonHandleRequest)), gnu::used]]
HttpDaemon::HandlerStorage HttpDaemon::s_handle_request_storage[TCP_MAX_TCBS_ALLOWED];

HttpDaemon::HttpDaemon() {
    DEBUG_ENTRY();

    assert(!is_listening_);

    is_listening_ = network::tcp::Listen(httpd::kPort, Data);
    assert(is_listening_);

    /*
     * Connection handles are global indices into s_Tcbs[].
     * Therefore the request-handler table must have one entry for every
     * possible global TCB slot.
     */
    for (uint32_t i = 0; i < TCP_MAX_TCBS_ALLOWED; ++i) {
        // Construct exactly one handler in each raw-storage slot.
        ::new (static_cast<void*>(s_handle_request_storage[i].data)) HttpDeamonHandleRequest(i);
    }

    network::apps::mdns::ServiceRecordAdd(nullptr, httpd::kService);

    DEBUG_EXIT();
}

HttpDeamonHandleRequest& HttpDaemon::GetHandler(uint32_t index) {
    assert(index < TCP_MAX_TCBS_ALLOWED);

    /*
     * std::launder is appropriate after constructing an object in raw storage
     * and subsequently obtaining a pointer through the storage address.
     */
    return *std::launder(reinterpret_cast<HttpDeamonHandleRequest*>(s_handle_request_storage[index].data));
}

void HttpDaemon::Data(network::tcp::ConnHandle conn_handle, const uint8_t* buffer, uint32_t size, [[maybe_unused]] void* context) {
    assert(conn_handle < TCP_MAX_TCBS_ALLOWED);
    assert(buffer != nullptr);

    GetHandler(conn_handle).HandleRequest(size, const_cast<char*>(reinterpret_cast<const char*>(buffer)));
}