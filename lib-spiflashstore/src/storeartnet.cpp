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

#include <cstdint>
#include <cassert>

#include "storeartnet.h"

#include "artnetstore.h"
#include "artnetparams.h"
#include "artnet.h"

#include "spiflashstore.h"

#include "debug.h"

using namespace spiflashstore;
using namespace artnet;
using namespace artnetparams;

StoreArtNet *StoreArtNet::s_pThis = nullptr;

StoreArtNet::StoreArtNet() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_EXIT
}

void StoreArtNet::SaveShortName(const char* pShortName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, aShortName), pShortName, ArtNet::SHORT_NAME_LENGTH, Mask::SHORT_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveLongName(const char* pLongName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, aLongName), pLongName, ArtNet::LONG_NAME_LENGTH, Mask::LONG_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveUniverseSwitch(uint32_t nPortIndex, uint8_t nAddress) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::PORTS);

	SpiFlashStore::Get()->Update(Store::ARTNET, nPortIndex + __builtin_offsetof(struct Params, nUniversePort), &nAddress, sizeof(uint8_t), Mask::UNIVERSE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SaveNetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, nNet), &nAddress, sizeof(uint8_t), Mask::NET);

	DEBUG_EXIT
}

void StoreArtNet::SaveSubnetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, nSubnet), &nAddress, sizeof(uint8_t), Mask::SUBNET);

	DEBUG_EXIT
}

void StoreArtNet::SaveMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, nMergeMode), &mergeMode, sizeof(uint8_t), Mask::MERGE_MODE);
	}

	SpiFlashStore::Get()->Update(Store::ARTNET, nPortIndex + __builtin_offsetof(struct Params, nMergeModePort), &mergeMode, sizeof(uint8_t), Mask::MERGE_MODE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SavePortProtocol(uint32_t nPortIndex, PortProtocol tPortProtocol) {
	DEBUG_ENTRY
	assert(nPortIndex < ArtNet::PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(Store::ARTNET, __builtin_offsetof(struct Params, nProtocol), &tPortProtocol, sizeof(uint8_t), Mask::PROTOCOL);
	}

	SpiFlashStore::Get()->Update(Store::ARTNET, nPortIndex + __builtin_offsetof(struct Params, nProtocolPort), &tPortProtocol, sizeof(uint8_t), Mask::PROTOCOL_A << nPortIndex);

	DEBUG_EXIT
}
