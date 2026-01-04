/**
 * @file json_config_ltc.cpp
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

#include "json/json_format_helpers.h"
#include "json/json_helpers.h"
#include "json/ltcparamsconst.h"
#include "json/ltcparams.h"
#include "ltcdisplayrgb.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_enum.h"
#include "common/utils/utils_flags.h"
#include "net/ip4_helpers.h"
#include "ltc.h"

using common::store::ltc::Flags;

namespace json::config
{
template <ltc::Destination::Output output> bool IsDisabledOutputSet(uint8_t disabled_outputs)
{
    return (disabled_outputs & common::ToValue(output)) == common::ToValue(output);
}

uint32_t GetLtc(char* buffer, uint32_t length)
{
    const auto kFlags = ConfigStore::Instance().LtcGet(&common::store::Ltc::flags);
    // source
    const auto kSource = ConfigStore::Instance().LtcGet(&common::store::Ltc::source);
    // Disabled outputs
    const auto kDisabledOutputs = ConfigStore::Instance().LtcGet(&common::store::Ltc::disabled_outputs);
    // source=systime
    const auto kUtcOffset = ConfigStore::Instance().LtcGet(&common::store::Ltc::utc_offset);
    // source=internal
    const auto kFps = ConfigStore::Instance().LtcGet(&common::store::Ltc::fps);
    const auto kStartFrame = ConfigStore::Instance().LtcGet(&common::store::Ltc::start_frame);
    const auto kStartSecond = ConfigStore::Instance().LtcGet(&common::store::Ltc::start_second);
    const auto kStartMinute = ConfigStore::Instance().LtcGet(&common::store::Ltc::start_minute);
    const auto kStartHour = ConfigStore::Instance().LtcGet(&common::store::Ltc::start_hour);
    const auto kStopSecond = ConfigStore::Instance().LtcGet(&common::store::Ltc::stop_second);
    const auto kStopMinute = ConfigStore::Instance().LtcGet(&common::store::Ltc::stop_minute);
    const auto kStopHour = ConfigStore::Instance().LtcGet(&common::store::Ltc::stop_hour);
    const auto kStopFrame = ConfigStore::Instance().LtcGet(&common::store::Ltc::stop_frame);
    const auto kSkipSeconds = ConfigStore::Instance().LtcGet(&common::store::Ltc::skip_seconds);
    // Art-Net
    const auto kTimecodeIp = ConfigStore::Instance().LtcGet(&common::store::Ltc::time_code_ip);
    // LTC output
    const auto kVolume = ConfigStore::Instance().LtcGet(&common::store::Ltc::volume);
    // NTP
    const auto kNtpYear = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_year);
    const auto kNtpMonth = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_month);
    const auto kNtpDay = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_day);
    //
    const auto kOscPort = ConfigStore::Instance().LtcGet(&common::store::Ltc::osc_port);
    //
    const auto kRgbLedType = ConfigStore::Instance().LtcGet(&common::store::Ltc::rgb_led_type);
    
    return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    // source
	    doc[LtcParamsConst::kSource.name] = ltc::GetSourceType(common::FromValue<ltc::Source>(kSource));
	    // Disabled outputs
	    doc[LtcParamsConst::kDisableDisplayOled.name] = IsDisabledOutputSet<ltc::Destination::Output::DISPLAY_OLED>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableMax7219.name] = IsDisabledOutputSet<ltc::Destination::Output::MAX7219>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableLtc.name] = IsDisabledOutputSet<ltc::Destination::Output::LTC>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableMidi.name] = IsDisabledOutputSet<ltc::Destination::Output::MIDI>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableArtnet.name] = IsDisabledOutputSet<ltc::Destination::Output::ARTNET>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableRtpmidi.name] = IsDisabledOutputSet<ltc::Destination::Output::RTPMIDI>(kDisabledOutputs);
	    doc[LtcParamsConst::kDisableEtc.name] = IsDisabledOutputSet<ltc::Destination::Output::ETC>(kDisabledOutputs);
	    // System clock / RTC
	    doc[LtcParamsConst::kShowSystime.name] = common::IsFlagSet(kFlags, Flags::Flag::kShowSystime);
	    doc[LtcParamsConst::kDisableTimesync.name] = common::IsFlagSet(kFlags, Flags::Flag::kTimeSyncDisabled);
	    // source=systime
	    doc[LtcParamsConst::kAutoStart.name] = common::IsFlagSet(kFlags, Flags::Flag::kAutoStart);
	    doc[LtcParamsConst::kGpsStart.name] = common::IsFlagSet(kFlags, Flags::Flag::kGpsStart);
	    int32_t hours = 0;
    	uint32_t minutes = 0;
    	hal::utc::SplitOffset(kUtcOffset, hours, minutes);
    	char offset[format::kOffsetBufferSize];
	    doc[LtcParamsConst::kUtcOffset.name] = format::FormatUtcOffset(hours, minutes, offset);
	    // source=internal
	    doc[LtcParamsConst::kFps.name] = kFps;
	    doc[LtcParamsConst::kStartFrame.name] = kStartFrame;
	    doc[LtcParamsConst::kStartSecond.name] = kStartSecond;
	    doc[LtcParamsConst::kStartMinute.name] = kStartMinute;
	    doc[LtcParamsConst::kStartHour.name] = kStartHour;
	    doc[LtcParamsConst::kIgnoreStart.name] = common::IsFlagSet(kFlags, Flags::Flag::kIgnoreStart);
	    doc[LtcParamsConst::kStopFrame.name] = kStopFrame;
	    doc[LtcParamsConst::kStopSecond.name] = kStopSecond;
	    doc[LtcParamsConst::kStopMinute.name] = kStopMinute;
	    doc[LtcParamsConst::kStopHour.name] = kStopHour;
	    doc[LtcParamsConst::kIgnoreStop.name] = common::IsFlagSet(kFlags, Flags::Flag::kIgnoreStop);
	    doc[LtcParamsConst::kAltFunction.name] = common::IsFlagSet(kFlags, Flags::Flag::kIsAltFuntion);
	    doc[LtcParamsConst::kSkipSeconds.name] = kSkipSeconds;
	    doc[LtcParamsConst::kSkipFree.name] = common::IsFlagSet(kFlags, Flags::Flag::kSkipFree);
	    // Art-Net
	    char ip[net::kIpBufferSize];
	    doc[LtcParamsConst::kTimecodeIp.name] = net::FormatIp(kTimecodeIp, ip);
	    // LTC output
	    doc[LtcParamsConst::kLtcVolume.name] = kVolume;
	    // NTP
	    doc[LtcParamsConst::kNtpEnable.name] = common::IsFlagSet(kFlags, Flags::Flag::kNtpEnable);
	    doc[LtcParamsConst::kNtpYear.name] = kNtpYear;
	    doc[LtcParamsConst::kNtpMonth.name] = kNtpMonth;
	    doc[LtcParamsConst::kNtpDay.name] = kNtpDay;
	    // OSC
	    doc[LtcParamsConst::kOscEnable.name] = common::IsFlagSet(kFlags, Flags::Flag::kOscEnabled);
	    doc[LtcParamsConst::kOscPort.name] = kOscPort;
	    // WS28xx Display
	    doc[LtcParamsConst::kWS28xxEnable.name] = (kRgbLedType == common::ToValue(ltc::display::rgb::Type::kWS28Xx));
#if !defined (CONFIG_LTC_DISABLE_RGB_PANEL)
	    doc[LtcParamsConst::kRgbpanelEnable.name] = (kRgbLedType == common::ToValue(ltc::display::rgb::Type::kRgbpanel));
#endif
    });
}

void SetLtc(const char* buffer, uint32_t buffer_size)
{
    ::json::LtcParams ltc_params;
    ltc_params.Store(buffer, buffer_size);
}
} // namespace json::config
