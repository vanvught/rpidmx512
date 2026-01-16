/**
 * @file globalparams.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifdef DEBUG_GLOBALPARAMS
#undef NDEBUG
#endif

#include <cstdio>

#include "json/globalparams.h"
#include "json/globalparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "utc.h"
#include "global.h"
 #include "firmware/debug/debug_debug.h"

namespace json
{

GlobalParams::GlobalParams()
{
    ConfigStore::Instance().Copy(&store_global, &ConfigurationStore::global);
}

void GlobalParams::SetUtcOffset(const char* val, uint32_t len)
{
    int32_t hours;
    uint32_t minutes;

    if (hal::utc::ParseOffset(val, len, hours, minutes))
    {
        DEBUG_PUTS("Parse OK");

        int32_t utc_offset;

        if (hal::utc::ValidateOffset(hours, minutes, utc_offset))
        {
            DEBUG_PUTS("Validate OK");
            store_global.utc_offset = utc_offset;
        }
    }
    else
    {
        DEBUG_PUTS("Parse ERROR");
    }
}

void GlobalParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kGlobalKeys);
    ConfigStore::Instance().Store(&store_global, &ConfigurationStore::global);

#ifndef NDEBUG
    Dump();
#endif
}

void GlobalParams::Set()
{
    int32_t hours;
    uint32_t minutes;

    hal::utc::SplitOffset(store_global.utc_offset, hours, minutes);
    Global::Instance().SetUtcOffsetIfValid(hours, minutes);

#ifndef NDEBUG
    Dump();
#endif
}

void GlobalParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::GlobalParamsConst::kFileName);

    int32_t hours;
    uint32_t minutes;
    hal::utc::SplitOffset(store_global.utc_offset, hours, minutes);
    printf("  %s=%d:%u [%d]\n", GlobalParamsConst::kUtcOffset.name, hours, minutes, store_global.utc_offset);
}
} // namespace json