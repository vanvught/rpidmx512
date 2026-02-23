/**
 * @file http_status_messages.h
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

#ifndef HTTP_HTTP_STATUS_MESSAGES_H_
#define HTTP_HTTP_STATUS_MESSAGES_H_

namespace httpd {
static constexpr const char STATUS_400[] = "{\"code\":400,\"status\":\"Bad Request\",\"message\":\"%s\"}\n";
static constexpr auto STATUS_400_LENGTH = sizeof(STATUS_400) - 1;
static constexpr const char STATUS_404[] = "{\"code\":404,\"status\":\"Not found\",\"message\":\"%s\"}\n";
static constexpr auto STATUS_404_LENGTH = sizeof(STATUS_404) - 1;
}  // namespace httpd

#endif  // HTTP_HTTP_STATUS_MESSAGES_H_
