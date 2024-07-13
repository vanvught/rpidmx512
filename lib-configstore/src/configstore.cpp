/**
 * @file configstore.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "configstore.h"

#include "hardware.h"

#include "platform_configstore.h"

#include "debug.h"

namespace global {
extern int32_t *gp_nUtcOffset;
}  // namespace global

using namespace configstore;

static constexpr uint8_t s_aSignature[] = {'A', 'v', 'V', 0x01};
static constexpr uint32_t s_aStorSize[static_cast<uint32_t>(Store::LAST)]  = {96,        32,    64,      64,    32,     32,        480,          64,         32,        96,           48,        32,      944,          48,        64,            32,        96,         32,      1024,     32,     32,       64,            96,               32,    32,          320,    32};
#ifndef NDEBUG
static constexpr char s_aStoreName[static_cast<uint32_t>(Store::LAST)][16] = {"Network", "DMX", "Pixel", "LTC", "MIDI", "LTC ETC", "OSC Server", "TLC59711", "USB Pro", "RDM Device", "RConfig", "TCNet", "OSC Client", "Display", "LTC Display", "Monitor", "SparkFun", "Slush", "Motors", "Show", "Serial", "RDM Sensors", "RDM SubDevices", "GPS", "RGB Panel", "Node", "PCA9685"};
#endif

bool ConfigStore::s_bHaveFlashChip;
State ConfigStore::s_State;
uint32_t ConfigStore::s_nStartAddress;
uint32_t ConfigStore::s_nSpiFlashStoreSize;
uint32_t ConfigStore::s_nWaitMillis;
uint8_t ConfigStore::s_SpiFlashData[FlashStore::SIZE] SECTION_CONFIGSTORE;

ConfigStore *ConfigStore::s_pThis;

ConfigStore::ConfigStore() {
	DEBUG_ENTRY

	static_assert(sizeof(s_aSignature) <= FlashStore::SIGNATURE_SIZE);

	assert(s_pThis == nullptr);
	s_pThis = this;

	global::gp_nUtcOffset = reinterpret_cast<int32_t *>(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE]);

	s_bHaveFlashChip = StoreDevice::IsDetected();

	assert(FlashStore::SIZE <= StoreDevice::GetSize());

	const auto nEraseSize = StoreDevice::GetSectorSize();
	assert(nEraseSize <= FlashStore::SIZE);

	const auto nSectors = FlashStore::SIZE / nEraseSize;

	DEBUG_PRINTF("KB_NEEDED=%u, nEraseSize=%u, nSectors=%u", FlashStore::SIZE, nEraseSize, nSectors);

	assert((nSectors * nEraseSize) <= StoreDevice::GetSize());

	s_nStartAddress = StoreDevice::GetSize() - (nSectors * nEraseSize);

	DEBUG_PRINTF("s_nStartAddress=%p", reinterpret_cast<void *>(s_nStartAddress));

	if (s_bHaveFlashChip) {
		storedevice::result result;
		StoreDevice::Read(s_nStartAddress, FlashStore::SIZE, reinterpret_cast<uint8_t *>(&s_SpiFlashData), result);
		assert(result == storedevice::result::OK);
	}

	bool bSignatureOK = true;

	for (uint32_t i = 0; i < sizeof(s_aSignature); i++) {
		if (s_aSignature[i] != s_SpiFlashData[i]) {
			s_SpiFlashData[i] = s_aSignature[i];
			bSignatureOK = false;
		}
	}

	if (!bSignatureOK) {
		DEBUG_PUTS("No signature");
		memset(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE], 0, FlashStore::SIZE - FlashStore::SIGNATURE_SIZE);
		s_State = State::CHANGED;
	}

	s_nSpiFlashStoreSize = FlashStore::OFFSET_STORES;

	for (uint32_t j = 0; j < static_cast<uint32_t>(Store::LAST); j++) {
		s_nSpiFlashStoreSize += s_aStorSize[j];
	}

	DEBUG_PRINTF("FlashStore::OFFSET_STORES=%d, m_nSpiFlashStoreSize=%d", static_cast<int>(FlashStore::OFFSET_STORES), s_nSpiFlashStoreSize);

	assert(s_nSpiFlashStoreSize <= FlashStore::SIZE);

	for (uint32_t nStore = 0; nStore < static_cast<uint32_t>(Store::LAST); nStore++) {
		auto *pSet = reinterpret_cast<uint32_t *>((&s_SpiFlashData[GetStoreOffset(static_cast<Store>(nStore))]));
		if (*pSet == UINT32_MAX) {
			*pSet = 0;
		}
	}

	auto *p = reinterpret_cast<struct Env *>(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE]);
	if (p->nUtcOffset == -1) {
		p->nUtcOffset = 0;
	}

	DEBUG_PUTS("");
	debug_dump(s_SpiFlashData, FlashStore::SIZE);

	DEBUG_EXIT
}

uint32_t ConfigStore::GetStoreOffset(Store store) {
	assert(store < Store::LAST);

	uint32_t nOffset = FlashStore::OFFSET_STORES;

	for (uint32_t i = 0; i < static_cast<uint32_t>(store); i++) {
		nOffset += s_aStorSize[i];
	}

	DEBUG_PRINTF("nOffset=%d", nOffset);

	return nOffset;
}

void ConfigStore::ResetSetList(Store store) {
	assert(store < Store::LAST);

	auto *pbSetList = &s_SpiFlashData[GetStoreOffset(store)];

	*pbSetList++ = 0x00;
	*pbSetList++ = 0x00;
	*pbSetList++ = 0x00;
	*pbSetList = 0x00;

	s_State = State::CHANGED;
}

void ConfigStore::Update(Store store, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList, uint32_t nOffsetSetList) {
	DEBUG_ENTRY
	DEBUG_PRINTF("[%s]:%u:%p, nOffset=%d, nDataLength=%d-%u, bSetList=0x%x, nOffsetSetList=%d", s_aStoreName[static_cast<uint32_t>(store)], static_cast<uint32_t>(store), pData, nOffset, nDataLength, static_cast<uint32_t>(s_State), nSetList, nOffsetSetList);

	assert(store < Store::LAST);
	assert(pData != nullptr);
	assert((nOffset + nDataLength) <= s_aStorSize[static_cast<uint32_t>(store)]);

	auto bIsChanged = false;
	const auto nBase = nOffset + GetStoreOffset(store);

	const auto *pSrc = static_cast<const uint8_t *>(pData);
	auto *pDst = &s_SpiFlashData[nBase];

	DEBUG_PRINTF("pSrc=%p [pData], pDst=%p", reinterpret_cast<const void *>(pSrc), reinterpret_cast<void *>(pDst));

#if defined(__linux__) || defined (__APPLE__)
//	debug_dump(pSrc, nDataLength);
//	debug_dump(pDst, nDataLength);
#endif

	for (uint32_t i = 0; i < nDataLength; i++) {
		if (*pSrc != *pDst) {
			bIsChanged = true;
			*pDst = *pSrc;
		}
		pDst++;
		pSrc++;
	}

	if (bIsChanged){
		auto *pSet = reinterpret_cast<uint32_t *>((&s_SpiFlashData[GetStoreOffset(store)] + nOffsetSetList));
		*pSet |= nSetList;
	}

	if (bIsChanged) {
		s_State = State::CHANGED;
	}

	debug_dump(&s_SpiFlashData[GetStoreOffset(store)] + nOffsetSetList, 8);
	DEBUG_EXIT
}

void ConfigStore::Copy(const Store store, void *pData, uint32_t nDataLength, uint32_t nOffset, const bool doUpdate) {
	DEBUG_ENTRY
	DEBUG_PRINTF("[%s]:%u pData=%p, nDataLength=%u, nOffset=%u, doUpdate=%u", s_aStoreName[static_cast<uint32_t>(store)], static_cast<uint32_t>(store), pData, nDataLength, nOffset, doUpdate);

	assert(store < Store::LAST);
	assert(pData != nullptr);
	assert((nDataLength + nOffset) <= s_aStorSize[static_cast<uint32_t>(store)]);

	const auto *pSrc = const_cast<const uint8_t *>(&s_SpiFlashData[GetStoreOffset(store)]) + nOffset;
	auto *pDst = static_cast<uint8_t *>(pData);

	auto isEmpty = true;

	for (uint32_t nIndex = 0; nIndex < nDataLength; nIndex++) {
		if (pSrc[nIndex] != 0) {
			isEmpty = false;
			break;
		}
	}

	if (!isEmpty) {
		memcpy(pDst, pSrc, nDataLength);
		DEBUG_EXIT
		return;
	}

	if (doUpdate) {
		Update(store, pData, nDataLength);
	}

	DEBUG_EXIT
}

void ConfigStore::Delay() {
	if (s_State != State::IDLE) {
		s_State = State::CHANGED;
	}
}

bool ConfigStore::Flash() {
	if (__builtin_expect((s_State == State::IDLE), 1)) {
		return false;
	}

	switch (s_State) {
	case State::CHANGED:
		s_nWaitMillis = Hardware::Get()->Millis();
		s_State = State::CHANGED_WAITING;
		return true;
	case State::CHANGED_WAITING:
		if ((Hardware::Get()->Millis() - s_nWaitMillis) < 100) {
			return true;
		}
		s_State = State::ERASING;
		return true;
		break;
	case State::ERASING: {
		storedevice::result result;
		if (StoreDevice::Erase(s_nStartAddress, FlashStore::SIZE, result)) {
			s_nWaitMillis = Hardware::Get()->Millis();
			s_State = State::ERASED_WAITING;
		}
		assert(result == storedevice::result::OK);
		DEBUG_PRINTF("s_State=%u", static_cast<uint32_t>(s_State));
		return true;
	}
		break;
	case State::ERASED_WAITING:
		if ((Hardware::Get()->Millis() - s_nWaitMillis) < 100) {
			return true;
		}
		s_State = State::ERASED;
		return true;
		break;
	case State::ERASED:
		s_State = State::WRITING;
		return true;
		break;
	case State::WRITING: {
		storedevice::result result;
		if (StoreDevice::Write(s_nStartAddress, s_nSpiFlashStoreSize, reinterpret_cast<uint8_t*>(&s_SpiFlashData), result)) {
			s_State = State::IDLE;
			return false;
		}
		assert(result == storedevice::result::OK);
		return true;
	}
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return false;
}

void ConfigStore::Dump() {
#ifndef NDEBUG
	if (!s_bHaveFlashChip) {
		return;
	}

	const auto IsWatchDog = Hardware::Get()->IsWatchdog();

	if (IsWatchDog) {
		Hardware::Get()->WatchdogStop();
	}

	debug_dump(s_SpiFlashData, FlashStore::OFFSET_STORES);
	puts("");

	for (uint32_t j = 0; j < static_cast<uint32_t>(Store::LAST); j++) {
		printf("Store [%s]:%d\n", s_aStoreName[j], j);

		auto *p = &s_SpiFlashData[GetStoreOffset(static_cast<Store>(j))];
		debug_dump(p, static_cast<uint16_t>(s_aStorSize[j]));

		puts("");
	}

	if (IsWatchDog) {
		Hardware::Get()->WatchdogInit();
	}

	printf("m_tState=%d\n", static_cast<uint32_t>(s_State));
#endif
}
