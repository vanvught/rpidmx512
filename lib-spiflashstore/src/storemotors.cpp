/**
 * @file storemotors.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "storemotors.h"

#include "modeparams.h"
#include "motorparams.h"
#include "l6470params.h"

#include "spiflashstore.h"

#include "debug.h"

struct TMotorStore {
	struct TModeParams ModeParams;
	uint8_t Filler1[64 - sizeof(struct TModeParams)];
	struct TMotorParams MotorParams;
	uint8_t Filler2[32 - sizeof(struct TMotorParams)];
	struct TL6470Params L6470Params;
	uint8_t Filler3[32 - sizeof(struct TL6470Params)];
} __attribute__((packed));

#define STORE_MOTORS_MAX_SIZE		1024
#define STORE_MOTORS_MAX_MOTORS		8
#define STORE_MOTORS_STRUCT_SIZE	(STORE_MOTORS_MAX_MOTORS * sizeof(struct TMotorStore))

#define STORE_MOTOR_OFFSET(x)		((x) * sizeof(struct TMotorStore))

StoreMotors *StoreMotors::s_pThis = nullptr;

StoreMotors::StoreMotors() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_PRINTF("STORE_MOTORS_STRUCT_SIZE=%d", static_cast<int>(STORE_MOTORS_STRUCT_SIZE));

	assert(STORE_MOTORS_STRUCT_SIZE <= STORE_MOTORS_MAX_SIZE);

	for (uint32_t nMotorIndex = 0; nMotorIndex < STORE_MOTORS_MAX_MOTORS; nMotorIndex++) {
		struct TModeParams tModeParams;
		memset( &tModeParams, 0xFF, sizeof(struct TModeParams));
		tModeParams.nSetList = 0;

		Copy(nMotorIndex, &tModeParams);

		if (tModeParams.nSetList == static_cast<uint32_t>(~0)) {
			DEBUG_PRINTF("%d: Clear nSetList -> tModeParams", nMotorIndex);
			tModeParams.nSetList = 0;
			Update(nMotorIndex, &tModeParams);
		}

		struct TMotorParams tMotorParams;
		memset( &tMotorParams, 0xFF, sizeof(struct TMotorParams));
		tMotorParams.nSetList = 0;

		Copy(nMotorIndex, &tMotorParams);

		if (tMotorParams.nSetList == static_cast<uint32_t>(~0)) {
			DEBUG_PRINTF("%d: Clear nSetList -> tMotorParams", nMotorIndex);
			tMotorParams.nSetList = 0;
			Update(nMotorIndex, &tMotorParams);
		}

		struct TL6470Params tL6470Params;
		memset( &tL6470Params, 0xFF, sizeof(struct TL6470Params));
		tL6470Params.nSetList = 0;

		Copy(nMotorIndex, &tL6470Params);

		if (tL6470Params.nSetList == static_cast<uint32_t>(~0)) {
			DEBUG_PRINTF("%d: Clear nSetList -> tL6470Params", nMotorIndex);
			tL6470Params.nSetList = 0;
			Update(nMotorIndex, &tL6470Params);
		}
	}

	DEBUG_EXIT
}

void StoreMotors::Update(uint8_t nMotorIndex, const struct TModeParams *ptModeParams) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Update(STORE_MOTORS, STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, ModeParams), ptModeParams, sizeof(struct TModeParams));

	DEBUG_EXIT
}

void StoreMotors::Copy(uint8_t nMotorIndex, struct TModeParams *ptModeParams) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Copy(STORE_MOTORS, ptModeParams, sizeof(struct TModeParams), STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, ModeParams));

	DEBUG_EXIT
}

void StoreMotors::Update(uint8_t nMotorIndex, const struct TMotorParams *ptMotorParams) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Update(STORE_MOTORS, STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, MotorParams), ptMotorParams, sizeof(struct TMotorParams));

	DEBUG_EXIT
}

void StoreMotors::Copy(uint8_t nMotorIndex, struct TMotorParams *ptMotorParams) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Copy(STORE_MOTORS, ptMotorParams, sizeof(struct TMotorParams), STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, MotorParams));

	DEBUG_EXIT
}

void StoreMotors::Update(uint8_t nMotorIndex, const struct TL6470Params *ptL6470Params) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Update(STORE_MOTORS, STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, L6470Params), ptL6470Params, sizeof(struct TL6470Params));

	DEBUG_EXIT
}

void StoreMotors::Copy(uint8_t nMotorIndex, struct TL6470Params *ptL6470Params) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	SpiFlashStore::Get()->Copy(STORE_MOTORS, ptL6470Params, sizeof(struct TL6470Params), STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, L6470Params));

	DEBUG_EXIT
}

void StoreMotors::SaveDmxStartAddress(uint8_t nMotorIndex, uint16_t nDmxStartAddress) {
	DEBUG_ENTRY

	assert(nMotorIndex < STORE_MOTORS_MAX_MOTORS);

	const uint32_t nOffsetModeParms = STORE_MOTOR_OFFSET(nMotorIndex) + __builtin_offsetof(struct TMotorStore, ModeParams);

	DEBUG_PRINTF("nOffsetModeParms=%d", nOffsetModeParms);

	SpiFlashStore::Get()->Update(STORE_MOTORS, nOffsetModeParms + __builtin_offsetof(struct TModeParams, nDmxStartAddress), &nDmxStartAddress, sizeof(uint16_t), ModeParamsMask::DMX_START_ADDRESS, nOffsetModeParms);

	DEBUG_EXIT
}
