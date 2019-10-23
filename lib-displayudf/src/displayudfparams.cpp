/**
 * @file displayudfparams.cpp
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
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "displayudfparams.h"
#include "displayudfparamsconst.h"

#include "networkconst.h"
#include "lightsetconst.h"
#include "artnetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "artnetnode.h"
#include "e131bridge.h"
#include "display.h"

#include "debug.h"

static const char *pArray[DISPLAY_UDF_LABEL_UNKNOWN] = {
		DisplayUdfParamsConst::TITLE,
		DisplayUdfParamsConst::BOARD_NAME,
		NetworkConst::PARAMS_IP_ADDRESS,
		DisplayUdfParamsConst::VERSION,
		LightSetConst::PARAMS_UNIVERSE,
		DisplayUdfParamsConst::ACTIVE_PORTS,
		ArtNetParamsConst::NODE_SHORT_NAME,
		NetworkConst::PARAMS_HOSTNAME,
		ArtNetParamsConst::UNIVERSE_PORT[0],
		ArtNetParamsConst::UNIVERSE_PORT[1],
		ArtNetParamsConst::UNIVERSE_PORT[2],
		ArtNetParamsConst::UNIVERSE_PORT[3],
		NetworkConst::PARAMS_NET_MASK
};


DisplayUdfParams::DisplayUdfParams(DisplayUdfParamsStore *pDisplayUdfParamsStore): m_pDisplayUdfParamsStore(pDisplayUdfParamsStore) {
	DEBUG_ENTRY

	m_tDisplayUdfParams.nSetList = 0;

	DEBUG_EXIT
}

DisplayUdfParams::~DisplayUdfParams(void) {
	DEBUG_ENTRY

	m_tDisplayUdfParams.nSetList = 0;

	DEBUG_EXIT
}

bool DisplayUdfParams::Load(void) {
	m_tDisplayUdfParams.nSetList = 0;

	ReadConfigFile configfile(DisplayUdfParams::staticCallbackFunction, this);

	if (configfile.Read(DisplayUdfParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDisplayUdfParamsStore != 0) {
			m_pDisplayUdfParamsStore->Update(&m_tDisplayUdfParams);
		}
	} else if (m_pDisplayUdfParamsStore != 0) {
		m_pDisplayUdfParamsStore->Copy(&m_tDisplayUdfParams);
	} else {
		return false;
	}

	return true;
}

void DisplayUdfParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);

	assert(m_pDisplayUdfParamsStore != 0);

	if (m_pDisplayUdfParamsStore == 0) {
		return;
	}

	m_tDisplayUdfParams.nSetList = 0;

	ReadConfigFile config(DisplayUdfParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDisplayUdfParamsStore->Update(&m_tDisplayUdfParams);
}

void DisplayUdfParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::SLEEP_TIMEOUT, &m_tDisplayUdfParams.nSleepTimeout) == SSCAN_OK) {
		m_tDisplayUdfParams.nSetList |= DISPLAY_UDF_PARAMS_MASK_SLEEP_TIMEOUT;
		return;
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (Sscan::Uint8(pLine, pArray[i], &m_tDisplayUdfParams.nLabelIndex[i]) == SSCAN_OK) {
			m_tDisplayUdfParams.nSetList |= (1 << i);
			return;
		}
	}

}

void DisplayUdfParams::Builder(const struct TDisplayUdfParams *ptDisplayUdfParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (ptDisplayUdfParams != 0) {
		memcpy(&m_tDisplayUdfParams, ptDisplayUdfParams, sizeof(struct TDisplayUdfParams));
	} else {
		m_pDisplayUdfParamsStore->Copy(&m_tDisplayUdfParams);
	}

	PropertiesBuilder builder(DisplayUdfParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DisplayUdfParamsConst::SLEEP_TIMEOUT, m_tDisplayUdfParams.nSleepTimeout , isMaskSet(DISPLAY_UDF_PARAMS_MASK_SLEEP_TIMEOUT));

	for (uint32_t j = 1; j <= DISPLAY_LABEL_MAX_ROWS; j++) {
		for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
			if (j == m_tDisplayUdfParams.nLabelIndex[i]) {
				builder.Add(pArray[i], m_tDisplayUdfParams.nLabelIndex[i] , isMaskSet(1 << i));
			}
		}
	}

	nSize = builder.GetSize();

	DEBUG_EXIT
	return;
}

void DisplayUdfParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pDisplayUdfParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	return;
}

void DisplayUdfParams::Set(DisplayUdf *pDisplayUdf) {
	if (isMaskSet(DISPLAY_UDF_PARAMS_MASK_SLEEP_TIMEOUT)) {
		Display::Get()->SetSleepTimeout((uint32_t) m_tDisplayUdfParams.nSleepTimeout);
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (isMaskSet(1 << i)) {
			pDisplayUdf->Set(m_tDisplayUdfParams.nLabelIndex[i], (enum TDisplayUdfLabels) i);
		}
	}
}

void DisplayUdfParams::Dump(void) {
#ifndef NDEBUG
	if (m_tDisplayUdfParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DisplayUdfParamsConst::FILE_NAME);

	if (isMaskSet(DISPLAY_UDF_PARAMS_MASK_SLEEP_TIMEOUT)) {
		printf(" %s=%d\n", DisplayUdfParamsConst::SLEEP_TIMEOUT, m_tDisplayUdfParams.nSleepTimeout);
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (isMaskSet(1 << i)) {
			printf(" %s=%d\n", pArray[i], m_tDisplayUdfParams.nLabelIndex[i]);
		}
	}

#endif
}

void DisplayUdfParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((DisplayUdfParams *) p)->callbackFunction(s);
}

bool DisplayUdfParams::isMaskSet(uint32_t nMask) const {
	return (m_tDisplayUdfParams.nSetList & nMask) == nMask;
}
