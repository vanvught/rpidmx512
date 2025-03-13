/**
 * @file displayudfparams.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_DISPLAYUDF)
# undef NDEBUG
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

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayudfparamsconst.h"

#include "networkparamsconst.h"
#include "dmxnodeparamsconst.h"

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
#endif
#if defined (NODE_E131)
# include "e131bridge.h"
#endif

#include "debug.h"

using namespace displayudf;

static const char *pArray[static_cast<uint32_t>(Labels::UNKNOWN)] = {
		DisplayUdfParamsConst::TITLE,
		DisplayUdfParamsConst::BOARD_NAME,
		DisplayUdfParamsConst::VERSION,
		NetworkParamsConst::HOSTNAME,
		NetworkParamsConst::IP_ADDRESS,
		NetworkParamsConst::NET_MASK,
		NetworkParamsConst::DEFAULT_GATEWAY,
		DisplayUdfParamsConst::ACTIVE_PORTS,
		DisplayUdfParamsConst::DMX_DIRECTION,
		DmxNodeParamsConst::DMX_START_ADDRESS,
		DmxNodeParamsConst::UNIVERSE_PORT[0],
# if (DMX_MAX_PORTS > 1)
		DmxNodeParamsConst::UNIVERSE_PORT[1],
# endif
# if (DMX_MAX_PORTS > 2)
		DmxNodeParamsConst::UNIVERSE_PORT[2],
# endif
# if (DMX_MAX_PORTS == 4)
		DmxNodeParamsConst::UNIVERSE_PORT[3],
# endif
#if defined (NODE_ARTNET) && defined (ARTNET_HAVE_DMXIN)
		DmxNodeParamsConst::DESTINATION_IP_PORT[0],
# if DMX_MAX_PORTS >= 2
		DmxNodeParamsConst::DESTINATION_IP_PORT[1],
# endif
# if DMX_MAX_PORTS >= 3
		DmxNodeParamsConst::DESTINATION_IP_PORT[2],
# endif
# if DMX_MAX_PORTS == 4
		DmxNodeParamsConst::DESTINATION_IP_PORT[3],
# endif
#endif
};

namespace displayudf::store {
	static void Update(const struct displayudfparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::DISPLAYUDF, pParams, sizeof(struct displayudfparams::Params));
	}

	static void Copy(struct displayudfparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::DISPLAYUDF, pParams, sizeof(struct displayudfparams::Params));
	}
}  // namespace displayudf::store

DisplayUdfParams::DisplayUdfParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void DisplayUdfParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DisplayUdfParams::StaticCallbackFunction, this);

	if (configfile.Read(DisplayUdfParamsConst::FILE_NAME)) {
		displayudf::store::Update(&m_Params);
	} else
#endif
		displayudf::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DisplayUdfParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(m_Params));

	ReadConfigFile config(DisplayUdfParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	displayudf::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DisplayUdfParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);
	uint8_t value8;

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::INTENSITY, value8) == Sscan::OK) {
		m_Params.nIntensity = value8;
		m_Params.nSetList |= displayudfparams::Mask::INTENSITY;
		return;
	}

	if (Sscan::Uint8(pLine, DisplayUdfParamsConst::SLEEP_TIMEOUT, value8) == Sscan::OK) {
		m_Params.nSleepTimeout = value8;

		if (value8 != display::Defaults::SLEEP_TIMEOUT) {
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

void DisplayUdfParams::Builder(const struct displayudfparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct displayudfparams::Params));
	} else {
		displayudf::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(DisplayUdfParamsConst::FILE_NAME, pBuffer, nLength);

	if (!IsMaskSet(displayudfparams::Mask::INTENSITY)) {
		m_Params.nIntensity = defaults::INTENSITY;
	}

	builder.Add(DisplayUdfParamsConst::INTENSITY, m_Params.nIntensity);

	if (!IsMaskSet(displayudfparams::Mask::SLEEP_TIMEOUT)) {
		m_Params.nSleepTimeout = display::Defaults::SLEEP_TIMEOUT;
	}

	builder.Add(DisplayUdfParamsConst::SLEEP_TIMEOUT, m_Params.nSleepTimeout);

	builder.Add(DisplayUdfParamsConst::FLIP_VERTICALLY, IsMaskSet(displayudfparams::Mask::FLIP_VERTICALLY));

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (pArray[i][0] != '\0') {
			builder.Add(pArray[i], m_Params.nLabelIndex[i] , IsMaskSet(1U << i));
		}
	}

	nSize = builder.GetSize();
}

void DisplayUdfParams::Set(DisplayUdf *pDisplayUdf) {
	assert(pDisplayUdf != nullptr);

	if (IsMaskSet(displayudfparams::Mask::INTENSITY)) {
		pDisplayUdf->SetContrast(m_Params.nIntensity);
	}

	pDisplayUdf->SetSleepTimeout(m_Params.nSleepTimeout);
	pDisplayUdf->SetFlipVertically(IsMaskSet(displayudfparams::Mask::FLIP_VERTICALLY));

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (IsMaskSet(1U << i)) {
			pDisplayUdf->Set(m_Params.nLabelIndex[i], static_cast<Labels>(i));
		}
	}
}

void DisplayUdfParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DisplayUdfParams*>(p))->CallbackFunction(s);
}

void DisplayUdfParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DisplayUdfParamsConst::FILE_NAME);

	printf(" %s=%d\n", DisplayUdfParamsConst::INTENSITY, m_Params.nIntensity);
	printf(" %s=%d\n", DisplayUdfParamsConst::SLEEP_TIMEOUT, m_Params.nSleepTimeout);

	if (IsMaskSet(displayudfparams::Mask::FLIP_VERTICALLY)) {
		printf(" Flip vertically\n");
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(Labels::UNKNOWN); i++) {
		if (IsMaskSet(1U << i)) {
			printf(" %s=%d\n", pArray[i], m_Params.nLabelIndex[i]);
		}
	}
}
