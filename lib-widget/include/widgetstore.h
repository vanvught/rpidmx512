/**
 * @file widgetstore.h
 *
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

#ifndef WIDGETSTORE_H_
#define WIDGETSTORE_H_

#include <cstdint>

#if defined(WIDGET_HAVE_FLASHROM)
#include "configstore.h"
#endif

namespace widget_store
{

#if defined(WIDGET_HAVE_FLASHROM)
inline void SaveBreakTime(uint8_t break_time)
{
    ConfigStore::Instance().WidgetUpdate(&common::store::Widget::break_time, break_time);
}

inline void SaveMabTime(uint8_t mab_time)
{
    ConfigStore::Instance().WidgetUpdate(&common::store::Widget::mab_time, mab_time);
}

inline void SaveRefreshRate(uint8_t refresh_rate)
{
    ConfigStore::Instance().WidgetUpdate(&common::store::Widget::refresh_rate, refresh_rate);
}
#else
inline void SaveBreakTime([[maybe_unused]] uint8_t break_time) {}
inline void SaveMabTime([[maybe_unused]] uint8_t mab_time) {}
inline void SaveRefreshRate([[maybe_unused]] uint8_t refresh_rate) {}
#endif

} // namespace widget_store

#endif  // WIDGETSTORE_H_
