/**
 * @file storeartnet.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "storeartnet.h"
#include "spiflashstore.h"

#include "artnetparams.h"

#include "debug.h"

StoreArtNet::StoreArtNet(void) {
	DEBUG_ENTRY

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

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aShortName), (void *)pShortName, ARTNET_SHORT_NAME_LENGTH, ArtNetParams::GetMaskShortName());

	DEBUG_EXIT
}

void StoreArtNet::SaveLongName(const char* pLongName) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, aLongName), (void *)pLongName, ARTNET_LONG_NAME_LENGTH, ArtNetParams::GetMaskLongName());

	DEBUG_EXIT
}

void StoreArtNet::SaveUniverseSwitch(uint8_t nPortIndex, uint8_t nAddress) {
	DEBUG_ENTRY
	assert(nPortIndex < ARTNET_MAX_PORTS);

	SpiFlashStore::Get()->Update(STORE_ARTNET, nPortIndex + __builtin_offsetof(struct TArtNetParams, nUniversePort), (void *)&nAddress, sizeof(uint8_t), ArtNetParams::GetMaskUniverse(nPortIndex));

	DEBUG_EXIT
}

void StoreArtNet::SaveNetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nNet), (void *)&nAddress, sizeof(uint8_t), ArtNetParams::GetMaskNet());

	DEBUG_EXIT
}

void StoreArtNet::SaveSubnetSwitch(uint8_t nAddress) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_ARTNET, __builtin_offsetof(struct TArtNetParams, nSubnet), (void *)&nAddress, sizeof(uint8_t), ArtNetParams::GetMaskSubnet());

	DEBUG_EXIT
}
