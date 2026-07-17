/**
 * @file networkparamsconst.h
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

#ifndef JSON_NETWORKPARAMSCONST_H_
#define JSON_NETWORKPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct NetworkParamsConst {
    static constexpr char kFileName[] = "network.json";

    static constexpr auto kSecondaryIp = json::MakeSimpleKey("secondary_ip");
    static constexpr auto kUseStaticIp = json::MakeSimpleKey("use_static_ip");
    static constexpr auto kIpAddress = json::MakeSimpleKey("ip_address");
    static constexpr auto kNetMask = json::MakeSimpleKey("net_mask");
    static constexpr auto kDefaultGateway = json::MakeSimpleKey("default_gateway");
    static constexpr auto kHostname = json::MakeSimpleKey("hostname");
    static constexpr auto kNtpServer = json::MakeSimpleKey("ntp_server");
};
} // namespace json

#endif // JSON_NETWORKPARAMSCONST_H_
