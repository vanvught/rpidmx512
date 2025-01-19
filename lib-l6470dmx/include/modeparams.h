/**
 * @file modeparams.h
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

#ifndef MODEPARAMS_H_
#define MODEPARAMS_H_

#include <cstdint>
#include <cstddef>

#include "l6470.h"
#include "l6470dmxstore.h"

#include "configstore.h"

#include "lightset.h"
#include "dmxslotinfo.h"

#include "debug.h"

namespace modeparams {
struct Mask {
	static constexpr uint32_t SLOT_INFO_SHIFT = 28;
	static constexpr uint32_t SLOT_INFO_MASK  = (0xFU << SLOT_INFO_SHIFT);

	static constexpr uint32_t DMX_MODE          = (1U << 0);
	static constexpr uint32_t DMX_START_ADDRESS = (1U << 1);
	//
	static constexpr uint32_t MAX_STEPS = (1U << 2);
	//
	static constexpr uint32_t SWITCH_ACT = (1U << 3);
	static constexpr uint32_t SWITCH_DIR = (1U << 4);
	static constexpr uint32_t SWITCH_SPS = (1U << 5);
	static constexpr uint32_t SWITCH     = (1U << 6);
	//
	static constexpr uint32_t SLOT_INFO_0 = (1U << (SLOT_INFO_SHIFT + 0));
	static constexpr uint32_t SLOT_INFO_1 = (1U << (SLOT_INFO_SHIFT + 1));
	static constexpr uint32_t SLOT_INFO_2 = (1U << (SLOT_INFO_SHIFT + 2));
	static constexpr uint32_t SLOT_INFO_3 = (1U << (SLOT_INFO_SHIFT + 3));
};
}  // namespace modeparams

class ModeParamsStore {
public:
	static ModeParamsStore& Get() {
		static ModeParamsStore instance;
		return instance;
	}

	static void Update(uint32_t nMotorIndex, const struct modeparams::Params *pParams)  {
		Get().IUpdate(nMotorIndex, pParams);
	}

	static void Copy(uint32_t nMotorIndex, struct modeparams::Params *pParams)  {
		Get().ICopy(nMotorIndex, pParams);
	}

private:
	ModeParamsStore() {
		assert(motorstore::STRUCT_SIZE <= motorstore::MAX_SIZE);

		for (uint32_t nMotorIndex = 0; nMotorIndex < motorstore::MAX_MOTORS; nMotorIndex++) {
			struct modeparams::Params tModeParams;
			memset( &tModeParams, 0xFF, sizeof(struct modeparams::Params));
			tModeParams.nSetList = 0;

			ICopy(nMotorIndex, &tModeParams);

			if (tModeParams.nSetList == static_cast<uint32_t>(~0)) {
				DEBUG_PRINTF("%d: Clear nSetList -> tModeParams", nMotorIndex);
				tModeParams.nSetList = 0;
				IUpdate(nMotorIndex, &tModeParams);
			}
		}
	}

	void IUpdate(uint32_t nMotorIndex, const struct modeparams::Params *pParams)  {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Update(configstore::Store::MOTORS, motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, ModeParams), pParams, sizeof(struct modeparams::Params));
		DEBUG_EXIT
	}

	void ICopy(uint32_t nMotorIndex, struct modeparams::Params *pParams)  {
		DEBUG_ENTRY
		assert(nMotorIndex < motorstore::MAX_MOTORS);
		ConfigStore::Get()->Copy(configstore::Store::MOTORS, pParams, sizeof(struct modeparams::Params), motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, ModeParams));
		DEBUG_EXIT
	}
};

class ModeParams {
public:
	ModeParams();
	~ModeParams();

	bool Load(uint32_t nMotorIndex);
	void Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint32_t nMotorIndex, const struct modeparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize);

	uint16_t GetDmxMode() const {
		return m_Params.nDmxMode;
	}

	uint16_t GetDmxStartAddress() const {
		return m_Params.nDmxStartAddress;
	}

	uint32_t GetMaxSteps() const {
		return m_Params.nMaxSteps;
	}

	TL6470Action GetSwitchAction() const {
		return m_Params.tSwitchAction;
	}

	TL6470Direction GetSwitchDir() const {
		return m_Params.tSwitchDir;
	}

	float GetSwitchStepsPerSec() const {
		return m_Params.fSwitchStepsPerSec;
	}

	bool HasSwitch() const {
		return m_Params.bSwitch;
	}

	void GetSlotInfo(uint32_t nOffset, lightset::SlotInfo &tLightSetSlotInfo);

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    struct modeparams::Params m_Params;
    char m_aFileName[16];
    DmxSlotInfo *m_pDmxSlotInfo;
};

#endif /* MODEPARAMS_H_ */
