/**
 * @file configstore.h
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

#ifndef CONFIGSTORE_H_
#define CONFIGSTORE_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "configstoredevice.h"

#include "utc.h"

#include "debug.h"

namespace configstore {
enum class Store {
	NETWORK,
	DMXSEND,
	WS28XXDMX,
	LTC,
	MIDI,
	LTCETC,
	OSC,
	TLC5711DMX,
	WIDGET,
	RDMDEVICE,
	RCONFIG,
	TCNET,
	OSC_CLIENT,
	DISPLAYUDF,
	LTCDISPLAY,
	MONITOR,
	SPARKFUN,
	SLUSH,
	MOTORS,
	SHOW,
	SERIAL,
	RDMSENSORS,
	RDMSUBDEVICES,
	GPS,
	RGBPANEL,
	NODE,
	PCA9685,
	LAST
};

enum class State {
	IDLE, CHANGED, CHANGED_WAITING, ERASING, ERASED, ERASED_WAITING, WRITING
};
}  // namespace configstore

class ConfigStore: StoreDevice {
public:
	ConfigStore();
	~ConfigStore() {
		while (Flash())
			;
	}

	void Update(configstore::Store store, uint32_t nOffset, const void *pData, uint32_t nDataLength, uint32_t nSetList = 0, uint32_t nOffsetSetList = 0);
	void Update(configstore::Store store, const void *pData, uint32_t nDataLength) {
		Update(store, 0, pData, nDataLength);
	}

	void Copy(const configstore::Store store, void *pData, uint32_t nDataLength, uint32_t nOffset = 0, const bool doUpdate = true);

	void ResetSetList(configstore::Store store);
	void ResetSetListAll() {
		for (uint32_t i = 0; i < static_cast<uint32_t>(configstore::Store::LAST); i++) {
			ResetSetList(static_cast<configstore::Store>(i));
		}
	}

	bool Flash();

	void Dump();

	void Delay();

	/*
	 * Environment
	 */

	bool SetEnvUtcOffset(const int8_t nHours, const uint8_t nMinutes) {
		int32_t nUtcOffset;

		DEBUG_PRINTF("nHours=%d, nMinutes =%u", nHours, nMinutes);

		if (hal::utc_validate(nHours, nMinutes, nUtcOffset)) {
			auto *p = reinterpret_cast<struct Env *>(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE]);

			if (p->nUtcOffset != nUtcOffset) {
				p->nUtcOffset = nUtcOffset;
				s_State = configstore::State::CHANGED;
			}

			DEBUG_EXIT
			return true;
		}

		DEBUG_EXIT
		return false;
	}

	void GetEnvUtcOffset(int8_t& nHours, uint8_t& nMinutes) {
		const auto *p = reinterpret_cast<struct Env *>(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE]);

		DEBUG_PRINTF("p->nUtcOffset=%d", p->nUtcOffset);

		assert((p->nUtcOffset / 3600) <= INT8_MAX);
		assert((p->nUtcOffset / 3600) >= INT8_MIN);

		nHours = static_cast<int8_t>(p->nUtcOffset / 3600);

		if (nHours > 0) {
			nMinutes = static_cast<uint8_t>(static_cast<uint32_t>(p->nUtcOffset - (nHours * 3600)) / 60U);
		} else {
			nMinutes = static_cast<uint8_t>(static_cast<uint32_t>((nHours * 3600) - p->nUtcOffset) / 60U);
		}
	}

	int32_t GetEnvUtcOffset() const {
		const auto *p = reinterpret_cast<struct Env *>(&s_SpiFlashData[FlashStore::SIGNATURE_SIZE]);
		return p->nUtcOffset;
	}

	static ConfigStore *Get() {
		return s_pThis;
	}

private:
	uint32_t GetStoreOffset(configstore::Store tStore);

private:
	struct Env {
		int32_t nUtcOffset;
		uint8_t filler[12];
	};

	struct FlashStore {
		static constexpr uint32_t SIGNATURE_SIZE = 16;
		static constexpr uint32_t ENV_SIZE = 16;
		static constexpr uint32_t OFFSET_STORES = SIGNATURE_SIZE + ENV_SIZE;
		static constexpr uint32_t SIZE = 4096;
	};

	static_assert(sizeof(struct Env) == FlashStore::ENV_SIZE, "");

	static bool s_bHaveFlashChip;

	static configstore::State s_State;
	static uint32_t s_nStartAddress;

	static uint32_t s_nSpiFlashStoreSize;
	static uint8_t s_SpiFlashData[FlashStore::SIZE];

	static uint32_t s_nWaitMillis;

	static ConfigStore *s_pThis;
};

#endif /* CONFIGSTORE_H_ */
