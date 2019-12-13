/**
 * @file modeparams.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define MODE_PARAMS_MAX_DMX_FOOTPRINT		4
#define MODE_PARAMS_MASK_SLOT_INFO_SHIFT	28
#define MODE_PARAMS_MASK_SLOT_INFO_MASK		(0xF << MODE_PARAMS_MASK_SLOT_INFO_SHIFT)

struct TModeParams {
    uint32_t nSetList;
    //
    uint8_t nDmxMode;
    uint16_t nDmxStartAddress;
    //
    uint32_t nMaxSteps;
    //
    TL6470Action tSwitchAction;
    TL6470Direction tSwitchDir;
    float fSwitchStepsPerSec;
    bool bSwitch;
    //
    struct TLightSetSlotInfo tLightSetSlotInfo[MODE_PARAMS_MAX_DMX_FOOTPRINT];
} __attribute__((packed));

enum TModeParamsMask {
	MODE_PARAMS_MASK_DMX_MODE = (1 << 0),
	MODE_PARAMS_MASK_DMX_START_ADDRESS = (1 << 1),
	//
	MODE_PARAMS_MASK_MAX_STEPS = (1 << 2),
	//
	MODE_PARAMS_MASK_SWITCH_ACT = (1 << 3),
	MODE_PARAMS_MASK_SWITCH_DIR = (1 << 4),
	MODE_PARAMS_MASK_SWITCH_SPS = (1 << 5),
	MODE_PARAMS_MASK_SWITCH = (1 << 6),
	//
	MODE_PARAMS_MASK_SLOT_INFO_0 = (1 << (MODE_PARAMS_MASK_SLOT_INFO_SHIFT + 0)),
	MODE_PARAMS_MASK_SLOT_INFO_1 = (1 << (MODE_PARAMS_MASK_SLOT_INFO_SHIFT + 1)),
	MODE_PARAMS_MASK_SLOT_INFO_2 = (1 << (MODE_PARAMS_MASK_SLOT_INFO_SHIFT + 2)),
	MODE_PARAMS_MASK_SLOT_INFO_3 = (1 << (MODE_PARAMS_MASK_SLOT_INFO_SHIFT + 3))
};

class ModeParamsStore {
public:
	virtual ~ModeParamsStore(void);

	virtual void Update(uint8_t nMotorIndex, const struct TModeParams *ptModeParams)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TModeParams *ptModeParams)=0;
};

class ModeParams {
public:
	ModeParams(ModeParamsStore *pModeParamsStore=0);
	~ModeParams(void);

	bool Load(uint8_t nMotorIndex);
	void Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(uint8_t nMotorIndex, const struct TModeParams *ptModeParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint8_t nMotorIndex, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	uint16_t GetDmxMode(void) {
		return m_tModeParams.nDmxMode;
	}

	uint16_t GetDmxStartAddress(void) {
		return m_tModeParams.nDmxStartAddress;
	}

	uint32_t GetMaxSteps(void) {
		return m_tModeParams.nMaxSteps;
	}

	TL6470Action GetSwitchAction(void) {
		return m_tModeParams.tSwitchAction;
	}

	TL6470Direction GetSwitchDir(void) {
		return m_tModeParams.tSwitchDir;
	}

	float GetSwitchStepsPerSec(void) {
		return m_tModeParams.fSwitchStepsPerSec;
	}

	bool HasSwitch(void) {
		return m_tModeParams.bSwitch;
	}

	void GetSlotInfo(uint32_t nOffset, struct TLightSetSlotInfo &tLightSetSlotInfo);

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint32_t nMask) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    ModeParamsStore *m_pModeParamsStore;
    struct TModeParams m_tModeParams;
    char m_aFileName[16];
    DmxSlotInfo *m_pDmxSlotInfo;
};

#endif /* MODEPARAMS_H_ */
