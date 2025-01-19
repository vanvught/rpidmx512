/**
 * @file widgetparams.cpp
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "widgetparams.h"
#include "widgetparamsconst.h"
#include "widgetconfiguration.h"

#include "dmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "debug.h"

using namespace widget;

WidgetParams::WidgetParams() {
	m_Params.nBreakTime = WIDGET_DEFAULT_BREAK_TIME;
	m_Params.nMabTime = WIDGET_DEFAULT_MAB_TIME;
	m_Params.nRefreshRate = WIDGET_DEFAULT_REFRESH_RATE;
	m_Params.tMode = static_cast<uint8_t>(Mode::DMX_RDM);
	m_Params.nThrottle = 0;
}

void WidgetParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

	ReadConfigFile configfile(WidgetParams::StaticCallbackFunction, this);

#if defined (WIDGET_HAVE_FLASHROM)
# if !defined(DISABLE_FS)
	if (configfile.Read( WidgetParamsConst::FILE_NAME)) {
		WidgetParamsStore::Update(&m_Params);
	} else
# endif
		WidgetParamsStore::Copy(&m_Params);
#else
	configfile.Read(WidgetParamsConst::FILE_NAME);
#endif
#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void WidgetParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_BREAK_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 >= WIDGET_MIN_BREAK_TIME) && (nValue8 <= WIDGET_MAX_BREAK_TIME)) {
			m_Params.nBreakTime = nValue8;
			m_Params.nSetList |= WidgetParamsMask::BREAK_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_MAB_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 >= WIDGET_MIN_MAB_TIME) && (nValue8 <= WIDGET_MAX_MAB_TIME)) {
			m_Params.nMabTime = nValue8;
			m_Params.nSetList |= WidgetParamsMask::MAB_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, nValue8) == Sscan::OK) {
		m_Params.nRefreshRate = nValue8;
		m_Params.nSetList |= WidgetParamsMask::REFRESH_RATE;
		return;
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::WIDGET_MODE, nValue8) == Sscan::OK) {
		if (nValue8 <= static_cast<uint8_t>(Mode::RDM_SNIFFER)) {
			m_Params.tMode = nValue8;
			m_Params.nSetList |= WidgetParamsMask::MODE;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, nValue8) == Sscan::OK) {
		m_Params.nThrottle = nValue8;
		m_Params.nSetList |= WidgetParamsMask::THROTTLE;
		return;
	}

}

void WidgetParams::Set() {
	if (isMaskSet(WidgetParamsMask::REFRESH_RATE)) {
		WidgetConfiguration::SetRefreshRate(m_Params.nRefreshRate);
	}

	if (isMaskSet(WidgetParamsMask::BREAK_TIME)) {
		WidgetConfiguration::SetBreakTime(m_Params.nBreakTime);
	}

	if (isMaskSet(WidgetParamsMask::MAB_TIME)) {
		WidgetConfiguration::SetMabTime(m_Params.nMabTime);
	}

	if (isMaskSet(WidgetParamsMask::THROTTLE)) {
		WidgetConfiguration::SetThrottle(m_Params.nThrottle);
	}

	if (isMaskSet(WidgetParamsMask::MODE)) {
		WidgetConfiguration::SetMode(static_cast<Mode>(m_Params.tMode));
	}
}

void WidgetParams::StaticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<WidgetParams*>(p))->callbackFunction(s);
}

void WidgetParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, WidgetParamsConst::FILE_NAME);
	printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_BREAK_TIME, static_cast<int>(m_Params.nBreakTime));
	printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_MAB_TIME, static_cast<int>(m_Params.nMabTime));
	printf(" %s=%d\n", WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, static_cast<int>(m_Params.nRefreshRate));
	printf(" %s=%d\n", WidgetParamsConst::WIDGET_MODE, static_cast<int>(m_Params.tMode));
	printf(" %s=%d\n", WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, static_cast<int>(m_Params.nThrottle));
}
