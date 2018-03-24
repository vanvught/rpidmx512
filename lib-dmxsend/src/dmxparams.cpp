/**
 * @file dmxparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#if defined (__circle__)
#define ALIGNED
#include <circle/logger.h>
#include <circle/stdarg.h>
#include <circle/util.h>
#else
#include "util.h"
#endif

#include "dmxparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#define DMX_PARAMS_MIN_BREAK_TIME		9
#define DMX_PARAMS_DEFAULT_BREAK_TIME	9
#define DMX_PARAMS_MAX_BREAK_TIME		127

#define DMX_PARAMS_MIN_MAB_TIME			1
#define DMX_PARAMS_DEFAULT_MAB_TIME		1
#define DMX_PARAMS_MAX_MAB_TIME			127

#define DMX_PARAMS_DEFAULT_REFRESH_RATE	40	///<

#define SET_BREAK_TIME_MASK			1<<0
#define SET_MAB_TIME_MASK			1<<1
#define SET_REFRESH_RATE_MASK		1<<2

static const char PARAMS_FILE_NAME[] ALIGNED = "params.txt";
static const char PARAMS_BREAK_TIME[] ALIGNED = "dmxsend_break_time";
static const char PARAMS_MAB_TIME[] ALIGNED = "dmxsend_mab_time";
static const char PARAMS_REFRESH_RATE[] ALIGNED = "dmxsend_refresh_rate";

void DMXParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((DMXParams *) p)->callbackFunction(s);
}

void DMXParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_BREAK_TIME, &value8) == SSCAN_OK) {
		if ((value8 >= (uint8_t) DMX_PARAMS_MIN_BREAK_TIME) && (value8 <= (uint8_t) DMX_PARAMS_MAX_BREAK_TIME)) {
			m_nBreakTime = value8;
			m_bSetList |= SET_BREAK_TIME_MASK;
		}
	} else if (Sscan::Uint8(pLine, PARAMS_MAB_TIME, &value8) == SSCAN_OK) {
		if ((value8 >= (uint8_t) DMX_PARAMS_MIN_MAB_TIME) && (value8 <= (uint8_t) DMX_PARAMS_MAX_MAB_TIME)) {
			m_nMabTime = value8;
			m_bSetList |= SET_MAB_TIME_MASK;
		}
	} else if (Sscan::Uint8(pLine, PARAMS_REFRESH_RATE, &value8) == SSCAN_OK) {
		m_nRefreshRate = value8;
		m_bSetList |= SET_REFRESH_RATE_MASK;
	}
}

DMXParams::DMXParams(void): m_bSetList(0) {
	m_nBreakTime = DMX_PARAMS_DEFAULT_BREAK_TIME;
	m_nMabTime = DMX_PARAMS_DEFAULT_MAB_TIME;
	m_nRefreshRate = DMX_PARAMS_DEFAULT_REFRESH_RATE;
}

DMXParams::~DMXParams(void) {
}

bool DMXParams::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(DMXParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void DMXParams::Set(DMXSend *pDMXSend) {
	assert(pDMXSend != 0);

	if (isMaskSet(SET_BREAK_TIME_MASK)) {
		pDMXSend->SetDmxBreakTime(m_nBreakTime);
	}

	if (isMaskSet(SET_MAB_TIME_MASK)) {
		pDMXSend->SetDmxMabTime(m_nMabTime);
	}

	if (isMaskSet(SET_REFRESH_RATE_MASK)) {
		uint32_t period = (uint32_t) 0;
		if (m_nRefreshRate != (uint8_t) 0) {
			period = (uint32_t) (1000000 / m_nRefreshRate);
		}
		pDMXSend->SetDmxPeriodTime(period);
	}
}

void DMXParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_BREAK_TIME_MASK)) {
		printf(" %s=%d\n", PARAMS_BREAK_TIME, (int) m_nBreakTime);
	}

	if (isMaskSet(SET_MAB_TIME_MASK)) {
		printf(" %s=%d\n", PARAMS_MAB_TIME, (int) m_nMabTime);
	}

	if (isMaskSet(SET_REFRESH_RATE_MASK)) {
		printf(" %s=%d\n", PARAMS_REFRESH_RATE, (int) m_nRefreshRate);
	}
#endif
}

uint8_t DMXParams::GetBreakTime(void) const {
	return m_nBreakTime;
}

uint8_t DMXParams::GetMabTime(void) const {
	return m_nMabTime;
}

uint8_t DMXParams::GetRefreshRate(void) const {
	return m_nRefreshRate;
}

bool DMXParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}

#if defined (__circle__)
void DMXParams::printf(const char *fmt, ...) {
	assert(fmt != 0);

	size_t fmtlen = strlen(fmt);
	char fmtbuf[fmtlen + 1];

	strcpy(fmtbuf, fmt);

	if (fmtbuf[fmtlen - 1] == '\n') {
		fmtbuf[fmtlen - 1] = '\0';
	}

	va_list var;
	va_start(var, fmt);

	CLogger::Get()->WriteV("", LogNotice, fmtbuf, var);

	va_end(var);
}
#endif
