/**
 * @file widgetconfiguration.cpp
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

#include <cstdint>

#include "widgetconfiguration.h"
#include "widget.h"
#include "dmx.h"

void WidgetConfiguration::SetRefreshRate(uint8_t refresh_rate)
{
    s_refresh_rate = refresh_rate;

    uint32_t period = 0;

    if (refresh_rate != 0)
    {
        period = (1000000U / refresh_rate);
    }

    Dmx::Get()->SetDmxPeriodTime(period);
}

void WidgetConfiguration::SetBreakTime(uint8_t break_time)
{
    s_break_time = break_time;
    Dmx::Get()->SetDmxBreakTime(static_cast<uint32_t>(break_time * 10.67f));
}

void WidgetConfiguration::SetMabTime(uint8_t mab_time)
{
    s_mab_time = mab_time;
    Dmx::Get()->SetDmxMabTime(static_cast<uint32_t>(mab_time * 10.67f));
}

void WidgetConfiguration::SetMode(widget::Mode mode)
{
    s_firmware_lsb = static_cast<uint8_t>(mode);
    Widget::Get()->SetMode(mode);
}

void WidgetConfiguration::SetThrottle(uint8_t throttle)
{
    uint32_t period = 0;

    if (throttle != 0)
    {
        period = (1000U / throttle);
    }

    Widget::Get()->SetReceivedDmxPacketPeriodMillis(period);
}
