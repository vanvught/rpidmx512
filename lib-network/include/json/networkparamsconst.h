/**
 * @file networkparamsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

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
