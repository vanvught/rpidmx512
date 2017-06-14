/**
 * @file dmxsender.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "dmx.h"
#include "dmxsender.h"

DMXSender::DMXSender(void) : m_bIsStarted(false) {
	dmx_init();
}

DMXSender::~DMXSender(void) {
}

void DMXSender::Start(void) {
	if (m_bIsStarted) {
		return;
	}

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, true);
	m_bIsStarted = true;
}

void DMXSender::Stop(void) {
	if (!m_bIsStarted) {
		return;
	}

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);
	m_bIsStarted = false;
}

void DMXSender::SetData(const uint8_t nPortId, const uint8_t *pData, const uint16_t nLength) {
	dmx_set_send_data_without_sc(pData, nLength);
}

void DMXSender::SetBreakTime(const uint32_t nBreakTime) {
	dmx_set_output_break_time(nBreakTime);
}

const uint32_t DMXSender::GetBreakTime(void) {
	return dmx_get_output_break_time();
}

void DMXSender::SetMabTime(const uint32_t nMabTime) {
	dmx_set_output_mab_time(nMabTime);
}

const uint32_t DMXSender::GetMabTime(void) {
	return dmx_get_output_mab_time();
}

void DMXSender::SetPeriodTime(const uint32_t nPeriodTime) {
	dmx_set_output_period(nPeriodTime);
}

const uint32_t DMXSender::GetPeriodTime(void) {
	return dmx_get_output_period();
}
