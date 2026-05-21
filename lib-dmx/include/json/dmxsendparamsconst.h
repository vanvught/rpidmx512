/**
 * @file dmxsendparamsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXSENDPARAMSCONST_H_
#define JSON_DMXSENDPARAMSCONST_H_

#include "json/json_key.h"
#include "common/utils/utils_hash.h"

namespace json {
struct DmxSendParamsConst {
    static constexpr char kFileName[] = "dmxsend.json";

    static constexpr SimpleKey kBreakTime{"break_time", 10, Fnv1a32("break_time", 10)};
    static constexpr SimpleKey kMabTime{"mab_time", 8, Fnv1a32("mab_time", 8)};
    static constexpr SimpleKey kRefreshRate{"refresh_rate", 12, Fnv1a32("refresh_rate", 12)};
    static constexpr SimpleKey kSlotsCount{"slots_count", 11, Fnv1a32("slots_count", 11)};
};
} // namespace json

#endif // JSON_DMXSENDPARAMSCONST_H_
