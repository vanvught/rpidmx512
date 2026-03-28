/**
 * @file httpdhandlerequest.cpp
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

#if defined(DEBUG_HTTPD)
#undef NDEBUG
#endif
#if defined(__GNUC__) && !defined(__clang__)
#if !defined(CONFIG_HTTPD_OPTIMIZE_NONE)
#pragma GCC push_options
#if defined(CONFIG_HTTPD_OPTIMIZE_O3)
#pragma GCC optimize("O3")
#else
#pragma GCC optimize("O2")
#endif
#endif
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "http/http.h"
#include "httpd/httpdhandlerequest.h"
#include "http/html_infos.h"
#include "http/json_infos.h"
#include "network_tcp.h"
#include "network_iface.h"
#include "hal.h"
#include "hal_millis.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

#if !defined(_TIME_STAMP_)
#define _TIME_STAMP_ 0
#endif

const char* GetFileContent(const char* file_name, uint32_t& size, http::ContentTypes& content_type);

void HttpDeamonHandleRequest::HandleRequest(uint32_t bytes_received, char* receive_buffer)
{
    DEBUG_ENTRY();

    bytes_received_ = bytes_received;
    receive_buffer_ = receive_buffer;

    const char* status_msg = "OK";

    DEBUG_PRINTF("%u: status_=%u", connection_handle_, static_cast<uint32_t>(status_));

    // The HTTP handler keeps state across TCP segments (e.g. POST body arriving later).
    // status_ == UNKNOWN_ERROR means "we are not currently processing an in-progress request".
    if (status_ == http::Status::kUnknownError)
    {
        // Initial incoming HTTP request (header + maybe body)
        status_ = ParseRequest();

        DEBUG_PRINTF("%s %s", kRequestMethod[static_cast<uint32_t>(request_method_)],
                     request_content_type_ < http::ContentTypes::kNotDefined ? kSContentType[static_cast<uint32_t>(request_content_type_)] : "Unknown");

        if (status_ == http::Status::kOk)
        {
            // Request is syntactically valid and supported.
            if (request_method_ == http::RequestMethod::kGet)
            {
                status_ = HandleGet();
            }
            else if (request_method_ == http::RequestMethod::kPost)
            {
                // If POST has Content-Length but no data in this segment,
                // we must wait for next TCP segment(s).
                if ((request_content_length_ != 0U) && (request_data_length_ == 0U))
                {
                    DEBUG_PUTS("There is a POST header only -> no data");
                    DEBUG_EXIT();
                    return;
                }

                status_ = HandlePost();
            }
        }
    }
    else if ((status_ == http::Status::kOk) && (request_method_ == http::RequestMethod::kPost))
    {
        // Follow-up TCP segment containing POST body data.
        // We treat the new receive_buffer as body data.
        file_data_ = receive_buffer_;
        request_data_length_ = bytes_received_;

        status_ = HandlePost();

        // If we haven't received the full body yet, wait for more segments.
        if (request_data_length_ < request_content_length_)
        {
            DEBUG_EXIT();
            return;
        }
    }

#if defined(ENABLE_METHOD_DELETE)
    else if ((status_ == http::Status::kOk) && (request_method_ == http::RequestMethod::DELETE))
    {
        status_ = HandleDelete();
    }
#endif

    // If request handling failed, generate an error response or abort.
    if (status_ != http::Status::kOk)
    {
        switch (status_)
        {
            case ::http::Status::kNotModified:
                status_msg = "Not Modified";
                break;
            case http::Status::kBadRequest:
                status_msg = "Bad Request";
                break;
            case http::Status::kNotFound:
                status_msg = "Not Found";
                break;
            case http::Status::kRequestEntityTooLarge:
                status_msg = "Payload Too Large";
                break;
            case http::Status::kRequestUriTooLong:
                status_msg = "URI Too Long";
                break;
            case http::Status::kInternalServerError:
                status_msg = "Internal Server Error";
                break;
            case http::Status::kMethodNotImplemented:
                status_msg = "Not Implemented";
                break;
            case http::Status::kVersionNotSupported:
                status_msg = "HTTP Version Not Supported";
                break;
            default:
                network::tcp::Abort(connection_handle_);
                status_ = http::Status::kUnknownError;
                request_method_ = http::RequestMethod::kUnknown;
                DEBUG_EXIT();
                return;
        }

        request_content_type_ = http::ContentTypes::kTextHtml;
        content_ = dynamic_content_;
        content_size_ = static_cast<uint32_t>(snprintf(dynamic_content_, sizeof(dynamic_content_), "%u %s\n", static_cast<unsigned>(status_), status_msg));
    }

    // Build HTTP header into receive_buffer_ and send it first.
    const auto kHeaderLength = static_cast<uint32_t>(
        snprintf(receive_buffer_, sizeof(dynamic_content_) - 1U,
                 "HTTP/1.1 %u %s\r\n"
                 "Server: %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %u\r\n"
                 "Cache-Control: no-cache\r\n"
                 "ETag: \"%u\"\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 static_cast<unsigned int>(status_), status_msg, network::iface::HostName(), http::kContentType[static_cast<uint32_t>(request_content_type_)],
                 static_cast<unsigned int>(content_size_), (content_ == dynamic_content_) ? hal::Millis() : _TIME_STAMP_));

    network::tcp::Send(connection_handle_, reinterpret_cast<const uint8_t*>(receive_buffer_), kHeaderLength);

    DEBUG_PRINTF("content_size_=%u", content_size_);

    if (content_size_ != 0U)
    {
        network::tcp::Send(connection_handle_, reinterpret_cast<const uint8_t*>(content_), content_size_);
    }

    // Reset request state after reply is sent.
    status_ = http::Status::kUnknownError;
    request_method_ = http::RequestMethod::kUnknown;
    request_content_length_ = 0;
    request_data_length_ = 0;
    file_data_ = nullptr;
    firmwarefile_name_ = nullptr;

    DEBUG_EXIT();
}

http::Status HttpDeamonHandleRequest::ParseRequest()
{
    char* line_buffer = receive_buffer_;
    uint32_t line_index = 0;
    auto http_status = http::Status::kUnknownError;
    request_content_type_ = http::ContentTypes::kNotDefined;
    request_content_length_ = 0;
    request_data_length_ = 0;
    firmwarefile_name_ = nullptr;

    for (uint32_t i = 0; i < bytes_received_; i++)
    {
        if (receive_buffer_[i] == '\n')
        {
            assert(i > 0);
            receive_buffer_[i - 1] = '\0'; // Turn \r\n into \0

            if (line_index++ == 0)
            {
                http_status = ParseMethod(line_buffer);
            }
            else
            {
                if (line_buffer[0] == '\0')
                {
                    uint32_t header_end_index = i + 1;

                    request_data_length_ = static_cast<uint16_t>(bytes_received_ - header_end_index);

                    if (request_data_length_ > 0)
                    {
                        file_data_ = &receive_buffer_[header_end_index];
                        receive_buffer_[bytes_received_] = '\0';
                    }

                    return http::Status::kOk;
                }

                http_status = ParseHeaderField(line_buffer);
            }

            if (http_status != http::Status::kOk)
            {
                return http_status;
            }

            line_buffer = &receive_buffer_[i + 1];
        }
    }

    return http::Status::kOk;
}

http::Status HttpDeamonHandleRequest::ParseMethod(char* line)
{
    DEBUG_ENTRY();

    assert(line != nullptr);
    char* token;

    if ((token = strtok(line, " ")) == nullptr)
    {
        return http::Status::kMethodNotImplemented;
    }

    if (strcmp(token, "GET") == 0)
    {
        request_method_ = http::RequestMethod::kGet;
    }
    else if (strcmp(token, "POST") == 0)
    {
        request_method_ = http::RequestMethod::kPost;
    }
#if defined(CONFIG_HTTPD_ENABLE_DELETE)
    else if (strcmp(token, "DELETE") == 0)
    {
        request_method_ = http::RequestMethod::DELETE;
    }
#endif
    else
    {
        return http::Status::kMethodNotImplemented;
    }

    if ((token = strtok(nullptr, " ")) == nullptr)
    {
        return http::Status::kBadRequest;
    }

    uri_ = token;

    if ((token = strtok(nullptr, "/")) == nullptr || strcmp(token, "HTTP") != 0)
    {
        return http::Status::kBadRequest;
    }

    if ((token = strtok(nullptr, " \n")) == nullptr)
    {
        return http::Status::kBadRequest;
    }

    if (strcmp(token, "1.1") != 0)
    {
        return http::Status::kVersionNotSupported;
    }

    return http::Status::kOk;
}

static bool ParseUint32(const char* str, uint32_t& out)
{
    out = 0;
    if (str == nullptr) return false;

    while (*str == ' ') ++str; // Skip leading spaces

    if (*str == '\0') return false; // Empty string

    while (*str != '\0')
    {
        char c = *str++;
        if (c < '0' || c > '9') return false;
        uint32_t digit = static_cast<uint32_t>(c - '0');

        // Optional: Add overflow protection
        if (out > (UINT32_MAX - digit) / 10) return false;

        out = out * 10 + digit;
    }

    return true;
}

http::Status HttpDeamonHandleRequest::ParseHeaderField(char* line)
{
    char* token;

    assert(line != nullptr);

    if ((token = strtok(line, ":")) == nullptr)
    {
        return http::Status::kBadRequest;
    }

    DEBUG_PUTS(token);

    if (strcasecmp(token, "Content-Type") == 0)
    {
        if ((token = strtok(nullptr, " ;")) == nullptr)
        {
            return http::Status::kBadRequest;
        }
        if (memcmp(token, "application/", 12) == 0)
        {
            if (strcmp(&token[12], "json") == 0)
            {
                request_content_type_ = http::ContentTypes::kApplicationJson;
                return http::Status::kOk;
            }
#if defined(CONFIG_HTTPD_ENABLE_UPLOAD)
            if (strcmp(&token[12], "octet-stream") == 0)
            {
                request_content_type_ = http::ContentTypes::kApplicationOctetStream;
                return http::Status::kOk;
            }
#endif
        }
    }
    else if (strcasecmp(token, "Content-Length") == 0)
    {
        if ((token = strtok(nullptr, " ")) == nullptr)
        {
            return http::Status::kBadRequest;
        }

        if (!ParseUint32(token, request_content_length_))
        {
            return http::Status::kBadRequest;
        }

        if (request_content_length_ > network::tcp::kTcpDataMss)
        {
            return http::Status::kRequestEntityTooLarge;
        }

        return http::Status::kOk;
    }
    else if (strcasecmp(token, "If-None-Match") == 0)
    {
        if ((token = strtok(nullptr, ": \"")) == nullptr)
        {
            return http::Status::kBadRequest;
        }

        uint32_t etag;

        if (!ParseUint32(token, etag))
        {
            return http::Status::kBadRequest;
        }

        if (etag == _TIME_STAMP_)
        {
            return http::Status::kNotModified;
        }
        else
        {
            return http::Status::kOk;
        }
    }
#if defined(CONFIG_HTTPD_ENABLE_UPLOAD)
    else if (strcasecmp(token, "X-Upload-Size") == 0)
    {
        if ((token = strtok(nullptr, " ")) == nullptr)
        {
            return http::Status::kBadRequest;
        }

        if (!ParseUint32(token, upload_size_))
        {
            return http::Status::kBadRequest;
        }

        return http::Status::kOk;
    }
    else if (strcasecmp(token, "X-Upload-Name") == 0)
    {
        if ((token = strtok(nullptr, " ")) == nullptr)
        {
            return http::Status::kBadRequest;
        }

        strncpy(upload_filename_, token, sizeof(upload_filename_) - 1);
        upload_filename_[sizeof(upload_filename_) - 1] = '\0';

        return http::Status::kOk;
    }
#endif
    return http::Status::kOk;
}

inline uint8_t ParsePortIndex(const char* str)
{
    if (str[0] >= 'A' && str[0] <= 'F' && str[1] == '\0')
    {
        return static_cast<uint8_t>(str[0] - 'A');
    }
    return 0xFF;
}

// GET

namespace json::status
{
uint32_t Dmx(char*, uint32_t, uint32_t);
uint32_t RdmTod(char*, uint32_t, uint32_t);
} // namespace json::status

http::Status HttpDeamonHandleRequest::HandleGet()
{
    DEBUG_ENTRY();

    uint32_t length = 0;
    content_ = dynamic_content_;
    DEBUG_PUTS(uri_);

    if (memcmp(uri_, "/json/", 6) == 0)
    {
        request_content_type_ = http::ContentTypes::kApplicationJson;

        const auto* get = &uri_[6];
        DEBUG_PUTS(get);

#if !defined(CONFIG_HTTP_HTML_INDEX_ONLY)
        // Special handling: status/dmx?N
        if (memcmp(get, "status/dmx?", 11) == 0)
        {
#if (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
            const auto kPort = ParsePortIndex(&get[11]); // for dmx/status

            if (kPort != 0xFF)
            {
                length = json::status::Dmx(dynamic_content_, static_cast<uint32_t>(sizeof(dynamic_content_)), kPort);
            }
#endif // #if (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
        }
        // Special handling: rdm/tod?N
        else if (memcmp(get, "status/rdm/tod?", 15) == 0)
        {
#if defined(RDM_CONTROLLER)
            const auto kPort = ParsePortIndex(&get[15]); // for rdm/tod

            if (kPort != 0xFF)
            {
                length = json::status::RdmTod(dynamic_content_, static_cast<uint32_t>(sizeof(dynamic_content_)), kPort);
            }
#endif // #if defined(RDM_CONTROLLER)
        }
        else
#endif // #if !defined(CONFIG_HTTP_HTML_INDEX_ONLY)
        {
            const auto kIndex = json::GetFileIndex(get);
            DEBUG_PRINTF("kIndex=%d", kIndex);

            if (kIndex >= 0)
            {
                const auto& handler = json::kFileInfos[kIndex];
                if (handler.get != nullptr)
                {
                    length = (*(handler.get))(dynamic_content_, static_cast<uint32_t>(sizeof(dynamic_content_)));
                }
            }
            else
            {
                content_ = GetFileContent(&uri_[6], length, request_content_type_);
            }
        }
    }
    else
    {
        const auto kIndex = html::GetFileIndex(uri_);
        DEBUG_PRINTF("kIndex=%d", kIndex);

        if (kIndex >= 0)
        {
            const auto& handler = html::kHtmlInfos[kIndex];
            content_ = GetFileContent(handler.label, length, request_content_type_);
        }
        else
        {
            content_ = GetFileContent(&uri_[1], length, request_content_type_);
        }
    }

    if (length == 0)
    {
        DEBUG_EXIT();
        return http::Status::kNotFound;
    }

    content_size_ = length;

    DEBUG_EXIT();
    return http::Status::kOk;
}

http::Status HttpDeamonHandleRequest::HandlePost()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("bytes_received_=%d, request_data_length_=%u, request_content_length_=%u", bytes_received_, request_data_length_, request_content_length_);
    DEBUG_PUTS(uri_);

    if (request_content_type_ == http::ContentTypes::kApplicationJson)
    {
        content_size_ = 0;
        DEBUG_EXIT();
        return HandlePostJSON();
    }

#if defined(CONFIG_HTTPD_ENABLE_UPLOAD)
    if (memcmp(uri_, "/upload", 7) == 0)
    {
        return HandlePostUpload();
    }
#endif

    if (memcmp(uri_, "/action/command=reboot", 23) == 0)
    {
        network::tcp::Abort(connection_handle_);
        hal::Reboot();
    }

    DEBUG_EXIT();
    return http::Status::kInternalServerError;
}

http::Status HttpDeamonHandleRequest::HandlePostJSON()
{
    DEBUG_ENTRY();

    if (memcmp(uri_, "/json/", 6) == 0)
    {
        DEBUG_PUTS(uri_);
        const auto* get = &uri_[6];
        DEBUG_PUTS(get);

        const auto kIndex = json::GetFileIndex(get);

        DEBUG_PRINTF("kIndex=%d", kIndex);

        if (kIndex >= 0)
        {
            const auto& handler = json::kFileInfos[kIndex];
            if (handler.set != nullptr)
            {
                debug::Dump(file_data_, request_data_length_);

                if (file_data_ == nullptr)
                {
                    DEBUG_EXIT();
                    return http::Status::kInternalServerError;
                }

                (*(handler.set))(file_data_, request_data_length_);
                DEBUG_EXIT();
                return http::Status::kOk;
            }
        }
    }

    DEBUG_EXIT();
    return http::Status::kNotFound;
}

#if defined(CONFIG_HTTPD_ENABLE_UPLOAD)
static void ShowProgressSymbol()
{
    static constexpr char kProgressSymbols[] = {'/', '-', '\\', '|'};
    static uint32_t progress_symbols_index = 0;

    printf("%c\r", kProgressSymbols[progress_symbols_index++]);

    if (progress_symbols_index >= sizeof(kProgressSymbols))
    {
        progress_symbols_index = 0;
    }
}

http::Status HttpDeamonHandleRequest::HandlePostUpload()
{
    DEBUG_ENTRY();

    auto part_uri = &uri_[7];

    if (memcmp(part_uri, "_start", 7) == 0)
    {
        printf("Firmware: %s -> %u bytes\n", upload_filename_, upload_size_);

        if (strncmp(upload_filename_, firmware::kFileName, sizeof(upload_filename_)) != 0)
        {
            return http::Status::kBadRequest;
        }

        if ((upload_size_ >= 64) && (upload_size_ > (FIRMWARE_MAX_SIZE)))
        {
            return http::Status::kRequestEntityTooLarge;
        }

        if (!(FlashCodeInstall::Get()->Erase(upload_size_)))
        {
            puts("Erase failed.");
            DEBUG_EXIT();
            return http::Status::kInternalServerError;
        }

        content_size_ = static_cast<uint32_t>(snprintf(dynamic_content_, sizeof(dynamic_content_), "{\"status\":\"ok\"}"));
        content_ = dynamic_content_;
        request_content_type_ = http::ContentTypes::kApplicationJson;

        DEBUG_EXIT();
        return http::Status::kOk;
    }

    if (request_content_type_ == http::ContentTypes::kApplicationOctetStream)
    {
        if (part_uri[0] == 0)
        {
            ShowProgressSymbol();
            Display::Get()->Progress();

            if (!(FlashCodeInstall::Get()->WriteChunk(reinterpret_cast<uint8_t*>(file_data_), request_data_length_)))
            {
                puts("WriteChunk failed.");
                DEBUG_EXIT();
                return http::Status::kInternalServerError;
            }

            content_size_ = 0;

            DEBUG_EXIT();
            return http::Status::kOk;
        }
    }

    if (memcmp(part_uri, "_complete", 10) == 0)
    {
        putchar('\n');

        uint32_t write_count;
        if (!(FlashCodeInstall::Get()->WriteChunkComplete(write_count)))
        {
            puts("WriteChunkComplete failed.");
            DEBUG_EXIT();
            return http::Status::kInternalServerError;
        }

        printf("Written bytes -> %u [%s]\n", write_count, write_count == upload_size_ ? "Ok" : "Wrong");

        content_size_ = static_cast<uint32_t>(snprintf(dynamic_content_, sizeof(dynamic_content_), "{\"status\":\"ok\"}"));
        content_ = dynamic_content_;
        request_content_type_ = http::ContentTypes::kApplicationJson;
        upload_size_ = 0;
        upload_filename_[0] = '\0';
        DEBUG_EXIT();
        return http::Status::kOk;
    }

    return http::Status::kBadRequest;
    DEBUG_EXIT();
}
#endif

#if defined(CONFIG_HTTPD_ENABLE_DELETE)
http::Status HttpDeamonHandleRequest::HandleDelete()
{
    DEBUG_PRINTF("bytes_received_=%d, request_data_length_=%u, request_content_length_=%u", bytes_received_, request_data_length_, request_content_length_);
    DEBUG_EXIT();
    return http::Status::kInternalServerError;
}
#endif