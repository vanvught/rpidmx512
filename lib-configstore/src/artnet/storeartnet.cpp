/**
 * @file storeartnet.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "configstore.h"

#include "debug.h"

uint32_t StoreArtNet::s_nPortIndexOffset;
StoreArtNet *StoreArtNet::s_pThis;

StoreArtNet::StoreArtNet(uint32_t nPortIndexOffset) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	s_nPortIndexOffset = nPortIndexOffset;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_EXIT
}

void StoreArtNet::SaveUniverseSwitch(uint32_t nPortIndex, uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_nPortIndexOffset=%u, nPortIndex=%u, nAddress=%u", s_nPortIndexOffset, nPortIndex, static_cast<uint32_t>(nAddress));

	if (nPortIndex >= s_nPortIndexOffset) {
		nPortIndex -= s_nPortIndexOffset;
	} else {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	if (nPortIndex >= artnet::PORTS) {
		DEBUG_EXIT
		return;
	}

	ConfigStore::Get()->Update(configstore::Store::ARTNET, nPortIndex + __builtin_offsetof(struct artnetparams::Params, nUniversePort), &nAddress, sizeof(uint8_t), artnetparams::Mask::UNIVERSE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SaveNetSwitch(uint32_t nPage, uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPage=%u, nAddress=%u", nPage, static_cast<uint32_t>(nAddress));

	if (nPage > 0) {
		DEBUG_EXIT
		return;
	}

	ConfigStore::Get()->Update(configstore::Store::ARTNET, __builtin_offsetof(struct artnetparams::Params, nNet), &nAddress, sizeof(uint8_t), artnetparams::Mask::NET);

	DEBUG_EXIT
}

void StoreArtNet::SaveSubnetSwitch(uint32_t nPage, uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPage=%u, nAddress=%u", nPage, static_cast<uint32_t>(nAddress));

	if (nPage > 0) {
		DEBUG_EXIT
		return;
	}

	ConfigStore::Get()->Update(configstore::Store::ARTNET, __builtin_offsetof(struct artnetparams::Params, nSubnet), &nAddress, sizeof(uint8_t), artnetparams::Mask::SUBNET);

	DEBUG_EXIT
}

void StoreArtNet::SaveMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_nPortIndexOffset=%u, nPortIndex=%u, mergeMode=%u", s_nPortIndexOffset, nPortIndex, static_cast<uint32_t>(mergeMode));

	if (nPortIndex >= s_nPortIndexOffset) {
		nPortIndex -= s_nPortIndexOffset;
	} else {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	if (nPortIndex >= artnet::PORTS) {
		DEBUG_EXIT
		return;
	}

	ConfigStore::Get()->Update(configstore::Store::ARTNET, nPortIndex + __builtin_offsetof(struct artnetparams::Params, nMergeModePort), &mergeMode, sizeof(uint8_t), artnetparams::Mask::MERGE_MODE_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SavePortProtocol(uint32_t nPortIndex, artnet::PortProtocol portProtocol) {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_nPortIndexOffset=%u, nPortIndex=%u, portProtocol=%u", s_nPortIndexOffset, nPortIndex, static_cast<uint32_t>(portProtocol));

	if (nPortIndex >= s_nPortIndexOffset) {
		nPortIndex -= s_nPortIndexOffset;
	} else {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	if (nPortIndex >= artnet::PORTS) {
		DEBUG_EXIT
		return;
	}

	ConfigStore::Get()->Update(configstore::Store::ARTNET, nPortIndex + __builtin_offsetof(struct artnetparams::Params, nProtocolPort), &portProtocol, sizeof(uint8_t), artnetparams::Mask::PROTOCOL_A << nPortIndex);

	DEBUG_EXIT
}

void StoreArtNet::SaveRdmEnabled(uint32_t nPortIndex, bool isEnabled) {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_nPortIndexOffset=%u, nPortIndex=%u, isEnabled=%d", s_nPortIndexOffset, nPortIndex, isEnabled);

	if (nPortIndex >= s_nPortIndexOffset) {
		nPortIndex -= s_nPortIndexOffset;
	} else {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	if (nPortIndex >= artnet::PORTS) {
		DEBUG_EXIT
		return;
	}

	uint16_t nRdm;
	ConfigStore::Get()->Copy(configstore::Store::ARTNET, &nRdm, sizeof(uint16_t), __builtin_offsetof(struct artnetparams::Params, nRdm), false);

#if __GNUC__ < 10
/*
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
  nRdm &= artnetparams::clear_mask(nPortIndex);
 */
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wconversion"	// FIXME ignored "-Wconversion"
#endif

	nRdm &= artnetparams::clear_mask(nPortIndex);

	if (isEnabled) {
/*
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
   nRdm |= artnetparams::shift_left(1, nPortIndex);
 */
		nRdm |= artnetparams::shift_left(1, nPortIndex);
		nRdm |= static_cast<uint16_t>(1U << (nPortIndex + 8));
	}

#if __GNUC__ < 10
# pragma GCC diagnostic pop
#endif

	ConfigStore::Get()->Update(configstore::Store::ARTNET, __builtin_offsetof(struct artnetparams::Params, nRdm), &nRdm, sizeof(uint16_t));

	DEBUG_EXIT
}
