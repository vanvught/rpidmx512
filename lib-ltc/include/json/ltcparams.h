/**
 * @file ltcparams.h
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

#ifndef JSON_LTCPARAMS_H_
#define JSON_LTCPARAMS_H_

#include "configurationstore.h"
#include "json/ltcparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "ltc.h"

namespace json
{
class LtcParams : public JsonParamsBase<LtcParams>
{
   public:
    struct RgbLedType
    {
        static constexpr uint32_t kWS28Xx = (1U << 0);
        static constexpr uint32_t kRgbpanel = (1U << 1);
    };

    LtcParams();

    LtcParams(const LtcParams&) = delete;
    LtcParams& operator=(const LtcParams&) = delete;

    LtcParams(LtcParams&&) = delete;
    LtcParams& operator=(LtcParams&&) = delete;

    void Load() { JsonParamsBase::Load(LtcParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set(struct ltc::TimeCode* start_time_code, struct ltc::TimeCode* stop_time_code);

   protected:
    void Dump();

   private:
    static void SetSource(const char* val, uint32_t len);
    // Output options
    static void SetDisableDisplayOled(const char* val, uint32_t len);
    static void SetDisableMax7219(const char* val, uint32_t len);
    static void SetDisableMidi(const char* val, uint32_t len);
    static void SetDisableArtnet(const char* val, uint32_t len);
    static void SetDisableLtc(const char* val, uint32_t len);
    static void SetDisableRtpmidi(const char* val, uint32_t len);
    static void SetDisableEtc(const char* val, uint32_t len);
    // System clock / RTC
    static void SetShowSystime(const char* val, uint32_t len);
    static void SetDisableTimesync(const char* val, uint32_t len);
    // source=systime
    static void SetAutoStart(const char* val, uint32_t len);
    static void SetGpsStart(const char* val, uint32_t len);
    static void SetUtcOffset(const char* val, uint32_t len);
    // source=internal
    static void SetFps(const char* val, uint32_t len);
    static void SetStartFrame(const char* val, uint32_t len);
    static void SetStartSecond(const char* val, uint32_t len);
    static void SetStartMinute(const char* val, uint32_t len);
    static void SetStartHour(const char* val, uint32_t len);
    static void SetIgnoreStart(const char* val, uint32_t len);
    static void SetStopFrame(const char* val, uint32_t len);
    static void SetStopSecond(const char* val, uint32_t len);
    static void SetStopMinute(const char* val, uint32_t len);
    static void SetStopHour(const char* val, uint32_t len);
    static void SetIgnoreStop(const char* val, uint32_t len);
    // Art-Net
    static void SetTimecodeIp(const char* val, uint32_t len);
    // LTC output
    static void SetLtcVolume(const char* val, uint32_t len);
    // WS28xx Display
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    static void SetWS28xxEnable(const char* val, uint32_t len);
#endif
#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
    // RGB panel
    static void SetRgbpanelEnable(const char* val, uint32_t len);
#endif

    static constexpr json::Key kLtcKeys[] = {
        // source
        MakeKey(SetSource, LtcParamsConst::kSource), //
        // Output options
        MakeKey(SetDisableDisplayOled, LtcParamsConst::kDisableDisplayOled), //
        MakeKey(SetDisableMax7219, LtcParamsConst::kDisableMax7219),         //
        MakeKey(SetDisableMidi, LtcParamsConst::kDisableMidi),               //
        MakeKey(SetDisableArtnet, LtcParamsConst::kDisableArtnet),           //
        MakeKey(SetDisableLtc, LtcParamsConst::kDisableLtc),                 //
        MakeKey(SetDisableRtpmidi, LtcParamsConst::kDisableRtpmidi),         //
        MakeKey(SetDisableEtc, LtcParamsConst::kDisableEtc),                 //
        // System clock / RTC
        MakeKey(SetShowSystime, LtcParamsConst::kShowSystime),         //
        MakeKey(SetDisableTimesync, LtcParamsConst::kDisableTimesync), //
        // source=systime
        MakeKey(SetAutoStart, LtcParamsConst::kAutoStart), //
        MakeKey(SetGpsStart, LtcParamsConst::kGpsStart),   //
        MakeKey(SetUtcOffset, LtcParamsConst::kUtcOffset), //
        // LTC output
        MakeKey(SetLtcVolume, LtcParamsConst::kLtcVolume), //
        // source=internal
        MakeKey(SetFps, LtcParamsConst::kFps), //
        MakeKey(SetStartFrame, LtcParamsConst::kStartFrame), //
        MakeKey(SetStartSecond, LtcParamsConst::kStartSecond), //
        MakeKey(SetStartMinute, LtcParamsConst::kStartMinute), //
        MakeKey(SetStartHour, LtcParamsConst::kStartHour), //
        MakeKey(SetIgnoreStart, LtcParamsConst::kIgnoreStart), //
        MakeKey(SetStopFrame, LtcParamsConst::kStopFrame), //
        MakeKey(SetStopSecond, LtcParamsConst::kStopSecond), //
        MakeKey(SetStopMinute, LtcParamsConst::kStopMinute), //
        MakeKey(SetStopHour, LtcParamsConst::kStopHour), //
        MakeKey(SetIgnoreStop, LtcParamsConst::kIgnoreStop), //
        // Art-Net
        MakeKey(SetTimecodeIp, LtcParamsConst::kTimecodeIp), //
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
        // WS28xx Display
        MakeKey(SetWS28xxEnable, LtcParamsConst::kWS28xxEnable), //
#endif
#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
        // RGB panel
        MakeKey(SetRgbpanelEnable, LtcParamsConst::kRgbpanelEnable), //
#endif
    };

    inline static common::store::Ltc store_ltc;

    friend class JsonParamsBase<LtcParams>;
};
} // namespace json

#endif  // JSON_LTCPARAMS_H_
