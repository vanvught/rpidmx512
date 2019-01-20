/**
 * @file artnet4params.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "artnet4params.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)				(b) ? "Yes" : "No"

#define SET_MAP_UNIVERSE0_MASK		(1 << 0)

static const char PARAMS_FILE_NAME[] ALIGNED = "artnet.txt";
static const char PARAMS_MAP_UNIVERSE0[] ALIGNED = "map_universe0";

ArtNet4Params::ArtNet4Params(ArtNet4ParamsStore* pArtNet4ParamsStore): m_pArtNet4ParamsStore(pArtNet4ParamsStore)  {
	uint8_t *p = (uint8_t *) &m_tArtNet4Params;

	for (uint32_t i = 0; i < sizeof(struct TArtNet4Params); i++) {
		*p++ = 0;
	}

}

ArtNet4Params::~ArtNet4Params(void) {
}

bool ArtNet4Params::Load(void) {
	m_tArtNet4Params.nSetList = 0;

	ReadConfigFile configfile(ArtNet4Params::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNet4ParamsStore != 0) {
			m_pArtNet4ParamsStore->Update(&m_tArtNet4Params);
		}
	} else if (m_pArtNet4ParamsStore != 0) {
		m_pArtNet4ParamsStore->Copy(&m_tArtNet4Params);
	} else {
		return false;
	}

	return true;
}

void ArtNet4Params::Dump(void) {
#ifndef NDEBUG
	if (m_tArtNet4Params.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(SET_MAP_UNIVERSE0_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_MAP_UNIVERSE0, (int) m_tArtNet4Params.bMapUniverse0, BOOL2STRING(m_tArtNet4Params.bMapUniverse0));
	}

	#endif
}

void ArtNet4Params::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((ArtNet4Params *) p)->callbackFunction(s);
}

void ArtNet4Params::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_MAP_UNIVERSE0, &value8) == SSCAN_OK) {
		m_tArtNet4Params.bMapUniverse0 = (value8 != 0);
		m_tArtNet4Params.nSetList |= SET_MAP_UNIVERSE0_MASK;
		return;
	}
}

bool ArtNet4Params::isMaskSet(uint32_t nMask) const {
	return (m_tArtNet4Params.nSetList & nMask) == nMask;
}
