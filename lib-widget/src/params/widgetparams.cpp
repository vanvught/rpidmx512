/**
 * @file widgetparams.cpp
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "widgetparams.h"
#include "configurationstore.h"
#include "widgetparamsconst.h"
#include "widgetconfiguration.h"
#include "params/readconfigfile.h"
#include "params/sscan.h"
 #include "firmware/debug/debug_debug.h"

struct WidgetParamsMask
{
    static constexpr uint32_t kBreakTime = (1U << 0);
    static constexpr uint32_t kMabTime = (1U << 1);
    static constexpr uint32_t kRefreshRate = (1U << 2);
    static constexpr uint32_t kMode = (1U << 3);
    static constexpr uint32_t kThrottle = (1U << 4);
};


static void Update(const common::store::Widget* store)
{
    ConfigStore::Instance().Store(store, &ConfigurationStore::widget);
}

static void Copy(struct ::common::store::Widget* store)
{
    ConfigStore::Instance().Copy(store, &ConfigurationStore::widget);
}

WidgetParams::WidgetParams()
{
    store_widget_.break_time = WIDGET_DEFAULT_BREAK_TIME;
    store_widget_.mab_time = WIDGET_DEFAULT_MAB_TIME;
    store_widget_.refresh_rate = WIDGET_DEFAULT_REFRESH_RATE;
    store_widget_.mode = static_cast<uint8_t>(widget::Mode::kDmxRdm);
    store_widget_.throttle = 0;
}

void WidgetParams::Load()
{
    DEBUG_ENTRY();

    ReadConfigFile configfile(WidgetParams::StaticCallbackFunction, this);

#if defined(WIDGET_HAVE_FLASHROM)
#if !defined(DISABLE_FS)
    if (configfile.Read(WidgetParamsConst::FILE_NAME))
    {
        Update(&store_widget_);
    }
    else
#endif
       Copy(&store_widget_);
#else
    configfile.Read(WidgetParamsConst::FILE_NAME);
#endif
#ifndef NDEBUG
    Dump();
#endif
    DEBUG_EXIT();
}

void WidgetParams::CallbackFunction(const char* line)
{
    assert(line != nullptr);

    uint8_t value8;

    if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_BREAK_TIME, value8) == Sscan::OK)
    {
        if ((value8 >= WIDGET_MIN_BREAK_TIME) && (value8 <= WIDGET_MAX_BREAK_TIME))
        {
            store_widget_.break_time = value8;
            store_widget_.set_list |= WidgetParamsMask::kBreakTime;
            return;
        }
    }

    if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_MAB_TIME, value8) == Sscan::OK)
    {
        if ((value8 >= WIDGET_MIN_MAB_TIME) && (value8 <= WIDGET_MAX_MAB_TIME))
        {
            store_widget_.mab_time = value8;
            store_widget_.set_list |= WidgetParamsMask::kMabTime;
            return;
        }
    }

    if (Sscan::Uint8(line, WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, value8) == Sscan::OK)
    {
        store_widget_.refresh_rate = value8;
        store_widget_.set_list |= WidgetParamsMask::kRefreshRate;
        return;
    }

    if (Sscan::Uint8(line, WidgetParamsConst::WIDGET_MODE, value8) == Sscan::OK)
    {
        if (value8 <= static_cast<uint8_t>(widget::Mode::kRdmSniffer))
        {
            store_widget_.mode = value8;
            store_widget_.set_list |= WidgetParamsMask::kMode;
            return;
        }
    }

    if (Sscan::Uint8(line, WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, value8) == Sscan::OK)
    {
        store_widget_.throttle = value8;
        store_widget_.set_list |= WidgetParamsMask::kThrottle;
        return;
    }
}

void WidgetParams::Set()
{
    if (IsMaskSet(WidgetParamsMask::kRefreshRate))
    {
        WidgetConfiguration::SetRefreshRate(store_widget_.refresh_rate);
    }

    if (IsMaskSet(WidgetParamsMask::kBreakTime))
    {
        WidgetConfiguration::SetBreakTime(store_widget_.break_time);
    }

    if (IsMaskSet(WidgetParamsMask::kMabTime))
    {
        WidgetConfiguration::SetMabTime(store_widget_.mab_time);
    }

    if (IsMaskSet(WidgetParamsMask::kThrottle))
    {
        WidgetConfiguration::SetThrottle(store_widget_.throttle);
    }

    if (IsMaskSet(WidgetParamsMask::kMode))
    {
        WidgetConfiguration::SetMode(static_cast<widget::Mode>(store_widget_.mode));
    }
}

void WidgetParams::StaticCallbackFunction(void* p, const char* s)
{
    assert(p != nullptr);
    assert(s != nullptr);

    (static_cast<WidgetParams*>(p))->CallbackFunction(s);
}

void WidgetParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, WidgetParamsConst::FILE_NAME);
    printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_BREAK_TIME, static_cast<int>(store_widget_.break_time));
    printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_MAB_TIME, static_cast<int>(store_widget_.mab_time));
    printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, static_cast<int>(store_widget_.refresh_rate));
    printf(" %s=%d\n", WidgetParamsConst::WIDGET_MODE, static_cast<int>(store_widget_.mode));
    printf(" %s=%d\n", WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, static_cast<int>(store_widget_.throttle));
}
