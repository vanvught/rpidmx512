/**
 * @file hwclock.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HWCLOCK_H_
#define HWCLOCK_H_

#include <cstdint>
#include <time.h>
#include <sys/time.h>

namespace rtc
{
enum class Type : uint8_t
{
    kMcP7941X,
    kDS3231,
    kPcF8563,
    kSocInternal,
    kUnknown
};
} // namespace rtc

class HwClock
{
   public:
    HwClock();
    void RtcProbe();

    void HcToSys(); // Set the System Clock from the Hardware Clock
    void SysToHc(); // Set the Hardware Clock from the System Clock

    bool Set(const struct tm* time);
    bool Get(struct tm* time) { return RtcGet(time); }

    bool AlarmSet(const struct tm* time) { return RtcSetAlarm(time); }

    bool AlarmGet(struct tm* time) { return RtcGetAlarm(time); }

    void AlarmEnable(bool enable) { alarm_enabled_ = enable; }

    bool AlarmIsEnabled() const { return alarm_enabled_; }

    bool IsConnected() const { return is_connected_; }

    void Run(bool do_run)
    {
        if (!do_run || !is_connected_)
        {
            return;
        }
        Process();
    }

    void Print();

    static HwClock* Get() { return s_this; }

   private:
    void Process();
    bool RtcSet(const struct tm* time);
    bool RtcGet(struct tm* time);
    bool RtcSetAlarm(const struct tm* time);
    bool RtcGetAlarm(struct tm* time);
    int MCP794xxAlarmWeekday(struct tm* time);
    void PCF8563GetAlarmMode();
    void PCF8563SetAlarmMode();

   private:
    uint32_t delay_micros_{0};
    uint32_t last_hc_to_sys_millis_{0};
    uint8_t address_{0};
    rtc::Type type_{rtc::Type::kUnknown};
    bool is_connected_{false};
    bool alarm_enabled_{false};
    bool alarm_pending_{false};

    static inline HwClock* s_this;
};

#endif  // HWCLOCK_H_
