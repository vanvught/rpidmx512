/**
 * @file showfileparamsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_SHOWFILEPARAMSCONST_H_
#define JSON_SHOWFILEPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct ShowFileParamsConst
{
    static constexpr char kFileName[] = "showfile.json";
    
	static constexpr json::SimpleKey kShow {
	    "show",
	    4,
	    Fnv1a32("show", 4)
	};
	
	static constexpr json::SimpleKey kOptionAutoPlay {
	    "auto_play",
	    9,
	    Fnv1a32("auto_play", 9)
	};
	
	static constexpr json::SimpleKey kOptionLoop {
	    "loop",
	    4,
	    Fnv1a32("loop", 4)
	};
	
	static constexpr json::SimpleKey kOptionSacnSyncUniverse {
	    "sync_universe",
	    13,
	    Fnv1a32("sync_universe", 13)
	};
	
	static constexpr json::SimpleKey kOptionArtnetDisableUnicast {
	    "disable_unicast",
	    15,
	    Fnv1a32("disable_unicast", 15)
	};
};
} // namespace json

#endif  // JSON_SHOWFILEPARAMSCONST_H_
