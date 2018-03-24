/**
 * @file l6470dump.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>

#include "l6470.h"

#define L6470_OCD_TH_STEP_MA	375
#define L6470_OCD_TH_MIN_MA		375
#define L6470_OCD_TH_MAX_MA		6000

#define L6470_STALL_TH_STEP_MA	31.25
#define L6470_STALL_TH_MIN_MA	31.25
#define L6470_STALL_TH_MAX_MA	4000

#define L6470_SLP_STEP			0.000015
#define L6470_SLP_MIN			0.0
#define L6470_SLP_MAX			0.004

#define L6470_K_THERM_STEP		0.03125
#define L6470_K_THERM_MIN		1.0
#define L6470_K_THERM_MAX		1.46875

#ifndef NDEBUG

static uint16_t OcdThCalcValueReg(uint8_t reg) {
	if (reg > 0x0F) {
		reg = 0x0F;
	}

	return (reg * L6470_OCD_TH_STEP_MA) + L6470_OCD_TH_MIN_MA;
}

static uint16_t StallThCalcValueReg(uint8_t reg) {
	if (reg > 0x7F) {
		reg = 0x7F;
	}

	return ((float) reg * L6470_STALL_TH_STEP_MA) + (float) L6470_STALL_TH_MIN_MA;
}

static float SlpCalcValueReg(uint8_t reg) {
	return ((float) reg * L6470_SLP_STEP) + (float) L6470_SLP_MIN;
}

static float KThermCalcValueReg(uint8_t reg) {
	if (reg > 0x0F) {
		reg = 0x0F;
	}

	return ((float) reg * L6470_K_THERM_STEP) + (float) L6470_K_THERM_MIN;
}
#endif

void L6470::Dump(void) {
#ifndef NDEBUG
	uint8_t reg;

	printf("Registers:\n");
	printf("01:ABS_POS    - Current position: %ld\n", getPos());
	printf("02:EL_POS     - Electrical position: %ld\n", getParam(L6470_PARAM_EL_POS));
	printf("03:MARK       - Mark position: %ld\n", getMark());
	printf("04:SPEED      - Current speed: %ld (raw)\n", getParam(L6470_PARAM_SPEED));
	printf("05:ACC        - Acceleration: %.2f steps/s2\n", getAcc());
	printf("06:DEC        - Deceleration: %.2f steps/s2\n", getDec());
	printf("07:MAX_SPEED  - Maximum speed: %.2f steps/s\n", getMaxSpeed());
	printf("08:MIN_SPEED  - Minimum speed: %.2f steps/s\n", getMinSpeed());
	printf("09:KVAL_HOLD  - Holding KVAL: %d\n", (int) getHoldKVAL());
	printf("0A:KVAL_RUN   - Constant speed KVAL: %d\n", (int) getRunKVAL());
	printf("0B:KVAL_ACC   - Acceleration starting KVAL: %d\n", (int) getAccKVAL());
	printf("0C:KVAL_Dec   - Deceleration starting KVAL: %d\n", (int) getDecKVAL());
	printf("0D:INT_SPEED  - Intersect speed: 0x%.4X (raw)\n", (unsigned int) getParam(L6470_PARAM_INT_SPD));
	reg = getParam(L6470_PARAM_ST_SLP);
	printf("0E:ST_SLP     - Start slope: %.3f\%% s/step (0x%.2X)\n", (float) 100 * SlpCalcValueReg(reg), reg);
	reg = getParam(L6470_PARAM_FN_SLP_ACC);
	printf("0F:FN_SLP_ACC - Acceleration final slope: %.3f\%% s/step (0x%.2X)\n", (float) 100 * SlpCalcValueReg(reg), reg);
	reg = getParam(L6470_PARAM_FN_SLP_DEC);
	printf("10:FN_SLP_DEC - Deceleration final slope: %.3f\%% s/step (0x%.2X)\n", (float) 100 * SlpCalcValueReg(reg), reg);
	reg = getParam(L6470_PARAM_K_THERM);
	printf("11:K_THERM    - Thermal compensation factor: %.3f (0x%.2X)\n", KThermCalcValueReg(reg), reg);
	reg = getParam(L6470_PARAM_ADC_OUT);
	printf("12:ADC_OUT    - ADC output: 0x%.2X\n", reg & 0x1F);
	reg = getOCThreshold();
	printf("13:OCD_TH     - OCD threshold: %d mA (0x%.2X)\n", OcdThCalcValueReg(reg), reg);
	reg = getParam(L6470_PARAM_STALL_TH);
	printf("14:STALL_TH   - STALL threshold: %d mA (0x%.2X)\n", StallThCalcValueReg(reg & 0x7F), reg);
	printf("15:FS_SPD     - Full-step speed: %.2f steps/s\n", getFullSpeed());
	printf("16:STEP_MODE  - Step mode: %d microsteps\n", 1 << getStepMode());
	printf("17:ALARM_EN   - Alarm enable: 0x%.2X\n", (unsigned int) getParam(L6470_PARAM_ALARM_EN));
	printf("18:CONFIG     - IC configuration: 0x%.4X\n", (unsigned int) getParam(L6470_PARAM_CONFIG));
#endif
}

