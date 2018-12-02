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
#include <string.h>
#include <uuid/uuid.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "e131params.h"
#include "e131.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_UNIVERSE_MASK		(1 << 0)
#define SET_MERGE_MODE_MASK		(1 << 1)
#define SET_OUTPUT_MASK			(1 << 2)
#define SET_CID_MASK			(1 << 3)
#define SET_UNIVERSE_A_MASK		(1 << 4)
#define SET_UNIVERSE_B_MASK		(1 << 5)
#define SET_UNIVERSE_C_MASK		(1 << 6)
#define SET_UNIVERSE_D_MASK		(1 << 7)
#define SET_MERGE_MODE_A_MASK	(1 << 8)
#define SET_MERGE_MODE_B_MASK	(1 << 9)
#define SET_MERGE_MODE_C_MASK	(1 << 10)
#define SET_MERGE_MODE_D_MASK	(1 << 11)

static const char PARAMS_FILE_NAME[] ALIGNED = "e131.txt";
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";
static const char PARAMS_MERGE_MODE[] ALIGNED = "merge_mode";
static const char PARAMS_OUTPUT[] ALIGNED = "output";
static const char PARAMS_CID[] ALIGNED = "cid";

E131Params::E131Params(E131ParamsStore *pE131ParamsStore):m_pE131ParamsStore(pE131ParamsStore) {
	uint8_t *p = (uint8_t *) &m_tE131Params;

	for (uint32_t i = 0; i < sizeof(struct TE131Params); i++) {
		*p++ = 0;
	}

	m_tE131Params.nUniverse = E131_UNIVERSE_DEFAULT;
}

E131Params::~E131Params(void) {
}

bool E131Params::Load(void) {
	m_tE131Params.nSetList = 0;

	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pE131ParamsStore != 0) {
			m_pE131ParamsStore->Update(&m_tE131Params);
		}
	} else if (m_pE131ParamsStore != 0) {
		m_pE131ParamsStore->Copy(&m_tE131Params);
	} else {
		return false;
	}

	return true;
}

void E131Params::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[UUID_STRING_LENGTH + 2];
	uint8_t len;
	uint16_t value16;

	if (Sscan::Uint16(pLine, PARAMS_UNIVERSE, &value16) == SSCAN_OK) {
		if ((value16 == 0) || (value16 > E131_UNIVERSE_MAX)) {
			m_tE131Params.nUniverse = E131_UNIVERSE_DEFAULT;
		} else {
			m_tE131Params.nUniverse = value16;
		}
		m_tE131Params.nSetList |= SET_UNIVERSE_MASK;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		if (memcmp(value, "spi", 3) == 0) {
			m_tE131Params.tOutputType = E131_OUTPUT_TYPE_SPI;
			m_tE131Params.nSetList |= SET_OUTPUT_MASK;
		} else if (memcmp(value, "mon", 3) == 0) {
			m_tE131Params.tOutputType = E131_OUTPUT_TYPE_MONITOR;
			m_tE131Params.nSetList |= SET_OUTPUT_MASK;
		} else {
			m_tE131Params.tOutputType = E131_OUTPUT_TYPE_DMX;
			m_tE131Params.nSetList |= SET_OUTPUT_MASK;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_MERGE_MODE, value, &len) == SSCAN_OK) {
		if (memcmp(value, "ltp", 3) == 0) {
			m_tE131Params.nMergeMode = E131_MERGE_LTP;
			m_tE131Params.nSetList |= SET_MERGE_MODE_MASK;
		} else if (memcmp(value, "htp", 3) == 0) {
			m_tE131Params.nMergeMode = E131_MERGE_HTP;
			m_tE131Params.nSetList |= SET_MERGE_MODE_MASK;
		}
		return;
	}

	len = UUID_STRING_LENGTH;
	if (Sscan::Uuid(pLine, PARAMS_CID, value, &len) == SSCAN_OK) {
		memcpy(m_tE131Params.aCidString, value, UUID_STRING_LENGTH);
		m_tE131Params.aCidString[UUID_STRING_LENGTH] = '\0';
		m_tE131Params.bHaveCustomCid = true;
		m_tE131Params.nSetList |= SET_CID_MASK;
		return;
	}
}

// FIXME E131Params::Set(E131Bridge *pE131Bridge)
void E131Params::Set(E131Bridge *pE131Bridge) {
	assert(pE131Bridge != 0);

	if (m_tE131Params.nSetList == 0) {
		return;
	}

	if (isMaskSet(SET_MERGE_MODE_MASK)) {
		pE131Bridge->SetMergeMode((TE131Merge) m_tE131Params.nMergeMode);
	}

}

void E131Params::Dump(void) {
#ifndef NDEBUG
	if (m_tE131Params.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_UNIVERSE_MASK)) {
		printf(" %s=%d\n", PARAMS_UNIVERSE, (int) m_tE131Params.nUniverse);
	}

	if (isMaskSet(SET_CID_MASK)) {
		printf(" %s=%s\n", PARAMS_CID, m_tE131Params.aCidString);
	}

	if (isMaskSet(SET_MERGE_MODE_MASK)) {
		printf(" %s=%s\n", PARAMS_MERGE_MODE, (TE131Merge)m_tE131Params.nMergeMode == E131_MERGE_HTP ? "HTP" : "LTP");
	}

	if (isMaskSet(SET_OUTPUT_MASK)) {
		printf(" %s=%s [%d]\n", PARAMS_OUTPUT, m_tE131Params.tOutputType == E131_OUTPUT_TYPE_MONITOR ? "mon" : (m_tE131Params.tOutputType == E131_OUTPUT_TYPE_SPI ? "spi": "dmx"), (int) m_tE131Params.tOutputType);
	}
#endif
}

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((E131Params *) p)->callbackFunction(s);
}

bool E131Params::isMaskSet(uint32_t nMask) const {
	return (m_tE131Params.nSetList & nMask) == nMask;
}
