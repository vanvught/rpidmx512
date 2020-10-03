/**
 * @file storeartnet.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "storeartnet.h"

#include "artnetstore.h"
#include "artnetparams.h"
#include "artnet.h"

#include "spiflashstore.h"

#include "debug.h"

StoreArtNet *StoreArtNet::s_pThis = nullptr;

StoreArtNet::StoreArtNet() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_EXIT
}

void StoreArtNet::Update(const struct TArtNetParams* pArtNetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, pArtNetParams, sizeof(struct TArtNetParams));

	DEBUG_EXIT
}

void StoreArtNet::Copy(struct TArtNetParams* pArtNetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Copy(STORE_ARTNET, pArtNetParams, sizeof(struct TArtNetParams));

	DEBUG_EXIT
}

void StoreArtNet::SaveShortName(const char* pShortName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aShortName), pShortName, ArtNet::SHORT_NAME_LENGTH, ArtnetParamsMask::SHORT_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveLongName(const char* pLongName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aLongName), pLongName, ArtNet::LONG_NAME_LENGTH, ArtnetParamsMask::LONG_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nUniverse), &nAddress, sizeof(uint8_t), ArtnetParamsMask::UNIVERSE);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nUniversePort), &nAddress, sizeof(uint8_t), ArtnetParamsMask::UNIVERSE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SaveNetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nNet), &nAddress, sizeof(uint8_t), ArtnetParamsMask::NET);

	DEBUG_EXIT
}

void StoreArtNet::SaveSubnetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nSubnet), &nAddress, sizeof(uint8_t), ArtnetParamsMask::SUBNET);

	DEBUG_EXIT
}

void StoreArtNet::SaveMergeMode(uint8_t nPortIndex, ArtNetMerge tMerge) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nMergeMode), &tMerge, sizeof(ArtNetMerge), ArtnetParamsMask::MERGE_MODE);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nMergeModePort), &tMerge, sizeof(ArtNetMerge), ArtnetParamsMask::MERGE_MODE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SavePortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nProtocol), &tPortProtocol, sizeof(TPortProtocol), ArtnetParamsMask::PROTOCOL);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nProtocolPort), &tPortProtocol, sizeof(TPortProtocol), ArtnetParamsMask::PROTOCOL_A << nPortIndex);

	DEBUG_EXIT
}
