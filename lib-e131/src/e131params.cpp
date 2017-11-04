/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <assert.h>
#include <uuid/uuid.h>

#if defined(__linux__) || defined (__CYGWIN__)
#define ALIGNED
#include <string.h>
#else
#include "util.h"
#endif

#include "e131params.h"
#include "e131.h"

#include "readconfigfile.h"
#include "sscan.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "e131.txt";
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";
static const char PARAMS_MERGE_MODE[] ALIGNED = "merge_mode";
static const char PARAMS_OUTPUT[] ALIGNED = "output";
static const char PARAMS_CID[] ALIGNED = "cid";

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((E131Params *) p)->callbackFunction(s);
}

void E131Params::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[UUID_STRING_LENGTH + 2];
	uint8_t len;
	uint16_t value16;

	if (Sscan::Uint16(pLine, PARAMS_UNIVERSE, &value16) == SSCAN_OK) {
		if (value16 == 0 || value16 > E131_UNIVERSE_MAX) {
			m_nUniverse = E131_UNIVERSE_DEFAULT;
		} else {
			m_nUniverse = value16;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		if(memcmp(value, "mon", 3) == 0) {
			m_tOutputType = OUTPUT_TYPE_MONITOR;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_MERGE_MODE, value, &len) == SSCAN_OK) {
		if(memcmp(value, "ltp", 3) == 0) {
			m_tMergeMode = E131_MERGE_LTP;
		}
		return;
	}

	len = UUID_STRING_LENGTH;
	if (Sscan::Uuid(pLine, PARAMS_CID, value, &len) == SSCAN_OK) {
		memcpy(m_aCidString, value, UUID_STRING_LENGTH);
		m_aCidString[UUID_STRING_LENGTH] = '\0';
		m_bHaveCustomCid = true;
	}

}

E131Params::E131Params(void): m_bSetList(0) {
	m_nUniverse = E131_UNIVERSE_DEFAULT;
	m_tMergeMode = E131_MERGE_HTP;
	m_tOutputType = OUTPUT_TYPE_DMX;
	memset(m_aCidString, 0, sizeof(m_aCidString));
	m_bHaveCustomCid = false;
}

E131Params::~E131Params(void) {
}

bool E131Params::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void E131Params::Set(E131Bridge *pE131Bridge) {
	assert(pE131Bridge != 0);

	if (m_bSetList == 0) {
		return;
	}
}

void E131Params::Dump(void) {
	if (m_bSetList == 0) {
		return;
	}

	printf("E131 parameters \'%s\':\n", PARAMS_FILE_NAME);
}

uint16_t E131Params::GetUniverse(void) const {
	return m_nUniverse;
}

TOutputType E131Params::GetOutputType(void) const {
	return m_tOutputType;
}

TMerge E131Params::GetMergeMode(void) const {
	return m_tMergeMode;
}

bool E131Params::isHaveCustomCid(void) const {
	return m_bHaveCustomCid;
}

const char* E131Params::GetCidString(void) {
	return m_aCidString;
}

bool E131Params::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}
