/**
 * @file widgetparamsdump.cpp
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>

#include "widgetparams.h"
#include "widgetparamsconst.h"

void WidgetParams::Dump() {
#ifndef NDEBUG
	if (m_tWidgetParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, WidgetParamsConst::FILE_NAME);

	if (isMaskSet(WidgetParamsMask::BREAK_TIME)) {
		printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_BREAK_TIME, static_cast<int>(m_tWidgetParams.nBreakTime));
	}

	if (isMaskSet(WidgetParamsMask::MAB_TIME)) {
		printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_MAB_TIME, static_cast<int>(m_tWidgetParams.nMabTime));
	}

	if (isMaskSet(WidgetParamsMask::REFRESH_RATE)) {
		printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, static_cast<int>(m_tWidgetParams.nRefreshRate));
	}

	if (isMaskSet(WidgetParamsMask::MODE)) {
		printf(" %s=%d\n", WidgetParamsConst::WIDGET_MODE, static_cast<int>(m_tWidgetParams.tMode));
	}

	if (isMaskSet(WidgetParamsMask::THROTTLE)) {
		printf(" %s=%d\n", WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, static_cast<int>(m_tWidgetParams.nThrottle));
	}
#endif
}
