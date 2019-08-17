/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "circle/actled.h"
#include "circle/bcmpropertytags.h"
#include "circle/machineinfo.h"
#include "circle/timer.h"
#include "circle/util.h"
#include "circle/version.h"

#include "hardware.h"

static const char s_CpuName[4][24] __attribute__((aligned(4))) = { "ARM1176JZF-S", "Cortex-A7", "Cortex-A53 (ARMv8)", "Unknown" };
static const uint8_t s_nCpuNameLength[4] __attribute__((aligned(4))) = {(uint8_t) 12, (uint8_t) 9, (uint8_t) 18, (uint8_t) 8};

const char s_Machine[] __attribute__((aligned(4))) = "arm";
#define MACHINE_LENGTH (sizeof(s_Machine)/sizeof(s_Machine[0]) - 1)

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware(void) {
	s_pThis = this;
}

Hardware::~Hardware(void) {
}

const char* Hardware::GetMachine(uint8_t& nLength) {
	nLength = MACHINE_LENGTH;
	return s_Machine;
}

const char* Hardware::GetSysName(uint8_t& nLength) {
	nLength = sizeof(CIRCLE_NAME);
	return CIRCLE_NAME;
}

const char* Hardware::GetBoardName(uint8_t& nLength) {
	const char *p = CMachineInfo::Get()->GetMachineName();
	nLength = strlen(p);
	return p;
}

const char* Hardware::GetCpuName(uint8_t& nLength) {
	TSoCType tSocType = CMachineInfo::Get()->GetSoCType();
		nLength = s_nCpuNameLength[tSocType];
	return s_CpuName[tSocType];
}

const char* Hardware::GetSocName(uint8_t& nLength) {
	const char *p = CMachineInfo::Get()->GetSoCName();
	nLength = strlen(p);
	return p;
}

float Hardware::GetCoreTemperature(void) {
	CBcmPropertyTags Tags;
	TPropertyTagTemperature TagTemperature;
	TagTemperature.nTemperatureId = TEMPERATURE_ID;

	if (!Tags.GetTag(PROPTAG_GET_TEMPERATURE, &TagTemperature, sizeof TagTemperature, 4)) {
		return -1;
	}

	return (float) TagTemperature.nValue / 1000;
}
