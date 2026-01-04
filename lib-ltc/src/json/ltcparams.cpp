/**
 * @file ltcparams.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "json/ltcparams.h"
#include "json/ltcparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "net/ip4_helpers.h"
#include "configstore.h"
#include "configurationstore.h"
#include "ltc.h"
#include "common/utils/utils_enum.h"
#include "common/utils/utils_flags.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

using common::store::ltc::Flags;

namespace json
{
LtcParams::LtcParams()
{
    ConfigStore::Instance().Copy(&store_ltc, &ConfigurationStore::ltc);
}

void LtcParams::SetSource(const char* val, uint32_t len)
{
    if (len >= ltc::kSourceMaxNameLength)
    {
        return;
    }

    char source[ltc::kSourceMaxNameLength];
    memcpy(source, val, len);
    source[len] = '\0';

    store_ltc.source = common::ToValue(ltc::GetSourceType(source));
}

template <ltc::Destination::Output output> static uint8_t HandleDisabledOutput(uint8_t disabled_outputs, char val)
{
    if (val == '0')
    {
        return disabled_outputs &= ~common::ToValue(output);
    }

    return disabled_outputs |= common::ToValue(output);
}

void LtcParams::SetDisableDisplayOled(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::DISPLAY_OLED>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableMax7219(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::MAX7219>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableMidi(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::MIDI>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableArtnet(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::ARTNET>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableLtc(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::LTC>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableRtpmidi(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::RTPMIDI>(store_ltc.disabled_outputs, val[0]);
}

void LtcParams::SetDisableEtc(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.disabled_outputs = HandleDisabledOutput<ltc::Destination::Output::ETC>(store_ltc.disabled_outputs, val[0]);
}

// System clock / RTC
void LtcParams::SetShowSystime(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kShowSystime, val[0] != '0');
}
void LtcParams::SetDisableTimesync(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kTimeSyncDisabled, val[0] != '0');
}

// source=systime
void LtcParams::SetAutoStart(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kAutoStart, val[0] != '0');
}

void LtcParams::SetGpsStart(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kGpsStart, val[0] != '0');
}

void LtcParams::SetUtcOffset(const char* val, uint32_t len)
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
            store_ltc.utc_offset = utc_offset;
        }
    }
    else
    {
        DEBUG_PUTS("Parse ERROR");
    }
}

// source=internal
void LtcParams::SetFps(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kFps = net::ParseIpString(val, len);
    store_ltc.fps = common::ToValue(ltc::get_type(kFps));
}

void LtcParams::SetStartFrame(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kFrame = net::ParseIpString(val, len);
    if (kFrame <= 30)
    {
        store_ltc.start_frame = kFrame;
    }
}

void LtcParams::SetStartSecond(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kSecond = net::ParseIpString(val, len);
    if (kSecond <= 59)
    {
        store_ltc.start_second = kSecond;
    }
}

void LtcParams::SetStartMinute(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kMinute = net::ParseIpString(val, len);
    if (kMinute <= 59)
    {
        store_ltc.start_minute = kMinute;
    }
}

void LtcParams::SetStartHour(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kHour = net::ParseIpString(val, len);
    if (kHour <= 59)
    {
        store_ltc.start_hour = kHour;
    }
}

void LtcParams::SetIgnoreStart(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kIgnoreStart, val[0] != '0');
}

void LtcParams::SetStopFrame(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kFrame = net::ParseIpString(val, len);
    if (kFrame <= 30)
    {
        store_ltc.stop_frame = kFrame;
    }
}

void LtcParams::SetStopSecond(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kSecond = net::ParseIpString(val, len);
    if (kSecond <= 59)
    {
        store_ltc.stop_second = kSecond;
    }
}

void LtcParams::SetStopMinute(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kMinute = net::ParseIpString(val, len);
    if (kMinute <= 59)
    {
        store_ltc.stop_minute = kMinute;
    }
}

void LtcParams::SetStopHour(const char* val, uint32_t len)
{
    if (len != 2) return;

    const auto kHour = net::ParseIpString(val, len);
    if (kHour <= 59)
    {
        store_ltc.stop_hour = kHour;
    }
}

void LtcParams::SetIgnoreStop(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kIgnoreStop, val[0] != '0');
}

// Art-Net
void LtcParams::SetTimecodeIp(const char* val, uint32_t len)
{
    if (len == 0)
    {
        store_ltc.time_code_ip = net::GetBroadcastIp();
    }
    store_ltc.time_code_ip = net::ParseIpString(val, len);
}

// LTC output
void LtcParams::SetLtcVolume(const char* val, uint32_t len)
{
    if ((len == 1) || (len == 2))
    {
        const auto kValue = ParseValue<uint8_t>(val, len);
        if ((kValue > 1) && (kValue < 32))
        {
            store_ltc.volume = kValue;
        }
    }
}

// OSC

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
// WS28xx Display
void LtcParams::SetWS28xxEnable(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kWS28xxEnable, val[0] != '0');
}
#endif

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
void LtcParams::SetRgbpanelEnable(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltc.flags = common::SetFlagValue(store_ltc.flags, Flags::Flag::kRgbpanelEnable, val[0] != '0');
}
#endif

void LtcParams::Store(const char* buffer, uint32_t buffer_size)
{
    //    store_ltc.fps = 25;
    //    store_ltc.stop_frame = static_cast<uint8_t>(store_ltc.fps - 1);
    //    store_ltc.stop_second = 59;
    //    store_ltc.stop_minute = 59;
    //    store_ltc.stop_hour = 23;
    //    store_ltc.osc_port = 8000;
    //    store_ltc.skip_seconds = 5;

    ParseJsonWithTable(buffer, buffer_size, kLtcKeys);
    ConfigStore::Instance().Store(&store_ltc, &ConfigurationStore::ltc);

#ifndef NDEBUG
    Dump();
#endif
}

void LtcParams::Set(struct ltc::TimeCode* start_time_code, struct ltc::TimeCode* stop_time_code)
{
    const auto kFlags = store_ltc.flags;

    ltc::g_nDisabledOutputs = store_ltc.disabled_outputs;
    ltc::Destination::SetDisabled(ltc::Destination::Output::NTP_SERVER, !common::IsFlagSet(kFlags, common::store::ltc::Flags::Flag::kNtpEnable));
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    ltc::Destination::SetDisabled(ltc::Destination::Output::WS28XX, (store_ltc.rgb_led_type != LtcParams::RgbLedType::kWS28Xx));
#else
    ltc::Destination::SetDisabled(ltc::Destination::Output::WS28XX);
#endif
#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
    ltc::Destination::SetDisabled(ltc::Destination::Output::RGBPANEL, (store_ltc.rgb_led_type != LtcParams::RgbLedType::kRgbpanel));
#else
    ltc::Destination::SetDisabled(ltc::Destination::Output::RGBPANEL);
#endif

    assert(start_time_code != nullptr);

    start_time_code->frames = store_ltc.start_frame;
    start_time_code->seconds = store_ltc.start_second;
    start_time_code->minutes = store_ltc.start_minute;
    start_time_code->hours = store_ltc.start_hour;
    start_time_code->type = common::ToValue(ltc::g_Type);

    assert(stop_time_code != nullptr);

    stop_time_code->frames = store_ltc.stop_frame;
    stop_time_code->seconds = store_ltc.stop_second;
    stop_time_code->minutes = store_ltc.stop_minute;
    stop_time_code->hours = store_ltc.stop_hour;

    stop_time_code->type = common::ToValue(ltc::g_Type);

#ifndef NDEBUG
    Dump();
#endif
}

void LtcParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcParamsConst::kFileName);
    printf(" %s=%s\n", LtcParamsConst::kSource.name, ltc::GetSourceType(common::FromValue<ltc::Source>(store_ltc.source)));
}
} // namespace json