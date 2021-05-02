/**
 * @file widgetconfiguration.cpp
 *
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

#include <cstdint>
#include <cassert>

#include "widgetconfiguration.h"
#include "widget.h"

#include "dmx.h"

#include "debug.h"

uint8_t WidgetConfiguration::s_aDeviceTypeId[DEVICE_TYPE_ID_LENGTH] { 1, 0 };
uint8_t WidgetConfiguration::s_nFirmwareLsb { WIDGET_DEFAULT_FIRMWARE_LSB };	///< Firmware version LSB. Valid range is 0 to 255.
uint8_t WidgetConfiguration::s_nFirmwareMsb { FIRMWARE_RDM };					///< Firmware version MSB. Valid range is 0 to 255.
uint8_t WidgetConfiguration::s_nBreakTime { WIDGET_DEFAULT_BREAK_TIME };		///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
uint8_t WidgetConfiguration::s_nMabTime { WIDGET_DEFAULT_MAB_TIME };			///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
uint8_t WidgetConfiguration::s_nRefreshRate { WIDGET_DEFAULT_REFRESH_RATE };	///< DMX output rate in packets per second. Valid range is 1 to 40.

using namespace widget;

void WidgetConfiguration::SetRefreshRate(uint8_t nRefreshRate) {
	s_nRefreshRate = nRefreshRate;

	uint32_t nPeriod = 0;

	if (nRefreshRate != 0) {
		nPeriod = (1000000U / nRefreshRate);
	}

	Dmx::Get()->SetDmxPeriodTime(nPeriod);
}

void WidgetConfiguration::SetBreakTime(uint8_t nBreakTime) {
	s_nBreakTime = nBreakTime;
	Dmx::Get()->SetDmxBreakTime(static_cast<uint32_t>(nBreakTime * 10.67f));
}

void WidgetConfiguration::SetMabTime(uint8_t nMabTime) {
	s_nMabTime = nMabTime;
	Dmx::Get()->SetDmxMabTime(static_cast<uint32_t>(nMabTime * 10.67f));
}

void WidgetConfiguration::SetMode(Mode tMode) {
	DEBUG_PRINTF("%d", static_cast<int>(tMode));

	if (tMode == Mode::DMX_RDM) {
		s_nFirmwareLsb = FIRMWARE_RDM;
	} else {
		s_nFirmwareLsb = static_cast<uint8_t>(tMode);
	}

	Widget::Get()->SetMode(tMode);
}

void WidgetConfiguration::SetThrottle(uint8_t nThrottle) {
	uint32_t nPeriod = 0;

	if (nThrottle != 0) {
		nPeriod = (1000U / nThrottle);
	}

	Widget::Get()->SetReceivedDmxPacketPeriodMillis(nPeriod);
}
