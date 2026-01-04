/**
 * @file http.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HTTP_HTTP_H_
#define HTTP_HTTP_H_

namespace http
{
enum class Status
{
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    REQUEST_TIMEOUT = 408,
    REQUEST_ENTITY_TOO_LARGE = 413,
    REQUEST_URI_TOO_LONG = 414,
    INTERNAL_SERVER_ERROR = 500,
    METHOD_NOT_IMPLEMENTED = 501,
    VERSION_NOT_SUPPORTED = 505,
    UNKNOWN_ERROR = 520
};
enum class RequestMethod
{
    GET,
    POST,
    DELETE,
    UNKNOWN
};

enum class contentTypes
{
    TEXT_HTML,
    TEXT_CSS,
    TEXT_JS,
    APPLICATION_JSON,
    APPLICATION_OCTET_STREAM,
    NOT_DEFINED
};
} // namespace http

#endif  // HTTP_HTTP_H_
