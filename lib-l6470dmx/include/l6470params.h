/**
 * @file l6470params.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef L6470PARAMS_H_
#define L6470PARAMS_H_

#include <cstdint>
#include <cstddef>

#include "l6470.h"
#include "l6470dmxstore.h"

#include "configstore.h"

#include "debug.h"

namespace l6470params {
struct Mask {
	static constexpr uint32_t MIN_SPEED   = (1U << 0);
	static constexpr uint32_t MAX_SPEED   = (1U << 1);
	static constexpr uint32_t ACC         = (1U << 2);
	static constexpr uint32_t DEC         = (1U << 3);
	static constexpr uint32_t KVAL_HOLD   = (1U << 4);
	static constexpr uint32_t KVAL_RUN    = (1U << 5);
	static constexpr uint32_t KVAL_ACC    = (1U << 6);
	static constexpr uint32_t KVAL_DEC    = (1U << 7);
	static constexpr uint32_t MICRO_STEPS = (1U << 8);
};
}  // namespace l6470params

class L6470ParamsStore {
public:
	static L6470ParamsStore& Get() {
		static L6470ParamsStore instance;
		return instance;
	}

	static void Update(uint32_t nMotorIndex, const struct l6470params::Params *pParams) {
		Get().IUpdate(nMotorIndex, pParams);
	}

	static void Copy(uint32_t nMotorIndex, struct l6470params::Params *pParams) {
		Get().ICopy(nMotorIndex, pParams);
	}

private:
	L6470ParamsStore() {
		assert(motorstore::STRUCT_SIZE <= motorstore::MAX_SIZE);

		for (uint32_t nMotorIndex = 0; nMotorIndex < motorstore::MAX_MOTORS; nMotorIndex++) {
			// struct l6470params::Params
			struct l6470params::Params tL6470Params;
			memset( &tL6470Params, 0xFF, sizeof(struct l6470params::Params));
			tL6470Params.nSetList = 0;

			ICopy(nMotorIndex, &tL6470Params);

			if (tL6470Params.nSetList == static_cast<uint32_t>(~0)) {
				DEBUG_PRINTF("%d: Clear nSetList -> tL6470Params", nMotorIndex);
				tL6470Params.nSetList = 0;
				IUpdate(nMotorIndex, &tL6470Params);
			}
		}
	}

	void IUpdate(uint32_t nMotorIndex, const struct l6470params::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Update(configstore::Store::MOTORS, motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, L6470Params), pParams, sizeof(struct l6470params::Params));
		DEBUG_EXIT
	}

	void ICopy(uint32_t nMotorIndex, struct l6470params::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Copy(configstore::Store::MOTORS, pParams, sizeof(struct l6470params::Params), motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, L6470Params));
		DEBUG_EXIT
	}
 };

class L6470Params {
public:
	L6470Params();

	void Load(uint32_t nMotorIndex);
	void Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint32_t nMotorIndex, const struct l6470params::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(L6470 *);

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    l6470params::Params m_Params;
    char m_aFileName[16];
};

#endif /* L6470PARAMS_H_ */
