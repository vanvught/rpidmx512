/**
 * @file widgetmonitor.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef WIDGETMONITOR_H_
#define WIDGETMONITOR_H_

#include <cstdint>

namespace widgetmonitor
{
struct MonitorLine
{
    static constexpr auto kTime = 3;
    static constexpr auto kWidgetParms = 4;
    static constexpr auto kLabel = 6;
    static constexpr auto kInfo = 7;
    static constexpr auto kPortDirection = 9;
    static constexpr auto kDmxData = 11;
    static constexpr auto kPackets = 14;
    static constexpr auto kRdmData = 17;
    static constexpr auto kRdmCc = 27;
    static constexpr auto kStatus = 28;
    static constexpr auto kStats = 29;
};
} // namespace widgetmonitor

class WidgetMonitor
{
   public:
    static void Line(int, const char*, ...);
    static void Uptime(uint8_t line);
    static void RdmData(int, uint16_t, const uint8_t*, bool);
    static void Update();

   private:
    static void Sniffer();
    static void DmxData(const uint8_t* dmx_data, int line);

   private:
    inline static uint32_t s_nWidgetReceivedDmxPacketCountPrevious{0};
    inline static uint32_t s_nUpdatesPerSecondeMin{UINT32_MAX};
    inline static uint32_t s_nUpdatesPerSecondeMax{0};
    inline static uint32_t s_nSlotsInPacketMin{UINT32_MAX};
    inline static uint32_t s_nSlotsInPacketMax{0};
    inline static uint32_t s_nSlotToSlotMin{UINT32_MAX};
    inline static uint32_t s_nSlotToSlotMax{0};
    inline static uint32_t s_nBreakToBreakMin{UINT32_MAX};
    inline static uint32_t s_nBreakToBreakMax{0};
};

#endif  // WIDGETMONITOR_H_
