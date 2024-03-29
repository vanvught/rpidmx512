/**
 * @file widgetstore.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstddef>

#include "widgetparams.h"

#if defined (WIDGET_HAVE_FLASHROM)
# include "configstore.h"
#endif

class WidgetStore {
public:
#if defined (WIDGET_HAVE_FLASHROM)
	static void UpdateBreakTime(uint8_t nBreakTime) {
		ConfigStore::Get()->Update(configstore::Store::WIDGET, offsetof(struct TWidgetParams, nBreakTime), &nBreakTime, sizeof(uint8_t), WidgetParamsMask::BREAK_TIME);
	}

	static void UpdateMabTime(uint8_t nMabTime) {
		ConfigStore::Get()->Update(configstore::Store::WIDGET, offsetof(struct TWidgetParams, nMabTime), &nMabTime, sizeof(uint8_t), WidgetParamsMask::MAB_TIME);
	}

	static void UpdateRefreshRate(uint8_t nRefreshRate) {
		ConfigStore::Get()->Update(configstore::Store::WIDGET, offsetof(struct TWidgetParams, nRefreshRate), &nRefreshRate, sizeof(uint8_t), WidgetParamsMask::REFRESH_RATE);
	}
#else
	static void UpdateBreakTime(uint8_t nBreakTime) { }

	static void UpdateMabTime(uint8_t nMabTime) { }

	static void UpdateRefreshRate(uint8_t nRefreshRate) {	}
#endif

};

#endif /* WIDGETSTORE_H_ */
