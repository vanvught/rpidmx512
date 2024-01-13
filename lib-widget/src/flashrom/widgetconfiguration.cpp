/**
 * @file widgetconfiguration.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "widgetconfiguration.h"
#include "widgetstore.h"

#include "dmx.h"

void WidgetConfiguration::Store(const struct TWidgetConfiguration *widget_params) {
	if (widget_params->nBreakTime != s_nBreakTime) {
		s_nBreakTime = widget_params->nBreakTime;
		Dmx::Get()->SetDmxBreakTime(static_cast<uint32_t>(s_nBreakTime * 10.67));
		WidgetStore::UpdateBreakTime(widget_params->nBreakTime);
	}

	if (widget_params->nMabTime != s_nMabTime) {
		s_nMabTime = widget_params->nMabTime;
		Dmx::Get()->SetDmxMabTime(static_cast<uint32_t>(s_nMabTime * 10.67));
		WidgetStore::UpdateMabTime(widget_params->nMabTime);
	}

	if (widget_params->nRefreshRate != s_nRefreshRate) {
		s_nRefreshRate = widget_params->nRefreshRate;
		Dmx::Get()->SetDmxPeriodTime(widget_params->nRefreshRate == 0 ? 0 : (1000000U / widget_params->nRefreshRate));
		WidgetStore::UpdateRefreshRate(widget_params->nRefreshRate);
	}
}
