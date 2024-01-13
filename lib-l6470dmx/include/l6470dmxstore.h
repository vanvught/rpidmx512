/**
 * @file l6470dmxstore.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef L6470DMXSTORE_H_
#define L6470DMXSTORE_H_

#include <cstdint>

#include "l6470.h"
#include "lightset.h"

namespace modeparams {
static constexpr uint16_t MODE_PARAMS_MAX_DMX_FOOTPRINT = 4;

struct Params {
	uint32_t nSetList;
	uint8_t nDmxMode;
	uint16_t nDmxStartAddress;
	uint32_t nMaxSteps;
	TL6470Action tSwitchAction;
	TL6470Direction tSwitchDir;
	float fSwitchStepsPerSec;
	bool bSwitch;
	//
	lightset::SlotInfo tLightSetSlotInfo[MODE_PARAMS_MAX_DMX_FOOTPRINT];
} __attribute__((packed));
}  // namespace modeparams

namespace l6470params {
struct Params {
	uint32_t nSetList;
	float fMinSpeed;
	float fMaxSpeed;
	float fAcc;
	float fDec;
	uint8_t nKvalHold;
	uint8_t nKvalRun;
	uint8_t nKvalAcc;
	uint8_t nKvalDec;
	uint8_t nMicroSteps;
} __attribute__((packed));
}  // namespace l6470params

namespace motorparams {
struct Params {
	uint32_t nSetList;
	float fStepAngel;
	float fVoltage;
	float fCurrent;
	float fResistance;
	float fInductance;
} __attribute__((packed));
}  // namespace motorparams

namespace motorstore {
struct MotorStore {
	struct modeparams::Params ModeParams;
	uint8_t Filler1[64 - sizeof(struct modeparams::Params)];
	struct motorparams::Params MotorParams;
	uint8_t Filler2[32 - sizeof(struct motorparams::Params)];
	struct l6470params::Params L6470Params;
	uint8_t Filler3[32 - sizeof(struct l6470params::Params)];
} __attribute__((packed));

static constexpr uint32_t MAX_SIZE = 1024;
static constexpr uint32_t MAX_MOTORS = 8;
static constexpr uint32_t STRUCT_SIZE = (MAX_MOTORS * sizeof(struct motorstore::MotorStore));
static constexpr uint32_t OFFSET(const uint32_t x) {
	return x * sizeof(struct motorstore::MotorStore);
}
}  // namespace motorstore

#endif /* L6470DMXSTORE_H_ */
