/**
 * @file storeartnet.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "spiflashstore.h"

#include "artnetstore.h"
#include "artnetparams.h"

#include "debug.h"

ArtNetStore::~ArtNetStore(void) {
}

ArtNetParamsStore::~ArtNetParamsStore(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

StoreArtNet::StoreArtNet(void) {
	DEBUG_ENTRY

	DEBUG_PRINTF("%p", this);

	DEBUG_EXIT
}

StoreArtNet::~StoreArtNet(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void StoreArtNet::Update(const struct TArtNetParams* pArtNetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, (void *)pArtNetParams, sizeof(struct TArtNetParams));

	DEBUG_EXIT
}

void StoreArtNet::Copy(struct TArtNetParams* pArtNetParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Copy(STORE_ARTNET, (void *)pArtNetParams, sizeof(struct TArtNetParams));

	DEBUG_EXIT
}

void StoreArtNet::SaveShortName(const char* pShortName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aShortName), (void *)pShortName, ARTNET_SHORT_NAME_LENGTH, ARTNET_PARAMS_MASK_SHORT_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveLongName(const char* pLongName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aLongName), (void *)pLongName, ARTNET_LONG_NAME_LENGTH, ARTNET_PARAMS_MASK_LONG_NAME);

	DEBUG_EXIT
}

void StoreArtNet::SaveUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress) {
	DEBUG_ENTRY
	assert(nPortIndex < ARTNET_MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nUniverse), (void *)&nAddress, sizeof(uint8_t), ARTNET_PARAMS_MASK_UNIVERSE);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nUniversePort), (void *)&nAddress, sizeof(uint8_t), ARTNET_PARAMS_MASK_UNIVERSE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SaveNetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nNet), (void *)&nAddress, sizeof(uint8_t), ARTNET_PARAMS_MASK_NET);

	DEBUG_EXIT
}

void StoreArtNet::SaveSubnetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nSubnet), (void *)&nAddress, sizeof(uint8_t), ARTNET_PARAMS_MASK_SUBNET);

	DEBUG_EXIT
}

void StoreArtNet::SaveMergeMode(uint8_t nPortIndex, TMerge tMerge) {
	DEBUG_ENTRY
	assert(nPortIndex < ARTNET_MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nMergeMode), (void *)&tMerge, sizeof(TMerge), ARTNET_PARAMS_MASK_MERGE_MODE);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nMergeModePort), (void *)&tMerge, sizeof(TMerge), ARTNET_PARAMS_MASK_MERGE_MODE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SavePortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
	DEBUG_ENTRY
	assert(nPortIndex < ARTNET_MAX_PORTS);

	if (nPortIndex == 0) {
		SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nProtocol), (void *)&tPortProtocol, sizeof(TPortProtocol), ARTNET_PARAMS_MASK_PROTOCOL);
	}

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nProtocolPort), (void *)&tPortProtocol, sizeof(TPortProtocol), ARTNET_PARAMS_MASK_PROTOCOL_A << nPortIndex);

	DEBUG_EXIT
}
