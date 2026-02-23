/**
 * @file dmxmonitorparamsconst.h
 */
 /* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXMONITORPARAMSCONST_H_
#define JSON_DMXMONITORPARAMSCONST_H_

 #include "json/json_key.h"

namespace json
{
struct DmxMonitorParamsConst
{
    static constexpr char kFileName[] = "monitor.json";
    
    inline static constexpr json::SimpleKey kDmxStartAddress {
	    "dmx_start_address",
	    17,
	    Fnv1a32("dmx_start_address", 17)
	};
    
	inline static constexpr json::SimpleKey kDmxMaxChannels {
	    "dmx_max_channels",
	    16,
	    Fnv1a32("dmx_max_channels", 16)
	};
	
	inline static constexpr json::SimpleKey kFormat {
	    "format",
	    6,
	    Fnv1a32("format", 6)
	};
};
} // namespace json

#endif  // JSON_DMXMONITORPARAMSCONST_H_
