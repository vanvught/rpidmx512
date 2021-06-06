/**
 * @file widget_monitor.h
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WIDGET_MONITOR_H_
#define WIDGET_MONITOR_H_

#include <stdbool.h>
#include <cstdint>

namespace widgetmonitor {
struct MonitorLine {
	static constexpr auto TIME = 3;
	static constexpr auto WIDGET_PARMS = 4;
	static constexpr auto LABEL = 6;
	static constexpr auto INFO = 7;
	static constexpr auto PORT_DIRECTION = 9;
	static constexpr auto DMX_DATA = 11;
	static constexpr auto PACKETS = 14;
	static constexpr auto RDM_DATA = 17;
	static constexpr auto RDM_CC = 27;
	static constexpr auto STATUS = 28;
	static constexpr auto STATS = 29;
};
}  // namespace widgetmonitor

class WidgetMonitor {
public:
	static void Line(int, const char *, ...);
	static void Uptime(uint8_t nLine);
	static void RdmData(int, uint16_t, const uint8_t *, bool);
	static void Update();

private:
	static void Sniffer();
	static void DmxData(const uint8_t * dmx_data, const int line);

private:
	static uint32_t s_nWidgetReceivedDmxPacketCountPrevious;
	static uint32_t s_nUpdatesPerSecondeMin;
	static uint32_t s_nUpdatesPerSecondeMax;
	static uint32_t s_nSlotsInPacketMin;
	static uint32_t s_nSlotsInPacketMax;
	static uint32_t s_nSlotToSlotMin;
	static uint32_t s_nSlotToSlotMax;
	static uint32_t s_nBreakToBreakMin;
	static uint32_t s_nBreakToBreakMax;
};

#endif /* WIDGET_MONITOR_H_ */
