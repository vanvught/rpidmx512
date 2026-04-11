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

namespace json
{
struct NetworkParamsConst
{
    static constexpr char kFileName[] = "network.json";

	static constexpr json::SimpleKey kSecondaryIp {
	    "secondary_ip",
	    12,
	    Fnv1a32("secondary_ip", 12)
	};
	
	static constexpr json::SimpleKey kUseStaticIp {
	    "use_static_ip",
	    13,
	    Fnv1a32("use_static_ip", 13)
	};
	
	static constexpr json::SimpleKey kIpAddress {
	    "ip_address",
	    10,
	    Fnv1a32("ip_address", 10)
	};
	
	static constexpr json::SimpleKey kNetMask {
	    "net_mask",
	    8,
	    Fnv1a32("net_mask", 8)
	};
	
	static constexpr json::SimpleKey kDefaultGateway {
	    "default_gateway",
	    15,
	    Fnv1a32("default_gateway", 15)
	};
	
	static constexpr json::SimpleKey kHostname {
	    "hostname",
	    8,
	    Fnv1a32("hostname", 8)
	};
	
	static constexpr json::SimpleKey kNtpServer {
	    "ntp_server",
	    10,
	    Fnv1a32("ntp_server", 10)
	};
};
} // namespace json

#endif  // JSON_NETWORKPARAMSCONST_H_
