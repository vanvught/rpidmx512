/**
 * @file motorparams.h
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

#ifndef MOTORPARAMS_H_
#define MOTORPARAMS_H_

#include <cstdint>
#include <cstddef>

#include "l6470.h"
#include "l6470dmxstore.h"

#include "configstore.h"

#include "debug.h"

namespace motorparams {
struct Mask {
	static constexpr uint32_t STEP_ANGEL = (1U << 0);
	static constexpr uint32_t VOLTAGE    = (1U << 1);
	static constexpr uint32_t CURRENT    = (1U << 2);
	static constexpr uint32_t RESISTANCE = (1U << 3);
	static constexpr uint32_t INDUCTANCE = (1U << 4);
};
}  // namespace motorparams

class MotorParamsStore {
public:
	static MotorParamsStore& Get() {
		static MotorParamsStore instance;
		return instance;
	}

	static void Update(uint32_t nMotorIndex, const struct motorparams::Params *pParams) {
		Get().IUpdate(nMotorIndex, pParams);
	}

	static void Copy(uint32_t nMotorIndex, struct motorparams::Params *pParams) {
		Get().ICopy(nMotorIndex, pParams);
	}

private:
	MotorParamsStore() {
		assert(motorstore::STRUCT_SIZE <= motorstore::MAX_SIZE);

		for (uint32_t nMotorIndex = 0; nMotorIndex < motorstore::MAX_MOTORS; nMotorIndex++) {
			struct motorparams::Params tMotorParams;
			memset( &tMotorParams, 0xFF, sizeof(struct motorparams::Params));
			tMotorParams.nSetList = 0;

			ICopy(nMotorIndex, &tMotorParams);

			if (tMotorParams.nSetList == static_cast<uint32_t>(~0)) {
				DEBUG_PRINTF("%d: Clear nSetList -> tMotorParams", nMotorIndex);
				tMotorParams.nSetList = 0;
				IUpdate(nMotorIndex, &tMotorParams);
			}
		}
	}

	void IUpdate(uint32_t nMotorIndex, const struct motorparams::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Update(configstore::Store::MOTORS, motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, MotorParams), pParams, sizeof(struct motorparams::Params));
		DEBUG_EXIT
	}

	void ICopy(uint32_t nMotorIndex, struct motorparams::Params *pParams) {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Copy(configstore::Store::MOTORS, pParams, sizeof(struct motorparams::Params), motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, MotorParams));
		DEBUG_EXIT
	}
};

class MotorParams {
public:
	MotorParams();

	bool Load(uint32_t nMotorIndex);
	void Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint32_t nMotorIndex, const struct motorparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(L6470 *pL6470);

	float GetStepAngel() const {
		return m_Params.fStepAngel;
	}

	float GetVoltage() const {
		return m_Params.fVoltage;
	}

	float GetCurrent() const {
		return m_Params.fCurrent;
	}

	float GetResistance() const {
		return m_Params.fResistance;
	}

	float GetInductance() const {
		return m_Params.fInductance;
	}

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

	float calcIntersectSpeed();
	uint32_t calcIntersectSpeedReg(float) const;

    static void StaticCallbackFunction(void *p, const char *s);

private:
    motorparams::Params m_Params;
    char m_aFileName[16];
};

#endif /* MOTORPARAMS_H_ */
