/**
 * @file displayudfparams.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#if defined (NODE_ARTNET_MULTI)
# define NODE_ARTNET
#endif

#if defined (NODE_E131_MULTI)
# define NODE_E131
#endif

#include "displayudfparams.h"
#include "displayudfparamsconst.h"

#include "networkparamsconst.h"
#include "lightsetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "display.h"

#if defined (NODE_NODE)
# include "node.h"
# include "nodeparamsconst.h"
#endif
#if defined (NODE_ARTNET)
# include "artnetnode.h"
# include "artnetparamsconst.h"
#endif
#if defined (NODE_E131)
# include "e131bridge.h"
#endif

#include "debug.h"

using namespace displayudf;

#if !defined (NODE_NODE)
static constexpr const char *pArray[static_cast<uint32_t>(Labels::UNKNOWN)] = {
		DisplayUdfParamsConst::TITLE,
		DisplayUdfParamsConst::BOARD_NAME,
		NetworkParamsConst::IP_ADDRESS,
		DisplayUdfParamsConst::VERSION,
		"",
		DisplayUdfParamsConst::ACTIVE_PORTS,
		"",
		NetworkParamsConst::HOSTNAME,
		LightSetParamsConst::UNIVERSE_PORT[0],
		LightSetParamsConst::UNIVERSE_PORT[1],
		LightSetParamsConst::UNIVERSE_PORT[2],
		LightSetParamsConst::UNIVERSE_PORT[3],
		NetworkParamsConst::NET_MASK,
		LightSetParamsConst::DMX_START_ADDRESS,
#if defined (NODE_ARTNET)
		ArtNetParamsConst::DESTINATION_IP_PORT[0],
		ArtNetParamsConst::DESTINATION_IP_PORT[1],
		ArtNetParamsConst::DESTINATION_IP_PORT[2],
		ArtNetParamsConst::DESTINATION_IP_PORT[3],
#else
		"",
		"",
		"",
		"",
#endif
		NetworkParamsConst::DEFAULT_GATEWAY,
		DisplayUdfParamsConst::DMX_DIRECTION
};
#else
# if LIGHTSET_PORTS > 8
#  define MAX_ARRAY 4
# else
#  define MAX_ARRAY LIGHTSET_PORTS
# endif
static constexpr const char *pArray[static_cast<uint32_t>(Labels::UNKNOWN)] = {
		DisplayUdfParamsConst::TITLE,
		DisplayUdfParamsConst::BOARD_NAME,
		DisplayUdfParamsConst::VERSION,
		NetworkParamsConst::HOSTNAME,
		NetworkParamsConst::IP_ADDRESS,
		NetworkParamsConst::NET_MASK,
		NetworkParamsConst::DEFAULT_GATEWAY,
		NodeParamsConst::UNIVERSE_PORT[0],
# if MAX_ARRAY >= 2
		NodeParamsConst::UNIVERSE_PORT[1],
# endif
# if MAX_ARRAY >= 3
		NodeParamsConst::UNIVERSE_PORT[2],
# endif
# if MAX_ARRAY == 4
		NodeParamsConst::UNIVERSE_PORT[3],
# endif
		NodeParamsConst::DESTINATION_IP_PORT[0],
# if MAX_ARRAY >= 2
		NodeParamsConst::DESTINATION_IP_PORT[1],
# endif
# if MAX_ARRAY >= 3
		NodeParamsConst::DESTINATION_IP_PORT[2],
# endif
# if MAX_ARRAY == 4
		NodeParamsConst::DESTINATION_IP_PORT[3]
# endif
};
# undef MAX_ARRAY
#endif

DisplayUdfParams::DisplayUdfParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct displayudfparams::Params));
	m_Params.nSleepTimeout = display::Defaults::SEEP_TIMEOUT;
	m_Params.nIntensity = defaults::INTENSITY;

	DEBUG_EXIT
}

void DisplayUdfParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DisplayUdfParams::staticCallbackFunction, this);

	if (configfile.Read(DisplayUdfParamsConst::FILE_NAME)) {
		DisplayUdfParamsStore::Update(&m_Params);
	} else
#endif
		DisplayUdfParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DisplayUdfParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(DisplayUdfParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	DisplayUdfParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DisplayUdfParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);
	uint8_t value8;

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::INTENSITY, value8) == Sscan::OK) {
		m_Params.nIntensity = value8;

		if (value8 != defaults::INTENSITY) {
			m_Params.nSetList |= displayudfparams::Mask::INTENSITY;
		} else {
			m_Params.nSetList &= ~displayudfparams::Mask::INTENSITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::SLEEP_TIMEOUT, value8) == Sscan::OK) {
		m_Params.nSleepTimeout = value8;

		if (value8 != display::Defaults::SEEP_TIMEOUT) {
			m_Params.nSetList |= displayudfparams::Mask::SLEEP_TIMEOUT;
		} else {
			m_Params.nSetList &= ~displayudfparams::Mask::SLEEP_TIMEOUT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::FLIP_VERTICALLY, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_Params.nSetList |= displayudfparams::Mask::FLIP_VERTICALLY;
		} else {
			m_Params.nSetList &= ~displayudfparams::Mask::FLIP_VERTICALLY;
		}
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (Sscan::Uint8(pLine, pArray[i], value8) == Sscan::OK) {
			if ((value8 > 0) && (value8 <= LABEL_MAX_ROWS)) {
				m_Params.nLabelIndex[i] = value8;
				m_Params.nSetList |= (1U << i);
			} else {
				m_Params.nLabelIndex[i] = 0;
				m_Params.nSetList &= ~(1U << i);
			}
			return;
		}
	}
}

void DisplayUdfParams::Builder(const struct displayudfparams::Params *ptDisplayUdfParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (ptDisplayUdfParams != nullptr) {
		memcpy(&m_Params, ptDisplayUdfParams, sizeof(struct displayudfparams::Params));
	} else {
		assert(m_pDisplayUdfParamsStore != nullptr);
		DisplayUdfParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(DisplayUdfParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DisplayUdfParamsConst::INTENSITY, m_Params.nIntensity , isMaskSet(displayudfparams::Mask::INTENSITY));
	builder.Add(DisplayUdfParamsConst::SLEEP_TIMEOUT, m_Params.nSleepTimeout , isMaskSet(displayudfparams::Mask::SLEEP_TIMEOUT));
	builder.Add(DisplayUdfParamsConst::FLIP_VERTICALLY, isMaskSet(displayudfparams::Mask::FLIP_VERTICALLY) , isMaskSet(displayudfparams::Mask::FLIP_VERTICALLY));

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (pArray[i][0] != '\0') {
			builder.Add(pArray[i], m_Params.nLabelIndex[i] , isMaskSet(1U << i));
		}
	}

	nSize = builder.GetSize();
}

void DisplayUdfParams::Set(DisplayUdf *pDisplayUdf) {
	assert(pDisplayUdf != nullptr);

	if (isMaskSet(displayudfparams::Mask::INTENSITY)) {
		pDisplayUdf->SetContrast(m_Params.nIntensity);
	}

	if (isMaskSet(displayudfparams::Mask::SLEEP_TIMEOUT)) {
		pDisplayUdf->SetSleepTimeout(m_Params.nSleepTimeout);
	}

	pDisplayUdf->SetFlipVertically(isMaskSet(displayudfparams::Mask::FLIP_VERTICALLY));

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (isMaskSet(1U << i)) {
			pDisplayUdf->Set(m_Params.nLabelIndex[i], static_cast<Labels>(i));
		}
	}
}

void DisplayUdfParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DisplayUdfParams*>(p))->callbackFunction(s);
}

void DisplayUdfParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DisplayUdfParamsConst::FILE_NAME);

	if (isMaskSet(displayudfparams::Mask::INTENSITY)) {
		printf(" %s=%d\n", DisplayUdfParamsConst::INTENSITY, m_Params.nIntensity);
	}

	if (isMaskSet(displayudfparams::Mask::SLEEP_TIMEOUT)) {
		printf(" %s=%d\n", DisplayUdfParamsConst::SLEEP_TIMEOUT, m_Params.nSleepTimeout);
	}

	if (isMaskSet(displayudfparams::Mask::FLIP_VERTICALLY)) {
		printf(" Flip vertically\n");
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (isMaskSet(1U << i)) {
			printf(" %s=%d\n", pArray[i], m_Params.nLabelIndex[i]);
		}
	}
}
