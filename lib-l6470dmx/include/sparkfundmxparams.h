/**
 * @file sparkfundmxparams.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SPARKFUNDMXPARAMS_H_
#define SPARKFUNDMXPARAMS_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "sparkfundmx.h"
#include "configstore.h"

#include "debug.h"

namespace sparkfundmxparams {
struct Params {
	uint32_t nSetList;
	uint8_t nPosition;
	uint8_t nSpiCs;
	uint8_t nResetPin;
	uint8_t nBusyPin;
} __attribute__((packed));

struct Mask {
	static constexpr uint32_t POSITION = (1U << 0);
	static constexpr uint32_t SPI_CS = (1U << 1);
	static constexpr uint32_t RESET_PIN = (1U << 2);
	static constexpr uint32_t BUSY_PIN = (1U << 3);
};
}  // namespace sparkfundmxparams

namespace sparkfunparamsstore {
static constexpr uint32_t MAX_SIZE      = 96;
static constexpr uint32_t MAX_MOTORS    = 8;
static constexpr uint32_t STRUCT_OFFSET = 16;
static constexpr uint32_t STRUCT_SIZE = (MAX_MOTORS * sizeof(struct sparkfundmxparams::Params));
static constexpr uint32_t OFFSET(const uint32_t x) {
	return STRUCT_OFFSET + (x * sizeof(struct sparkfundmxparams::Params));
}
}  // namespace sparkfunparamsstore

class SparkFunDmxParamsStore {
public:
	static SparkFunDmxParamsStore& Get() {
		static SparkFunDmxParamsStore instance;
		return instance;
	}

	static void Update(const struct sparkfundmxparams::Params *pSparkFunDmxParams) {
		Get().IUpdate(pSparkFunDmxParams);
	}

	static void Copy(struct sparkfundmxparams::Params *pSparkFunDmxParams) {
		Get().ICopy(pSparkFunDmxParams);
	}

	static void Update(uint32_t nMotorIndex, const struct sparkfundmxparams::Params *pParams) {
		Get().IUpdate(nMotorIndex, pParams);
	}

	static void Copy(uint32_t nMotorIndex, struct sparkfundmxparams::Params *pParams) {
		Get().ICopy(nMotorIndex, pParams);
	}

private:
	SparkFunDmxParamsStore() {
		DEBUG_ENTRY
		DEBUG_PRINTF("sizeof(TSparkFunDmxParams) = %d", static_cast<int>(sizeof(struct sparkfundmxparams::Params)));
		DEBUG_PRINTF("sparkfunparamsstore::STRUCT_OFFSET + sparkfunparamsstore::STRUCT_SIZE = %d", static_cast<int>(sparkfunparamsstore::STRUCT_OFFSET + sparkfunparamsstore::STRUCT_SIZE));

		assert(sizeof(struct sparkfundmxparams::Params) <= sparkfunparamsstore::STRUCT_OFFSET);
		assert((sparkfunparamsstore::STRUCT_OFFSET + sparkfunparamsstore::STRUCT_SIZE) <= sparkfunparamsstore::MAX_SIZE);

		for (uint32_t nMotorIndex = 0; nMotorIndex < sparkfunparamsstore::MAX_MOTORS; nMotorIndex++) {
			struct sparkfundmxparams::Params sparkFunDmxParams;
			memset( &sparkFunDmxParams, 0xFF, sizeof(struct sparkfundmxparams::Params));
			sparkFunDmxParams.nSetList = 0;

			ICopy(nMotorIndex, &sparkFunDmxParams);

			if (sparkFunDmxParams.nSetList == static_cast<uint32_t>(~0)) {
				DEBUG_PRINTF("%d: Clear nSetList", nMotorIndex);
				sparkFunDmxParams.nSetList = 0;
				IUpdate(nMotorIndex, &sparkFunDmxParams);
			}
		}

		DEBUG_EXIT
	}

	void IUpdate(const struct sparkfundmxparams::Params *pSparkFunDmxParams) {
		DEBUG_ENTRY
		ConfigStore::Get()->Update(configstore::Store::SPARKFUN, pSparkFunDmxParams, sizeof(struct sparkfundmxparams::Params));
		DEBUG_EXIT
	}

	void ICopy(struct sparkfundmxparams::Params *pSparkFunDmxParams) {
		DEBUG_ENTRY
		ConfigStore::Get()->Copy(configstore::Store::SPARKFUN, pSparkFunDmxParams, sizeof(struct sparkfundmxparams::Params));
		DEBUG_EXIT
	}

	void IUpdate(uint32_t nMotorIndex, const struct sparkfundmxparams::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < sparkfunparamsstore::MAX_MOTORS);
		ConfigStore::Get()->Update(configstore::Store::SPARKFUN, sparkfunparamsstore::OFFSET(nMotorIndex), pParams, sizeof(struct sparkfundmxparams::Params));
		DEBUG_EXIT
	}

	void ICopy(uint32_t nMotorIndex, struct sparkfundmxparams::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < sparkfunparamsstore::MAX_MOTORS);
		ConfigStore::Get()->Copy(configstore::Store::SPARKFUN, pParams, sizeof(struct sparkfundmxparams::Params), sparkfunparamsstore::OFFSET(nMotorIndex));
		DEBUG_EXIT
	}
};

class SparkFunDmxParams {
public:
	SparkFunDmxParams();

	void Load();
	void Load(uint32_t nMotorIndex);
	void Load(const char *pBuffer, uint32_t nLength);
	void Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(const struct sparkfundmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize, uint32_t nMotorIndex = 0xFF);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize, uint32_t nMotorIndex = 0xFF);

	void SetGlobal(SparkFunDmx *pSparkFunDmx);
	void SetLocal(SparkFunDmx *pSparkFunDmx);

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump(uint32_t nMotorIndex = 0xFF);
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    sparkfundmxparams::Params m_Params;
    char m_aFileName[16];
};

#endif /* SPARKFUNDMXPARAMS_H_ */
