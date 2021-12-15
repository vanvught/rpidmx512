/**
 * @file widgetparams.cpp
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "widgetparams.h"
#include "widgetparamsconst.h"
#include "widgetconfiguration.h"

#include "dmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "debug.h"

using namespace widget;

#if defined (HAVE_FLASHROM)
WidgetParams::WidgetParams(WidgetParamsStore* pWidgetParamsStore): m_pWidgetParamsStore(pWidgetParamsStore) {
#else
WidgetParams::WidgetParams() {
#endif
	m_tWidgetParams.nBreakTime = WIDGET_DEFAULT_BREAK_TIME;
	m_tWidgetParams.nMabTime = WIDGET_DEFAULT_MAB_TIME;
	m_tWidgetParams.nRefreshRate = WIDGET_DEFAULT_REFRESH_RATE;
	m_tWidgetParams.tMode = static_cast<uint8_t>(Mode::DMX_RDM);
	m_tWidgetParams.nThrottle = 0;
}

bool WidgetParams::Load() {
	m_tWidgetParams.nSetList = 0;

	ReadConfigFile configfile(WidgetParams::staticCallbackFunction, this);

#if defined (HAVE_FLASHROM)
# if !defined(DISABLE_FS)
	if (configfile.Read( WidgetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pWidgetParamsStore != nullptr) {
			m_pWidgetParamsStore->Update(&m_tWidgetParams);
		}
	} else
# endif
	if (m_pWidgetParamsStore != nullptr) {
		m_pWidgetParamsStore->Copy(&m_tWidgetParams);
	} else {
		return false;
	}

	return true;
#else
	return configfile.Read(WidgetParamsConst::FILE_NAME);
#endif
}

void WidgetParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_BREAK_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 >= WIDGET_MIN_BREAK_TIME) && (nValue8 <= WIDGET_MAX_BREAK_TIME)) {
			m_tWidgetParams.nBreakTime = nValue8;
			m_tWidgetParams.nSetList |= WidgetParamsMask::BREAK_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_MAB_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 >= WIDGET_MIN_MAB_TIME) && (nValue8 <= WIDGET_MAX_MAB_TIME)) {
			m_tWidgetParams.nMabTime = nValue8;
			m_tWidgetParams.nSetList |= WidgetParamsMask::MAB_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMXUSBPRO_REFRESH_RATE, nValue8) == Sscan::OK) {
		m_tWidgetParams.nRefreshRate = nValue8;
		m_tWidgetParams.nSetList |= WidgetParamsMask::REFRESH_RATE;
		return;
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::WIDGET_MODE, nValue8) == Sscan::OK) {
		if (nValue8 <= static_cast<uint8_t>(Mode::RDM_SNIFFER)) {
			m_tWidgetParams.tMode = nValue8;
			m_tWidgetParams.nSetList |= WidgetParamsMask::MODE;
			return;
		}
	}

	if (Sscan::Uint8(pLine,  WidgetParamsConst::DMX_SEND_TO_HOST_THROTTLE, nValue8) == Sscan::OK) {
		m_tWidgetParams.nThrottle = nValue8;
		m_tWidgetParams.nSetList |= WidgetParamsMask::THROTTLE;
		return;
	}

}

void WidgetParams::Set() {
	if (isMaskSet(WidgetParamsMask::REFRESH_RATE)) {
		WidgetConfiguration::SetRefreshRate(m_tWidgetParams.nRefreshRate);
	}

	if (isMaskSet(WidgetParamsMask::BREAK_TIME)) {
		WidgetConfiguration::SetBreakTime(m_tWidgetParams.nBreakTime);
	}

	if (isMaskSet(WidgetParamsMask::MAB_TIME)) {
		WidgetConfiguration::SetMabTime(m_tWidgetParams.nMabTime);
	}

	if (isMaskSet(WidgetParamsMask::THROTTLE)) {
		WidgetConfiguration::SetThrottle(m_tWidgetParams.nThrottle);
	}

	if (isMaskSet(WidgetParamsMask::MODE)) {
		WidgetConfiguration::SetMode(static_cast<Mode>(m_tWidgetParams.tMode));
	}
}

void WidgetParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<WidgetParams*>(p))->callbackFunction(s);
}
