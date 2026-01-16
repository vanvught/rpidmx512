/**
 * @file dmxsendparams.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */


#ifdef DEBUG_DMXSENDPARAMS
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>

#include "json/dmxsendparams.h"
#include "json/dmxsendparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "dmxconst.h"
#include "configstore.h"
#include "dmx.h"

namespace json
{
using dmx::kChannelsMax;

static constexpr uint8_t RounddownSlots(uint16_t n)
{
    return static_cast<uint8_t>((n / 2U) - 1);
}

static constexpr uint16_t RoundupSlots(uint8_t n)
{
    return static_cast<uint16_t>((n + 1U) * 2U);
}

DmxSendParams::DmxSendParams()
{
    ConfigStore::Instance().Copy(&store_dmx_send, &ConfigurationStore::dmx_send);
}

void DmxSendParams::SetBreakTime(const char* val, uint32_t len)
{
    ParseAndApply<uint16_t>(val, len, [](uint16_t v) {
        store_dmx_send.break_time = v > dmx::transmit::kBreakTimeMin ? v : dmx::transmit::kBreakTimeMin;
    });
}

void DmxSendParams::SetMabTime(const char* val, uint32_t len)
{
    ParseAndApply<uint16_t>(val, len, [](uint16_t v) {
        store_dmx_send.mab_time = v > dmx::transmit::kMabTimeMin ? v : dmx::transmit::kMabTimeMin;
    });
}

void DmxSendParams::SetRefreshRate(const char* val, uint32_t len)
{
    ParseAndApply<uint16_t>(val, len, [](uint16_t v) {
        store_dmx_send.refresh_rate = v;
    });
}

void DmxSendParams::SetSlotsCount(const char* val, uint32_t len)
{
    ParseAndApply<uint16_t>(val, len, [](uint16_t v) {
        if (v >= 2 && v < dmx::kChannelsMax) {
            store_dmx_send.slots_count = RounddownSlots(v);
        } else {
            store_dmx_send.slots_count = RounddownSlots(dmx::kChannelsMax);
        }
    });
}


void DmxSendParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kDmxSendKeys);
    ConfigStore::Instance().Store(&store_dmx_send, &ConfigurationStore::dmx_send);
}

void DmxSendParams::Set()
{
    auto& dmx = *Dmx::Get();

    dmx.SetDmxBreakTime(store_dmx_send.break_time);
    dmx.SetDmxMabTime(store_dmx_send.mab_time);

    uint32_t period = 0;
    if (store_dmx_send.refresh_rate != 0)
    {
        period = 1000000U / store_dmx_send.refresh_rate;
    }
    dmx.SetDmxPeriodTime(period);

    if (store_dmx_send.slots_count != 0)
    {
        dmx.SetDmxSlots(RoundupSlots(store_dmx_send.slots_count));
    }

#ifndef NDEBUG
    Dump();
#endif
}

void DmxSendParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxSendParamsConst::kFileName);
    printf(" %s=%u\n", DmxSendParamsConst::kBreakTime.name, store_dmx_send.break_time);
    printf(" %s=%u\n", DmxSendParamsConst::kMabTime.name, store_dmx_send.mab_time);
    printf(" %s=%u\n", DmxSendParamsConst::kRefreshRate.name, store_dmx_send.refresh_rate);
    printf(" %s=%u [%u]\n", DmxSendParamsConst::kSlotsCount.name, RoundupSlots(store_dmx_send.slots_count), store_dmx_send.slots_count);
}

} // namespace json