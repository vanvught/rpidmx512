/**
 * @file l6470config.cpp
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
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

#include <stdint.h>

#include "l6470.h"
#include "l6470constants.h"

void L6470::configSyncPin(uint8_t pinFunc, uint8_t syncSteps) {
	uint8_t syncPinConfig = getParam(L6470_PARAM_STEP_MODE);

	syncPinConfig &= 0x0F;
	syncPinConfig |= ((pinFunc & L6470_SYNC_EN) | (syncSteps & L6470_SYNC_SEL_MASK));

	setParam(L6470_PARAM_STEP_MODE, syncPinConfig);
}

void L6470::configStepMode(uint8_t stepMode) {
	uint8_t stepModeConfig = getParam(L6470_PARAM_STEP_MODE);

	stepModeConfig &= ~(L6470_STEP_SEL_MASK);
	stepModeConfig |= (stepMode & L6470_STEP_SEL_MASK);

	setParam(L6470_PARAM_STEP_MODE, stepModeConfig);
}

uint8_t L6470::getStepMode() {
	return (getParam(L6470_PARAM_STEP_MODE) & L6470_STEP_SEL_MASK);
}

void L6470::setMaxSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = maxSpdCalc(stepsPerSecond);

	setParam(L6470_PARAM_MAX_SPEED, integerSpeed);
}

float L6470::getMaxSpeed() {
	return maxSpdParse(static_cast<unsigned long>(getParam(L6470_PARAM_MAX_SPEED)));
}

void L6470::setMinSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = minSpdCalc(stepsPerSecond);
	unsigned long temp = getParam(L6470_PARAM_MIN_SPEED) & L6470_LSPD_OPT;

	setParam(L6470_PARAM_MIN_SPEED, integerSpeed | temp);
}

float L6470::getMinSpeed() {
	return minSpdParse(static_cast<unsigned long>(getParam(L6470_PARAM_MIN_SPEED)));
}

void L6470::setFullSpeed(float stepsPerSecond) {
	unsigned long integerSpeed = FSCalc(stepsPerSecond);
	setParam(L6470_PARAM_FS_SPD, integerSpeed);
}

float L6470::getFullSpeed() {
	return FSParse(static_cast<unsigned long>(getParam(L6470_PARAM_FS_SPD)));
}

void L6470::setAcc(float stepsPerSecondPerSecond) {
	unsigned long integerAcc = accCalc(stepsPerSecondPerSecond);
	setParam(L6470_PARAM_ACC, integerAcc);
}

float L6470::getAcc() {
	return accParse(static_cast<unsigned long>(getParam(L6470_PARAM_ACC)));
}

void L6470::setDec(float stepsPerSecondPerSecond) {
	unsigned long integerDec = decCalc(stepsPerSecondPerSecond);
	setParam(L6470_PARAM_DECEL, integerDec);
}

float L6470::getDec() {
	return accParse(static_cast<unsigned long>(getParam(L6470_PARAM_DECEL)));
}

void L6470::setOCThreshold(uint8_t threshold) {
	setParam(L6470_PARAM_OCD_TH, 0x0F & threshold);
}

uint8_t L6470::getOCThreshold() {
	return (getParam(L6470_PARAM_OCD_TH) & 0xF);
}

void L6470::setPWMFreq(int divisor, int multiplier) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_F_PWM_INT_MASK);
	configVal &= ~(L6470_CONFIG_F_PWM_DEC_MASK);
	configVal |= ((L6470_CONFIG_F_PWM_INT_MASK & divisor) | (L6470_CONFIG_F_PWM_DEC_MASK & multiplier));

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getPWMFreqDivisor() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_F_PWM_INT_MASK);
}

int L6470::getPWMFreqMultiplier() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_F_PWM_DEC_MASK);
}

void L6470::setSlewRate(int slewRate) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_POW_SR_MASK);
	configVal |= (L6470_CONFIG_POW_SR_MASK & slewRate);

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getSlewRate() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_POW_SR_MASK);
}

void L6470::setOCShutdown(int OCShutdown) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_OC_SD_MASK);
	configVal |= (L6470_CONFIG_OC_SD_MASK & OCShutdown);

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getOCShutdown() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_OC_SD_MASK);
}

void L6470::setVoltageComp(int vsCompMode) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_EN_VSCOMP_MASK);
	configVal |= (L6470_CONFIG_EN_VSCOMP_MASK & vsCompMode);

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getVoltageComp() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_EN_VSCOMP_MASK);
}

void L6470::setSwitchMode(int switchMode) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_SW_MODE_MASK);
	configVal |= (L6470_CONFIG_SW_MODE_MASK & switchMode);

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getSwitchMode() {
	return (getParam(L6470_PARAM_CONFIG) & L6470_CONFIG_SW_MODE_MASK);
}

void L6470::setOscMode(int oscillatorMode) {
	long configVal = getParam(L6470_PARAM_CONFIG);

	configVal &= ~(L6470_CONFIG_OSC_SEL_MASK);
	configVal |= (L6470_CONFIG_OSC_SEL_MASK & oscillatorMode);

	setParam(L6470_PARAM_CONFIG, static_cast<unsigned long>(configVal));
}

int L6470::getOscMode() {
	return (getParam(L6470_PARAM_CONFIG) & 0x000F);
}

void L6470::setAccKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_ACC, kvalInput);
}

uint8_t L6470::getAccKVAL() {
	return getParam(L6470_PARAM_KVAL_ACC);
}

void L6470::setDecKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_DEC, kvalInput);
}

uint8_t L6470::getDecKVAL() {
	return getParam(L6470_PARAM_KVAL_DEC);
}

void L6470::setRunKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_RUN, kvalInput);
}

uint8_t L6470::getRunKVAL() {
	return getParam(L6470_PARAM_KVAL_RUN);
}

void L6470::setHoldKVAL(uint8_t kvalInput) {
	setParam(L6470_PARAM_KVAL_HOLD, kvalInput);
}

uint8_t L6470::getHoldKVAL() {
	return getParam(L6470_PARAM_KVAL_HOLD);
}

void L6470::setLoSpdOpt(bool enable) {
	long temp = getParam(L6470_PARAM_MIN_SPEED);

	if (enable) {
		temp |= L6470_LSPD_OPT;
	} else {
		temp &= (~L6470_LSPD_OPT);
	}

	setParam(L6470_PARAM_MIN_SPEED, static_cast<unsigned long>(temp));
}

bool L6470::getLoSpdOpt() {
	return ((getParam(L6470_PARAM_MIN_SPEED) & L6470_LSPD_OPT) != 0);
}
