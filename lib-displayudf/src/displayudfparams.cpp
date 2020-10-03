/**
 * @file displayudfparams.cpp
 *
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
# include <stdio.h>
#endif
#include <cassert>

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

//static constexpr const char UNUSED[] = "not_used";

static constexpr const char *pArray[DISPLAY_UDF_LABEL_UNKNOWN] = {
		DisplayUdfParamsConst::TITLE,
		DisplayUdfParamsConst::BOARD_NAME,
		NetworkConst::PARAMS_IP_ADDRESS,
		DisplayUdfParamsConst::VERSION,
		LightSetConst::PARAMS_UNIVERSE,
		DisplayUdfParamsConst::ACTIVE_PORTS,
		ArtNetParamsConst::NODE_SHORT_NAME,
		NetworkConst::PARAMS_HOSTNAME,
		LightSetConst::PARAMS_UNIVERSE_PORT[0],
		LightSetConst::PARAMS_UNIVERSE_PORT[1],
		LightSetConst::PARAMS_UNIVERSE_PORT[2],
		LightSetConst::PARAMS_UNIVERSE_PORT[3],
		NetworkConst::PARAMS_NET_MASK,
		LightSetConst::PARAMS_DMX_START_ADDRESS,
		ArtNetParamsConst::DESTINATION_IP_PORT[0],
		ArtNetParamsConst::DESTINATION_IP_PORT[1],
		ArtNetParamsConst::DESTINATION_IP_PORT[2],
		ArtNetParamsConst::DESTINATION_IP_PORT[3]
};

DisplayUdfParams::DisplayUdfParams(DisplayUdfParamsStore *pDisplayUdfParamsStore): m_pDisplayUdfParamsStore(pDisplayUdfParamsStore) {
	DEBUG_ENTRY

	memset(&m_tDisplayUdfParams, 0, sizeof(struct TDisplayUdfParams));
	m_tDisplayUdfParams.nSleepTimeout = DISPLAY_SLEEP_TIMEOUT_DEFAULT;

	DEBUG_EXIT
}

bool DisplayUdfParams::Load() {
	m_tDisplayUdfParams.nSetList = 0;

	ReadConfigFile configfile(DisplayUdfParams::staticCallbackFunction, this);

	if (configfile.Read(DisplayUdfParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDisplayUdfParamsStore != nullptr) {
			m_pDisplayUdfParamsStore->Update(&m_tDisplayUdfParams);
		}
	} else if (m_pDisplayUdfParamsStore != nullptr) {
		m_pDisplayUdfParamsStore->Copy(&m_tDisplayUdfParams);
	} else {
		return false;
	}

	return true;
}

void DisplayUdfParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pDisplayUdfParamsStore != nullptr);

	if (m_pDisplayUdfParamsStore == nullptr) {
		return;
	}

	m_tDisplayUdfParams.nSetList = 0;

	ReadConfigFile config(DisplayUdfParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDisplayUdfParamsStore->Update(&m_tDisplayUdfParams);
}

void DisplayUdfParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);
	uint8_t value8;

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::SLEEP_TIMEOUT, value8) == Sscan::OK) {
		if (value8 != DISPLAY_SLEEP_TIMEOUT_DEFAULT) {
			m_tDisplayUdfParams.nSleepTimeout = value8;
			m_tDisplayUdfParams.nSetList |= DisplayUdfParamsMask::SLEEP_TIMEOUT;
		} else {
			m_tDisplayUdfParams.nSleepTimeout = DISPLAY_SLEEP_TIMEOUT_DEFAULT;
			m_tDisplayUdfParams.nSetList &= ~DisplayUdfParamsMask::SLEEP_TIMEOUT;
		}
		return;
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (Sscan::Uint8(pLine, pArray[i], value8) == Sscan::OK) {
			m_tDisplayUdfParams.nLabelIndex[i] = value8;
			m_tDisplayUdfParams.nSetList |= (1U << i);
			return;
		}
	}
}

void DisplayUdfParams::Builder(const struct TDisplayUdfParams *ptDisplayUdfParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptDisplayUdfParams != nullptr) {
		memcpy(&m_tDisplayUdfParams, ptDisplayUdfParams, sizeof(struct TDisplayUdfParams));
	} else {
		m_pDisplayUdfParamsStore->Copy(&m_tDisplayUdfParams);
	}

	PropertiesBuilder builder(DisplayUdfParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DisplayUdfParamsConst::SLEEP_TIMEOUT, m_tDisplayUdfParams.nSleepTimeout , isMaskSet(DisplayUdfParamsMask::SLEEP_TIMEOUT));

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (!isMaskSet(1U << i)) {
			m_tDisplayUdfParams.nLabelIndex[i] = DisplayUdf::Get()->GetLabel(i);
		}
		builder.Add(pArray[i], m_tDisplayUdfParams.nLabelIndex[i] , isMaskSet(1U << i));
	}

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void DisplayUdfParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pDisplayUdfParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void DisplayUdfParams::Set(DisplayUdf *pDisplayUdf) {
	if (isMaskSet(DisplayUdfParamsMask::SLEEP_TIMEOUT)) {
		Display::Get()->SetSleepTimeout(m_tDisplayUdfParams.nSleepTimeout);
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (isMaskSet(1U << i)) {
			pDisplayUdf->Set(m_tDisplayUdfParams.nLabelIndex[i], static_cast<enum TDisplayUdfLabels>(i));
		}
	}
}

void DisplayUdfParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DisplayUdfParams*>(p))->callbackFunction(s);
}

void DisplayUdfParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DisplayUdfParamsConst::FILE_NAME);

	if (isMaskSet(DisplayUdfParamsMask::SLEEP_TIMEOUT)) {
		printf(" %s=%d\n", DisplayUdfParamsConst::SLEEP_TIMEOUT, m_tDisplayUdfParams.nSleepTimeout);
	}

	for (uint32_t i = 0; i < DISPLAY_UDF_LABEL_UNKNOWN; i++) {
		if (isMaskSet(1U << i)) {
			printf(" %s=%d\n", pArray[i], m_tDisplayUdfParams.nLabelIndex[i]);
		}
	}

#endif
}
