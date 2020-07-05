/**
 * @file modeparams.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "l6470.h"
#include "lightset.h"
#include "dmxslotinfo.h"

static constexpr uint16_t MODE_PARAMS_MAX_DMX_FOOTPRINT	=	4;

struct TModeParams {
    uint32_t nSetList;
    uint8_t nDmxMode;
    uint16_t nDmxStartAddress;
    uint32_t nMaxSteps;
    TL6470Action tSwitchAction;
    TL6470Direction tSwitchDir;
    float fSwitchStepsPerSec;
    bool bSwitch;
    //
    alignas(uint32_t) struct TLightSetSlotInfo tLightSetSlotInfo[MODE_PARAMS_MAX_DMX_FOOTPRINT];
} __attribute__((packed));

struct ModeParamsMask {
	static constexpr auto SLOT_INFO_SHIFT = 28;
	static constexpr auto SLOT_INFO_MASK = (0xF << SLOT_INFO_SHIFT);

	static constexpr auto DMX_MODE = (1U << 0);
	static constexpr auto DMX_START_ADDRESS = (1U << 1);
	//
	static constexpr auto MAX_STEPS = (1U << 2);
	//
	static constexpr auto SWITCH_ACT = (1U << 3);
	static constexpr auto SWITCH_DIR = (1U << 4);
	static constexpr auto SWITCH_SPS = (1U << 5);
	static constexpr auto SWITCH = (1U << 6);
	//
	static constexpr auto SLOT_INFO_0 = (1U << (SLOT_INFO_SHIFT + 0));
	static constexpr auto SLOT_INFO_1 = (1U << (SLOT_INFO_SHIFT + 1));
	static constexpr auto SLOT_INFO_2 = (1U << (SLOT_INFO_SHIFT + 2));
	static constexpr auto SLOT_INFO_3 = (1U << (SLOT_INFO_SHIFT + 3));
};

class ModeParamsStore {
public:
	virtual ~ModeParamsStore() {}

	virtual void Update(uint8_t nMotorIndex, const struct TModeParams *ptModeParams)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TModeParams *ptModeParams)=0;
};

class ModeParams {
public:
	ModeParams(ModeParamsStore *pModeParamsStore=nullptr);
	~ModeParams();

	bool Load(uint8_t nMotorIndex);
	void Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint8_t nMotorIndex, const struct TModeParams *ptModeParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump();

	uint16_t GetDmxMode() {
		return m_tModeParams.nDmxMode;
	}

	uint16_t GetDmxStartAddress() {
		return m_tModeParams.nDmxStartAddress;
	}

	uint32_t GetMaxSteps() {
		return m_tModeParams.nMaxSteps;
	}

	TL6470Action GetSwitchAction() {
		return m_tModeParams.tSwitchAction;
	}

	TL6470Direction GetSwitchDir() {
		return m_tModeParams.tSwitchDir;
	}

	float GetSwitchStepsPerSec() {
		return m_tModeParams.fSwitchStepsPerSec;
	}

	bool HasSwitch() {
		return m_tModeParams.bSwitch;
	}

	void GetSlotInfo(uint32_t nOffset, struct TLightSetSlotInfo &tLightSetSlotInfo);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) {
    	return (m_tModeParams.nSetList & nMask) == nMask;
    }

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    ModeParamsStore *m_pModeParamsStore;
    struct TModeParams m_tModeParams;
    char m_aFileName[16];
    DmxSlotInfo *m_pDmxSlotInfo;
};

#endif /* MODEPARAMS_H_ */
