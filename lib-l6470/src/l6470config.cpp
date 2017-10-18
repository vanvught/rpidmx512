/**
 * @file l6470config.cpp
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
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

#include <stdint.h>
#include <stdbool.h>

#include "l6470.h"
#include "l6470constants.h"

void L6470::configSyncPin(uint8_t pinFunc, uint8_t syncSteps) {
	uint8_t syncPinConfig = (uint8_t) getParam(L6470_PARAM_STEP_MODE);

	syncPinConfig &= 0x0F;
	syncPinConfig |= ((pinFunc & L6470_SYNC_EN) | (syncSteps & L6470_SYNC_SEL_MASK));

	setParam(L6470_PARAM_STEP_MODE, (unsigned long) syncPinConfig);
}

void L6470::configStepMode(uint8_t stepMode) {
	uint8_t stepModeConfig = (uint8_t) getParam(L6470_PARAM_STEP_MODE);

	stepModeConfig &= ~(L6470_STEP_SEL_MASK);
	stepModeConfig |= (stepMode & L6470_STEP_SEL_MASK);

	setParam(L6470_PARAM_STEP_MODE, (unsigned long) stepModeConfig);
}

uint8_t L6470::getStepMode(void) {
	return (uint8_t) (getParam(L6470_PARAM_STEP_MODE) & L6470_STEP_SEL_MASK);
}

void L6470::setMaxSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = maxSpdCalc(stepsPerSecond);

	setParam(L6470_PARAM_MAX_SPEED, integerSpeed);
}

float L6470::getMaxSpeed(void) {
	return maxSpdParse(getParam(L6470_PARAM_MAX_SPEED));
}

void L6470::setMinSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = minSpdCalc(stepsPerSecond);
	unsigned long temp = getParam(L6470_PARAM_MIN_SPEED) & L6470_LSPD_OPT;

	setParam(L6470_PARAM_MIN_SPEED, integerSpeed | temp);
}

float L6470::getMinSpeed(void) {
	return minSpdParse(getParam(L6470_PARAM_MIN_SPEED));
}

void L6470::setFullSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = FSCalc(stepsPerSecond);
	setParam(L6470_PARAM_FS_SPD, integerSpeed);
}

float L6470::getFullSpeed(void) {
	return FSParse(getParam(L6470_PARAM_FS_SPD));
}

void L6470::setAcc(float stepsPerSecondPerSecond) {
	unsigned long integerAcc = accCalc(stepsPerSecondPerSecond);
	setParam(L6470_PARAM_ACC, integerAcc);
}

float L6470::getAcc(void) {
	return accParse(getParam(L6470_PARAM_ACC));
}

void L6470::setDec(float stepsPerSecondPerSecond) {
	unsigned long integerDec = decCalc(stepsPerSecondPerSecond);
	setParam(L6470_PARAM_DECEL, integerDec);
}

float L6470::getDec(void) {
	return accParse(getParam(L6470_PARAM_DECEL));
}

void L6470::setOCThreshold(uint8_t threshold) {
	setParam(L6470_PARAM_OCD_TH, 0x0F & threshold);
}

uint8_t L6470::getOCThreshold(void) {
	return (uint8_t) (getParam(L6470_PARAM_OCD_TH) & 0xF);
}

void L6470::setPWMFreq(int divisor, int multiplier) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_F_PWM_INT_MASK);
	configVal &= ~(L6470_CONFIG_F_PWM_DEC_MASK);
	configVal |= ((L6470_CONFIG_F_PWM_INT_MASK & divisor) | (L6470_CONFIG_F_PWM_DEC_MASK & multiplier));

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getPWMFreqDivisor(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_F_PWM_INT_MASK);
}

int L6470::getPWMFreqMultiplier(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_F_PWM_DEC_MASK);
}

void L6470::setSlewRate(int slewRate) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_POW_SR_MASK);
	configVal |= (L6470_CONFIG_POW_SR_MASK & slewRate);

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getSlewRate(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_POW_SR_MASK);
}

void L6470::setOCShutdown(int OCShutdown) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_OC_SD_MASK);
	configVal |= (L6470_CONFIG_OC_SD_MASK & OCShutdown);

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getOCShutdown(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_OC_SD_MASK);
}

void L6470::setVoltageComp(int vsCompMode) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_EN_VSCOMP_MASK);
	configVal |= (L6470_CONFIG_EN_VSCOMP_MASK & vsCompMode);

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getVoltageComp(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_EN_VSCOMP_MASK);
}

void L6470::setSwitchMode(int switchMode) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_SW_MODE_MASK);
	configVal |= (L6470_CONFIG_SW_MODE_MASK & switchMode);

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getSwitchMode(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_SW_MODE_MASK);
}

void L6470::setOscMode(int oscillatorMode) {
	unsigned long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_OSC_SEL_MASK);
	configVal |= (L6470_CONFIG_OSC_SEL_MASK & oscillatorMode);

	setParam(L6470_PARAM_CONFIG, configVal);
}

int L6470::getOscMode(void) {
	return (int) (getParam(L6470_PARAM_CONFIG) & 0x000F);
}

void L6470::setAccKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_ACC, kvalInput);
}

uint8_t L6470::getAccKVAL(void) {
	return (uint8_t) getParam(L6470_PARAM_KVAL_ACC);
}

void L6470::setDecKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_DEC, kvalInput);
}

uint8_t L6470::getDecKVAL(void) {
	return (uint8_t) getParam(L6470_PARAM_KVAL_DEC);
}

void L6470::setRunKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_RUN, kvalInput);
}

uint8_t L6470::getRunKVAL(void) {
	return (uint8_t) getParam(L6470_PARAM_KVAL_RUN);
}

void L6470::setHoldKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_HOLD, kvalInput);
}

uint8_t L6470::getHoldKVAL(void) {
	return (uint8_t) getParam(L6470_PARAM_KVAL_HOLD);
}

void L6470::setLoSpdOpt(bool enable) {
	unsigned long temp = getParam(L6470_PARAM_MIN_SPEED);

	if (enable) {
		temp |= L6470_LSPD_OPT;
	} else {
		temp &= (~L6470_LSPD_OPT);
	}

	setParam(L6470_PARAM_MIN_SPEED, temp);
}

bool L6470::getLoSpdOpt(void) {
	return (bool) ((getParam(L6470_PARAM_MIN_SPEED) & L6470_LSPD_OPT) != 0);
}
