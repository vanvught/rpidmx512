/**
 * @file httpdhandlerequest.h
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

#ifndef HTTPD_HTTPDHANDLEREQUEST_H_
#define HTTPD_HTTPDHANDLEREQUEST_H_

#include <cstdint>

#include "http/http.h"
#include "net/protocol/tcp.h"

 #include "firmware/debug/debug_debug.h"

namespace httpd
{
#if !defined(HTTPD_CONTENT_SIZE)
#if defined(GD32F450VI) || defined(GD32F470VG)
#define HTTPD_CONTENT_SIZE (2 * TCP_DATA_SIZE)
#else
#define HTTPD_CONTENT_SIZE TCP_DATA_SIZE
#endif
#endif
static constexpr uint32_t kBufsize = HTTPD_CONTENT_SIZE;
} // namespace httpd

class HttpDeamonHandleRequest
{
   public:
    HttpDeamonHandleRequest() : connection_handle_(0), handle_(-1)
    {
		DEBUG_ENTRY(); 
		DEBUG_EXIT();
    }

   HttpDeamonHandleRequest(uint32_t connection_handle, int32_t handle) : connection_handle_(connection_handle), handle_(handle)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("[%u] connection_handle=%u, handle=%d", httpd::kBufsize, connection_handle, handle);
        DEBUG_EXIT();
    }

    void HandleRequest(uint32_t bytes_received, char* receive_buffer);

   private:
    http::Status ParseRequest();
    http::Status ParseMethod(char* line);
    http::Status ParseHeaderField(char* line);
    http::Status HandleGet();
    http::Status HandleGetTxt();
    http::Status HandleGetJson();
    http::Status HandlePost();
    http::Status HandleDelete();
    http::Status HandlePostJSON();
    http::Status HandlePostUpload();

   private:
    uint32_t connection_handle_;
    int32_t handle_;
    uint32_t content_size_{0};
    uint32_t request_data_length_{0};
    uint32_t request_content_length_{0};
    uint32_t bytes_received_{0};
    uint32_t upload_size_{0};

    char* uri_{nullptr};
    char* file_data_{nullptr};
    char* firmwarefile_name_{nullptr};
    char* receive_buffer_{nullptr};
    const char* content_{nullptr};
    char upload_filename_[16];

    http::Status status_{http::Status::UNKNOWN_ERROR};
    http::RequestMethod request_method_{http::RequestMethod::UNKNOWN};
    http::contentTypes request_content_type_{http::contentTypes::NOT_DEFINED};

    char dynamic_content_[httpd::kBufsize];
};

#endif  // HTTPD_HTTPDHANDLEREQUEST_H_
