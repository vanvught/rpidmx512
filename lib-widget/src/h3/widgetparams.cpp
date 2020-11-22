/**
 * @file widgetparams.cpp
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "widgetparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "dmx.h"
#include "widget.h"

extern "C" {
	void widget_params_set_firmware_msb(uint8_t msb);
	void widget_params_set_break_time(uint8_t break_time);
	void widget_params_set_mab_time(uint8_t mab_time);
	void widget_params_set_refresh_rate(uint8_t refresh_rate);
}

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

enum {
	WIDGET_MIN_BREAK_TIME = 9,
	WIDGET_DEFAULT_BREAK_TIME = 9,
	WIDGET_MAX_BREAK_TIME = 127
};

enum {
	WIDGET_MIN_MAB_TIME = 1,
	WIDGET_DEFAULT_MAB_TIME = 1,
	WIDGET_MAX_MAB_TIME = 127,
};

enum {
	WIDGET_DEFAULT_REFRESH_RATE = 40
} ;


enum {
	FIRMWARE_NORMAL_DMX = 1,	///< Normal DMX firmware. Supports all messages except Send RDM (label=7), Send RDM Discovery Request(label=11) and receive RDM .
	FIRMWARE_RDM = 2,			///< RDM firmware. This enables the Widget to act as an RDM Controller.
	FIRMWARE_RDM_SNIFFER = 3	///< RDM Sniffer firmware. This is for use with the Openlighting RDM packet monitoring application.
};

static const char PARAMS_FILE_NAME[] = "params.txt";								///< Parameters file name
static const char DMXUSBPRO_PARAMS_BREAK_TIME[] = "dmxusbpro_break_time";			///<
static const char DMXUSBPRO_PARAMS_MAB_TIME[] = "dmxusbpro_mab_time";				///<
static const char DMXUSBPRO_PARAMS_REFRESH_RATE[] = "dmxusbpro_refresh_rate";		///<

static const char PARAMS_WIDGET_MODE[] = "widget_mode";								///<
static const char PARAMS_DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";	///<

struct _widget_params {
	uint8_t firmware_lsb;			///< Firmware version LSB. Valid range is 0 to 255.
	uint8_t firmware_msb;			///< Firmware version MSB. Valid range is 0 to 255.
	uint8_t break_time;				///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
	uint8_t mab_time;				///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
	uint8_t refresh_rate;			///< DMX output rate in packets per second. Valid range is 1 to 40.
};

WidgetParams::WidgetParams(WidgetParamsStore* pWidgetParamsStore): m_pWidgetParamsStore(pWidgetParamsStore) {
	m_tWidgetParams.nBreakTime = WIDGET_DEFAULT_BREAK_TIME;
	m_tWidgetParams.nMabTime = WIDGET_DEFAULT_MAB_TIME;
	m_tWidgetParams.nRefreshRate = WIDGET_DEFAULT_REFRESH_RATE;
	m_tWidgetParams.tMode = static_cast<TWidgetMode>(MODE_DMX_RDM);
	m_tWidgetParams.nThrottle = 0;
}

bool WidgetParams::Load() {
	m_tWidgetParams.nSetList = 0;

	ReadConfigFile configfile(WidgetParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pWidgetParamsStore != nullptr) {
			m_pWidgetParamsStore->Update(&m_tWidgetParams);
		}
	} else if (m_pWidgetParamsStore != nullptr) {
		m_pWidgetParamsStore->Copy(&m_tWidgetParams);
	} else {
		return false;
	}

	return true;
}

void WidgetParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t value8;

	if (Sscan::Uint8(pLine, DMXUSBPRO_PARAMS_BREAK_TIME, value8) == Sscan::OK) {
		if ((value8 >= WIDGET_MIN_BREAK_TIME) && (value8 <= WIDGET_MAX_BREAK_TIME)) {
			m_tWidgetParams.nBreakTime = value8;
			m_tWidgetParams.nSetList |= WIDGET_PARAMS_MASK_BREAK_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine, DMXUSBPRO_PARAMS_MAB_TIME, value8) == Sscan::OK) {
		if ((value8 >= WIDGET_MIN_MAB_TIME) && (value8 <= WIDGET_MAX_MAB_TIME)) {
			m_tWidgetParams.nMabTime = value8;
			m_tWidgetParams.nSetList |= WIDGET_PARAMS_MASK_MAB_TIME;
			return;
		}
	}

	if (Sscan::Uint8(pLine, DMXUSBPRO_PARAMS_REFRESH_RATE, value8) == Sscan::OK) {
		m_tWidgetParams.nRefreshRate = value8;
		m_tWidgetParams.nSetList |= WIDGET_PARAMS_MASK_REFRESH_RATE;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_WIDGET_MODE, value8) == Sscan::OK) {
		if (value8 <= WIDGET_MODE_RDM_SNIFFER) {
			m_tWidgetParams.tMode = static_cast<TWidgetMode>(value8);
			m_tWidgetParams.nSetList |= WIDGET_PARAMS_MASK_MODE;
			return;
		}
	}

	if (Sscan::Uint8(pLine, PARAMS_DMX_SEND_TO_HOST_THROTTLE, value8) == Sscan::OK) {
		m_tWidgetParams.nThrottle = value8;
		m_tWidgetParams.nSetList |= WIDGET_PARAMS_MASK_THROTTLE;
		return;
	}

}

void WidgetParams::Set() {
	uint32_t period = 0;

	if (isMaskSet(WIDGET_PARAMS_MASK_REFRESH_RATE)) {
		if (m_tWidgetParams.nRefreshRate != 0) {
			period = (1000000U / m_tWidgetParams.nRefreshRate);
		}

		dmx_set_output_period(period);

		widget_params_set_refresh_rate(m_tWidgetParams.nRefreshRate);
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_BREAK_TIME)) {
		dmx_set_output_break_time((static_cast<float>(m_tWidgetParams.nBreakTime) * 10.67));
		widget_params_set_break_time(m_tWidgetParams.nBreakTime);
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_MAB_TIME)) {
		dmx_set_output_mab_time((static_cast<float>(m_tWidgetParams.nMabTime) * 10.67));
		widget_params_set_mab_time(m_tWidgetParams.nMabTime);
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_THROTTLE)) {
		period = 0;

		if (m_tWidgetParams.nThrottle != 0) {
			period = (1000000U / m_tWidgetParams.nThrottle);
		}

		widget_set_received_dmx_packet_period(period);
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_MODE)) {
		widget_set_mode(static_cast<_widget_mode>(m_tWidgetParams.tMode));

		if (m_tWidgetParams.tMode == static_cast<TWidgetMode>(MODE_DMX_RDM)) {
			widget_params_set_firmware_msb(FIRMWARE_RDM);
		} else {
			widget_params_set_firmware_msb(m_tWidgetParams.tMode);
		}
	}
}

void WidgetParams::Dump() {
#ifndef NDEBUG
	if (m_tWidgetParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(WIDGET_PARAMS_MASK_BREAK_TIME)) {
		printf(" %s=%d\n", DMXUSBPRO_PARAMS_BREAK_TIME,
				static_cast<int>(m_tWidgetParams.nBreakTime));
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_MAB_TIME)) {
		printf(" %s=%d\n", DMXUSBPRO_PARAMS_MAB_TIME,
				static_cast<int>(m_tWidgetParams.nMabTime));
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_REFRESH_RATE)) {
		printf(" %s=%d\n", DMXUSBPRO_PARAMS_REFRESH_RATE,
				static_cast<int>(m_tWidgetParams.nRefreshRate));
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_MODE)) {
		printf(" %s=%d\n", PARAMS_WIDGET_MODE, static_cast<int>(m_tWidgetParams.tMode));
	}

	if (isMaskSet(WIDGET_PARAMS_MASK_THROTTLE)) {
		printf(" %s=%d\n", PARAMS_DMX_SEND_TO_HOST_THROTTLE, static_cast<int>(m_tWidgetParams.nThrottle));
	}
#endif
}

void WidgetParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<WidgetParams*>(p))->callbackFunction(s);
}
