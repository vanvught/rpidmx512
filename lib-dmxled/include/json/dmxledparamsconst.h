/**
 * @file dmxledparamsconst.h
 */

#ifndef JSON_DMXLEDPARAMSCONST_H_
#define JSON_DMXLEDPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct DmxLedParamsConst
{
    static constexpr char kFileName[] = "dmxled.json";
    
    inline static constexpr json::SimpleKey kType {
	    "type",
	    4,
	    Fnv1a32("type", 4)
	};
	
	inline static constexpr json::SimpleKey kMap {
	    "map",
	    3,
	    Fnv1a32("map", 3)
	};
	
	inline static constexpr json::SimpleKey kCount {
	    "count",
	    5,
	    Fnv1a32("count", 5)
	};
	
	inline static constexpr json::SimpleKey kGroupingCount {
	    "group_count",
	   11,
	    Fnv1a32("group_count", 11)
	};
	
	inline static constexpr json::SimpleKey kT0H {
	    "t0h",
	   3,
	    Fnv1a32("t0h", 3)
	};
	
	inline static constexpr json::SimpleKey kT1H {
	    "t1h",
	   3,
	    Fnv1a32("t1h", 3)
	};
	
	inline static constexpr json::SimpleKey kActiveOutputPorts {
	    "active_out",
	   10,
	    Fnv1a32("active_out", 10)
	};
	
	inline static constexpr json::SimpleKey kTestPattern {
	    "test_pattern",
	   12,
	    Fnv1a32("test_pattern", 12)
	};

	inline static constexpr json::SimpleKey kSpiSpeedHz {
	    "clock_speed_hz",
	   14,
	    Fnv1a32("clock_speed_hz", 14)
	};	
	
	inline static constexpr json::SimpleKey kGlobalBrightness {
	    "global_brightness",
	   17,
	    Fnv1a32("global_brightness", 17)
	};	
	
	
	inline static constexpr json::SimpleKey kGammaCorrection {
	    "gamma_correction",
	   16,
	    Fnv1a32("gamma_correction", 16)
	};	
	
	inline static constexpr json::SimpleKey kGammaValue {
	    "gamma_value",
	   11,
	    Fnv1a32("gamma_value", 11)
	};
};
} // namespace json

#endif  // JSON_DMXLEDPARAMSCONST_H_
