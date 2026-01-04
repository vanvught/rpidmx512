/**
 * @file widgetparams.h
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

#ifndef WIDGETPARAMS_H_
#define WIDGETPARAMS_H_

#include <cstdint>

#include "configurationstore.h"
#include "widgetconfiguration.h"

class WidgetParams
{
   public:
    WidgetParams();

    void Load();
    void Set();

    uint8_t GetBreakTime() const { return store_widget_.break_time; }
    uint8_t GetMabTime() const { return store_widget_.mab_time; }
    uint8_t GetRefreshRate() const { return store_widget_.refresh_rate; }
    widget::Mode GetMode() const { return static_cast<widget::Mode>(store_widget_.mode); }
    uint8_t GetThrottle() const { return store_widget_.throttle; }

    static void StaticCallbackFunction(void* p, const char* s);

   private:
    void Dump();
    void CallbackFunction(const char* s);
    bool IsMaskSet(uint32_t mask) const { return (store_widget_.set_list & mask) == mask; }

   private:
    common::store::Widget store_widget_;
};

#endif  // WIDGETPARAMS_H_
