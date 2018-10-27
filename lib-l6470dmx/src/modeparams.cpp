/**
 * @file modeparams.cpp
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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined(__linux__)
 #include <string.h>
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "modeparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_MAX_STEPS_MASK		(1 << 0)
#define SET_SWITCH_ACT_MASK		(1 << 1)
#define SET_SWITCH_DIR_MASK		(1 << 2)
#define SET_SWITCH_SPS_MASK		(1 << 3)
#define SET_SWITCH_MASK			(1 << 4)

static const char MODE_PARAMS_MAX_STEPS[] ALIGNED = "mode_max_steps";
static const char MODE_PARAMS_SWITCH_ACT[] ALIGNED = "mode_switch_act";
static const char MODE_PARAMS_SWITCH_DIR[] ALIGNED = "mode_switch_dir";
static const char MODE_PARAMS_SWITCH_SPS[] ALIGNED = "mode_switch_sps";
static const char MODE_PARAMS_SWITCH[] ALIGNED = "mode_switch";

ModeParams::ModeParams(const char *pFileName):
		m_bSetList(0),
		m_nMaxSteps (0),
		m_tSwitchAction(L6470_ABSPOS_RESET),
		m_tSwitchDir(L6470_DIR_REV),
		m_fSwitchStepsPerSec(0),
		m_bSwitch(true)
{
	assert(pFileName != 0);

	ReadConfigFile configfile(ModeParams::staticCallbackFunction, this);
	configfile.Read(pFileName);
}

ModeParams::~ModeParams(void) {
}

uint32_t ModeParams::GetMaxSteps(void) const {
	return m_nMaxSteps;
}

TL6470Action ModeParams::GetSwitchAction(void) const {
	return m_tSwitchAction;
}

TL6470Direction ModeParams::GetSwitchDir(void) const {
	return m_tSwitchDir;
}

float ModeParams::GetSwitchStepsPerSec(void) const {
	return m_fSwitchStepsPerSec;
}

bool ModeParams::HasSwitch(void) const {
	return m_bSwitch;
}

void ModeParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(SET_MAX_STEPS_MASK)) {
		printf("%s=%d steps\n", MODE_PARAMS_MAX_STEPS, m_nMaxSteps);
	}

	if(isMaskSet(SET_SWITCH_ACT_MASK)) {
		printf("%s=%s\n", MODE_PARAMS_SWITCH_ACT, m_tSwitchAction == L6470_ABSPOS_RESET ? "reset" : "copy");

	}

	if(isMaskSet(SET_SWITCH_DIR_MASK)) {
		printf("%s=%s\n", MODE_PARAMS_SWITCH_DIR, m_tSwitchDir == L6470_DIR_REV ? "reverse" : "forward");
	}

	if(isMaskSet(SET_SWITCH_SPS_MASK)) {
		printf("%s=%f step/s\n", MODE_PARAMS_SWITCH_SPS, m_fSwitchStepsPerSec);
	}

	if(isMaskSet(SET_SWITCH_MASK)) {
		printf("%s={%s}\n", MODE_PARAMS_SWITCH, m_bSwitch ? "Yes": "No");
	}
#endif
}

void ModeParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ModeParams *) p)->callbackFunction(s);
}

void ModeParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	float f;
	char value[128];
	uint8_t len;
	uint8_t value8;

	if (Sscan::Uint32(pLine, MODE_PARAMS_MAX_STEPS, &m_nMaxSteps) == SSCAN_OK) {
		m_bSetList |= SET_MAX_STEPS_MASK;
		return;
	}


	len = 5; //  copy, reset
	if (Sscan::Char(pLine, MODE_PARAMS_SWITCH_ACT, value, &len) == SSCAN_OK) {
		if (len == 4) {
			if (memcmp(value, "copy", 4) == 0) {
				m_tSwitchAction = L6470_ABSPOS_COPY;
				m_bSetList |= SET_SWITCH_ACT_MASK;
				return;
			}
		}
		if (len == 5) {
			if (memcmp(value, "reset", 5) == 0) {
				m_tSwitchAction = L6470_ABSPOS_RESET;
				m_bSetList |= SET_SWITCH_ACT_MASK;
				return;
			}
		}
	}

	len = 7; //  reverse, forward
	if (Sscan::Char(pLine, MODE_PARAMS_SWITCH_DIR, value, &len) == SSCAN_OK) {
		if (len != 7) {
			return;
		}
		if (memcmp(value, "forward", 7) == 0) {
			m_tSwitchDir = L6470_DIR_FWD;
			m_bSetList |= SET_SWITCH_DIR_MASK;
			return;
		}
		if (memcmp(value, "reverse", 7) == 0) {
			m_tSwitchDir = L6470_DIR_REV;
			m_bSetList |= SET_SWITCH_DIR_MASK;
			return;
		}
	}

	if (Sscan::Float(pLine, MODE_PARAMS_SWITCH_SPS, &f) == SSCAN_OK) {
		m_fSwitchStepsPerSec = f;
		m_bSetList |= SET_SWITCH_SPS_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, MODE_PARAMS_SWITCH, &value8) == SSCAN_OK) {
		if (value8 == 0) {
			m_bSwitch = false;
			m_bSetList |= SET_SWITCH_MASK;
		}
	}
}

bool ModeParams::isMaskSet(uint32_t mask) const {
	return (m_bSetList & mask) == mask;
}
