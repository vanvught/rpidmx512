/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#if defined(BARE_METAL)
 #include "util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "e131params.h"
#include "e131.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_UNIVERSE_MASK		1<<0
#define SET_MERGE_MODE_MASK		1<<1
#define SET_OUTPUT_MASK			1<<2
#define SET_CID_MASK			1<<3

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
		m_bSetList |= SET_UNIVERSE_MASK;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		if (memcmp(value, "spi", 3) == 0) {
			m_tOutputType = OUTPUT_TYPE_SPI;
			m_bSetList |= SET_OUTPUT_MASK;
		} else if (memcmp(value, "mon", 3) == 0) {
			m_tOutputType = OUTPUT_TYPE_MONITOR;
			m_bSetList |= SET_OUTPUT_MASK;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_MERGE_MODE, value, &len) == SSCAN_OK) {
		if(memcmp(value, "ltp", 3) == 0) {
			m_tMergeMode = E131_MERGE_LTP;
			m_bSetList |= SET_MERGE_MODE_MASK;
		}
		return;
	}

	len = UUID_STRING_LENGTH;
	if (Sscan::Uuid(pLine, PARAMS_CID, value, &len) == SSCAN_OK) {
		memcpy(m_aCidString, value, UUID_STRING_LENGTH);
		m_aCidString[UUID_STRING_LENGTH] = '\0';
		m_bHaveCustomCid = true;
		m_bSetList |= SET_CID_MASK;
		return;
	}
}

E131Params::E131Params(void):
	m_bSetList(0),
	m_nUniverse(E131_UNIVERSE_DEFAULT),
	m_tOutputType(OUTPUT_TYPE_DMX),
	m_tMergeMode(E131_MERGE_HTP),
	m_bHaveCustomCid(false)
{
	memset(m_aCidString, 0, sizeof(m_aCidString));
}

E131Params::~E131Params(void) {
}

bool E131Params::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

// FIXME E131Params::Set(E131Bridge *pE131Bridge)
void E131Params::Set(E131Bridge *pE131Bridge) {
	assert(pE131Bridge != 0);

	if (m_bSetList == 0) {
		return;
	}

	if (isMaskSet(SET_UNIVERSE_MASK)) {
		pE131Bridge->SetUniverse(m_nUniverse);
	}

	if (isMaskSet(SET_MERGE_MODE_MASK)) {
		pE131Bridge->SetMergeMode(m_tMergeMode);
	}

}

void E131Params::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_UNIVERSE_MASK)) {
		printf(" %s=%d\n", PARAMS_UNIVERSE, (int) m_nUniverse);
	}

	if (isMaskSet(SET_CID_MASK)) {
		printf(" %s=%s\n", PARAMS_CID, m_aCidString);
	}

	if (isMaskSet(SET_MERGE_MODE_MASK)) {
		printf(" %s=%s\n", PARAMS_MERGE_MODE, m_tMergeMode == E131_MERGE_HTP ? "HTP" : "LTP");
	}

	if (isMaskSet(SET_OUTPUT_MASK)) {
		printf(" %s=%s [%d]\n", PARAMS_OUTPUT, m_tOutputType == OUTPUT_TYPE_MONITOR ? "mon" : (m_tOutputType == OUTPUT_TYPE_SPI ? "spi": "dmx"), (int) m_tOutputType);
	}
#endif
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

bool E131Params::isMaskSet(uint32_t mask) const {
	return (m_bSetList & mask) == mask;
}
