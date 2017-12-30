/**
 * @file dmxrdm.cpp
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
#include <assert.h>

#include "dmxrdm.h"

#include "dmx.h"

DmxRdm::DmxRdm(uint8_t nGpioPin) {
	dmx_init_set_gpiopin(nGpioPin);
	dmx_init();
}

DmxRdm::~DmxRdm(void) {
}

void DmxRdm::SetPortDirection(TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert((tPortDirection = DMXRDM_PORT_DIRECTION_OUTP) || (tPortDirection = DMXRDM_PORT_DIRECTION_INP));

	dmx_set_port_direction((_dmx_port_direction)tPortDirection, bEnableData);
}

uint32_t DmxRdm::GetUpdatesPerSecond(void) const {
	return dmx_get_updates_per_seconde();
}

const uint8_t *DmxRdm::GetDmxCurrentData(void) {
	return dmx_get_current_data();
}

const uint8_t *DmxRdm::GetDmxAvailable(void) {
	return dmx_get_available();
}

void DmxRdm::SetDmxBreakTime(uint32_t nBreakTime) {
	dmx_set_output_break_time(nBreakTime);
}

uint32_t DmxRdm::GetDmxBreakTime(void) const {
	return dmx_get_output_break_time();
}

void DmxRdm::SetDmxMabTime(uint32_t nMabTime) {
	dmx_set_output_mab_time(nMabTime);
}

uint32_t DmxRdm::GetDmxMabTime(void) const {
	return dmx_get_output_mab_time();
}

void DmxRdm::SetDmxPeriodTime(uint32_t nPeriodTime) {
	dmx_set_output_period(nPeriodTime);
}

uint32_t DmxRdm::GetDmxPeriodTime(void) const {
	return dmx_get_output_period();
}
