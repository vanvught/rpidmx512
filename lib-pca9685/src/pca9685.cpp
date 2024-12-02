/**
 * @file pca9685.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "hal_i2c.h"
#include "hal_gpio.h"

#include "pca9685.h"

#include "debug.h"

#define DIV_ROUND_UP(n,d)	(((n) + static_cast<float>(d) - 1) / static_cast<float>(d))

#define PCA9685_OSC_FREQ 25000000L

enum TPCA9685Reg {
	PCA9685_REG_MODE1 = 0x00,
	PCA9685_REG_MODE2 = 0x01,
	PCA9685_REG_ALLCALLADR = 0x05,
	PCA9685_REG_LED0_ON_L = 0x06,
	PCA9685_REG_LED0_ON_H = 0x07,
	PCA9685_REG_LED0_OFF_L = 0x08,
	PCA9685_REG_LED0_OFF_H = 0x09,
	PCA9685_REG_ALL_LED_ON_L = 0xFA,
	PCA9685_REG_ALL_LED_ON_H = 0xFB,
	PCA9685_REG_ALL_LED_OFF_L = 0xFC,
	PCA9685_REG_ALL_LED_OFF_H = 0xFD,
	PCA9685_REG_PRE_SCALE = 0xFE
};

#define PCA9685_PRE_SCALE_MIN	0x03
#define PCA9685_PRE_SCALE_MAX	0xFF

/*
 * 7.3.1 Mode register 1, MODE1
 */
enum TPCA9685Mode1 {
	PCA9685_MODE1_ALLCALL = 1 << 0,
	PCA9685_MODE1_SUB3 = 1 << 1,
	PCA9685_MODE1_SUB2 = 1 << 2,
	PCA9685_MODE1_SUB1 = 1 << 3,
	PCA9685_MODE1_SLEEP = 1 << 4,
	PCA9685_MODE1_AI = 1 << 5,
	PCA9685_MODE1_EXTCLK = 1 << 6,
	PCA9685_MODE1_RESTART = 1 << 7
};

/*
 * 7.3.2 Mode register 2, MODE2
 */
enum TPCA9685Mode2 {
	PCA9685_MODE2_OUTDRV = 1 << 2,
	PCA9685_MODE2_OCH = 1 << 3,
	PCA9685_MODE2_INVRT = 1 << 4
};

enum TPCA9685Och {
	PCA9685_OCH_STOP = 0,
	PCA9685_OCH_ACK = 1 << 3
};

PCA9685::PCA9685(uint8_t nAddress) : m_nAddress(nAddress) {
	FUNC_PREFIX(i2c_begin());

	AutoIncrement(true);

	for (uint32_t i = 0; i < 16; i ++) {
		Write(static_cast<uint8_t>(i), static_cast<uint16_t>(0), static_cast<uint16_t>(0x1000));
	}

	Sleep(false);
}

void PCA9685::Sleep(bool bMode) {
	auto nData = I2cReadReg(PCA9685_REG_MODE1);

	nData &= static_cast<uint8_t>(~PCA9685_MODE1_SLEEP);

	if (bMode) {
		nData |= PCA9685_MODE1_SLEEP;
	}

	I2cWriteReg(PCA9685_REG_MODE1, nData);

//	if (nData & ~PCA9685_MODE1_RESTART) {
//		udelay(500);
//		nData |= PCA9685_MODE1_RESTART;
//	}
}

void PCA9685::SetPreScaller(uint8_t nPrescale) {
	nPrescale = nPrescale < PCA9685_PRE_SCALE_MIN ? PCA9685_PRE_SCALE_MIN : nPrescale;

	Sleep(true);
	I2cWriteReg(PCA9685_REG_PRE_SCALE, nPrescale);
	Sleep(false);
}

uint8_t PCA9685::GetPreScaller() {
	return 	I2cReadReg(PCA9685_REG_PRE_SCALE);
}

void PCA9685::SetFrequency(uint16_t nFreq) {
	SetPreScaller(CalcPresScale(nFreq));
}

uint16_t PCA9685::GetFrequency() {
	return CalcFrequency(GetPreScaller());
}

void PCA9685::SetOCH(pca9685::Och och) {
	auto nData = I2cReadReg(PCA9685_REG_MODE2);

	nData &= static_cast<uint8_t>(~PCA9685_MODE2_OCH);

	if (och == pca9685::Och::PCA9685_OCH_ACK) {
		nData |= PCA9685_OCH_ACK;
	} // else, default Outputs change on STOP command

	I2cWriteReg(PCA9685_REG_MODE2, nData);
}

pca9685::Och PCA9685::GetOCH() {
	const auto isOchACK = (I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_OCH) == PCA9685_MODE2_OCH;
	return isOchACK ? pca9685::Och::PCA9685_OCH_ACK : pca9685::Och::PCA9685_OCH_STOP;
}

void PCA9685::SetInvert(const pca9685::Invert invert) {
	auto Data = I2cReadReg(PCA9685_REG_MODE2);
	Data &= static_cast<uint8_t>(~PCA9685_MODE2_INVRT);

	if (invert == pca9685::Invert::OUTPUT_INVERTED) {
		Data |= PCA9685_MODE2_INVRT;
	}

	I2cWriteReg(PCA9685_REG_MODE2, Data);
}

pca9685::Invert PCA9685::GetInvert() {
	const auto nData = I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_INVRT;
	return (nData == PCA9685_MODE2_INVRT) ? pca9685::Invert::OUTPUT_INVERTED : pca9685::Invert::OUTPUT_NOT_INVERTED;
}

void PCA9685::SetOutDriver(const pca9685::Output output) {
	auto nData = I2cReadReg(PCA9685_REG_MODE2);
	nData &= static_cast<uint8_t>(~PCA9685_MODE2_OUTDRV);

	if (output == pca9685::Output::DRIVER_TOTEMPOLE) {
		nData |= PCA9685_MODE2_OUTDRV;
	}

	I2cWriteReg(PCA9685_REG_MODE2, nData);
}

pca9685::Output PCA9685::GetOutDriver() {
	const auto nData = I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_OUTDRV;
	return (nData == PCA9685_MODE2_OUTDRV) ? pca9685::Output::DRIVER_TOTEMPOLE : pca9685::Output::DRIVER_OPENDRAIN;
}

void PCA9685::Write(const uint32_t nChannel, const uint16_t nOn, const uint16_t nOff) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = static_cast<uint8_t>(PCA9685_REG_LED0_ON_L + (nChannel << 2));
	} else {
		reg = PCA9685_REG_ALL_LED_ON_L;
	}

	I2cWriteReg(reg, nOn, nOff);
}

void PCA9685::Write(const uint32_t nChannel, const uint16_t nValue) {
	Write(nChannel, static_cast<uint16_t>(0), nValue);
}

void PCA9685::Write(const uint16_t nOn, const uint16_t nOff) {
	Write(static_cast<uint32_t>(16), nOn, nOff);
}

void PCA9685::Write(const uint16_t nValue) {
	Write(static_cast<uint32_t>(16), nValue);
}

void PCA9685::Read(const uint32_t nChannel, uint16_t *pOn, uint16_t *pOff) {
	assert(pOn != nullptr);
	assert(pOff != nullptr);

	uint8_t reg;

	if (nChannel <= 15) {
		reg = static_cast<uint8_t>(PCA9685_REG_LED0_ON_L + (nChannel << 2));
	} else {
		reg = PCA9685_REG_ALL_LED_ON_L;
	}

	if (pOn != nullptr) {
		*pOn = I2cReadReg16(reg);
	}

	if (pOff) {
		*pOff = I2cReadReg16(static_cast<uint8_t>(reg + 2));
	}
}

void PCA9685::Read(uint16_t *pOn, uint16_t *pOff) {
	Read(static_cast<uint8_t>(16), pOn, pOff);
}

void PCA9685::SetFullOn(const uint32_t nChannel, const bool bMode) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = static_cast<uint8_t>(PCA9685_REG_LED0_ON_H + (nChannel << 2));
	} else {
		reg = PCA9685_REG_ALL_LED_ON_H;
	}

	uint8_t Data = I2cReadReg(reg);

	Data = bMode ? (Data | 0x10) : (Data & 0xEF);

	I2cWriteReg(reg, Data);

	if (bMode) {
		SetFullOff(nChannel, false);
	}

}

void PCA9685::SetFullOff(const uint32_t nChannel, const bool bMode) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = static_cast<uint8_t>(PCA9685_REG_LED0_OFF_H + (nChannel << 2));
	} else {
		reg = PCA9685_REG_ALL_LED_OFF_H;
	}

	uint8_t Data = I2cReadReg(reg);

	Data = bMode ? (Data | 0x10) : (Data & 0xEF);

	I2cWriteReg(reg, Data);
}

uint8_t PCA9685::CalcPresScale(uint32_t nFrequency) {
	nFrequency = (nFrequency > pca9685::Frequency::RANGE_MAX ? pca9685::Frequency::RANGE_MAX : (nFrequency < pca9685::Frequency::RANGE_MIN ? pca9685::Frequency::RANGE_MIN : nFrequency));

	constexpr auto f = static_cast<float>(PCA9685_OSC_FREQ) / 4096U;
	const auto nData = DIV_ROUND_UP(f, nFrequency) - 1U;

	return static_cast<uint8_t>(nData);
}

uint16_t PCA9685::CalcFrequency(uint32_t nPreScale) {
	constexpr auto f = static_cast<float>(PCA9685_OSC_FREQ) / 4096U;
	const auto Data =static_cast<uint32_t>(DIV_ROUND_UP(f, (nPreScale) + 1U));

	uint32_t nFrequencyMin;

	for (nFrequencyMin = Data; nFrequencyMin > pca9685::Frequency::RANGE_MIN; nFrequencyMin--) {
		if (CalcPresScale(nFrequencyMin) != nPreScale) {
			break;
		}
	}

	uint32_t nFrequencyMax;

	for (nFrequencyMax = Data; nFrequencyMax < pca9685::Frequency::RANGE_MAX; nFrequencyMax++) {
		if (CalcPresScale(nFrequencyMax) != nPreScale) {
			break;
		}
	}

	return static_cast<uint16_t>(nFrequencyMax + nFrequencyMin) / 2;
}

void PCA9685::Dump() {
#ifndef NDEBUG
	uint8_t reg = I2cReadReg(PCA9685_REG_MODE1);

	printf("MODE1 - Mode register 1 (address 00h) : %02Xh\n", reg);
	printf("\tbit 7 - RESTART : Restart %s\n", reg & PCA9685_MODE1_RESTART ? "enabled" : "disabled");
	printf("\tbit 6 - EXTCLK  : %s\n", reg & PCA9685_MODE1_EXTCLK ? "Use EXTCLK pin clock" : "Use internal clock");
	printf("\tbit 5 - AI      : Register Auto-Increment %s\n", reg & PCA9685_MODE1_AI ? "enabled" : "disabled");
	printf("\tbit 4 - SLEEP   : %s\n", reg & PCA9685_MODE1_SLEEP ? "Low power mode. Oscillator off" : "Normal mode");
	printf("\tbit 3 - SUB1    : PCA9685 %s to I2C-bus subaddress 1\n", reg & PCA9685_MODE1_SUB1 ? "responds" : "does not respond");
	printf("\tbit 2 - SUB1    : PCA9685 %s to I2C-bus subaddress 2\n", reg & PCA9685_MODE1_SUB2 ? "responds" : "does not respond");
	printf("\tbit 1 - SUB1    : PCA9685 %s to I2C-bus subaddress 3\n", reg & PCA9685_MODE1_SUB3 ? "responds" : "does not respond");
	printf("\tbit 0 - ALLCALL : PCA9685 %s to LED All Call I2C-bus address\n", reg & PCA9685_MODE1_ALLCALL ? "responds" : "does not respond");

	reg = I2cReadReg(PCA9685_REG_MODE2);

	printf("\nMODE2 - Mode register 2 (address 01h) : %02Xh\n", reg);
	printf("\tbit 7 to 5      : Reserved\n");
	printf("\tbit 4 - INVRT   : Output logic state %sinverted\n", reg & PCA9685_MODE2_INVRT ? "" : "not ");
	printf("\tbit 3 - OCH     : Outputs change on %s\n", reg & PCA9685_MODE2_OCH ? "ACK" : "STOP command");
	printf("\tbit 2 - OUTDRV  : The 16 LEDn outputs are configured with %s structure\n", reg & PCA9685_MODE2_OUTDRV ? "a totem pole" : "an open-drain");
	printf("\tbit 10- OUTNE   : %01x\n", reg & 0x3);

	reg = I2cReadReg(PCA9685_REG_PRE_SCALE);

	printf("\nPRE_SCALE register (address FEh) : %02Xh\n", reg);
	printf("\t Frequency : %d Hz\n", CalcFrequency(reg));

	puts("");

	uint16_t on, off;

	for (uint32_t nLed = 0; nLed <= 15; nLed ++) {
		Read(nLed, &on, &off);
		printf("LED%d_ON  : %04x\n", nLed, on);
		printf("LED%d_OFF : %04x\n", nLed, off);
	}

	puts("");

	Read(16, &on, &off);
	printf("ALL_LED_ON  : %04x\n", on);
	printf("ALL_LED_OFF : %04x\n", off);
#endif
}

void PCA9685::AutoIncrement(bool bMode) {
	auto nData = I2cReadReg(PCA9685_REG_MODE1);

	nData &= static_cast<uint8_t>(~PCA9685_MODE1_AI);	// 0 Register Auto-Increment disabled. {default}

	if (bMode) {
		nData |= PCA9685_MODE1_AI;	// 1 Register Auto-Increment enabled.
	}

	I2cWriteReg(PCA9685_REG_MODE1, nData);
}

void PCA9685::I2cSetup() {
	FUNC_PREFIX(i2c_set_address(m_nAddress));
	FUNC_PREFIX(i2c_set_baudrate(HAL_I2C::FULL_SPEED));
}

void PCA9685::I2cWriteReg(uint8_t reg, uint8_t data) {
	char buffer[2];

	buffer[0] = reg;
	buffer[1] = data;

	I2cSetup();

	FUNC_PREFIX(i2c_write(buffer, 2));
}

uint8_t PCA9685::I2cReadReg(uint8_t reg) {
	char data = reg;

	I2cSetup();

	FUNC_PREFIX(i2c_write(&data, 1));
	FUNC_PREFIX(i2c_read(&data, 1));

	return data;
}

void PCA9685::I2cWriteReg(uint8_t reg, uint16_t data) {
	char buffer[3];

	buffer[0] = reg;
	buffer[1] = static_cast<char>(data & 0xFF);
	buffer[2] = static_cast<char>(data >> 8);

	I2cSetup();

	FUNC_PREFIX(i2c_write(buffer, 3));
}

uint16_t PCA9685::I2cReadReg16(uint8_t reg) {
	char data = reg;
	char buffer[2] = { 0, 0 };

	I2cSetup();

	FUNC_PREFIX(i2c_write(&data, 1));
	FUNC_PREFIX(i2c_read(reinterpret_cast<char *>(&buffer), 2));

	return static_cast<uint16_t>((buffer[1] << 8) | buffer[0]);
}

void PCA9685::I2cWriteReg(uint8_t reg, uint16_t data, uint16_t data2) {
	char buffer[5];

	buffer[0] = reg;
	buffer[1] = static_cast<char>(data & 0xFF);
	buffer[2] = static_cast<char>(data >> 8);
	buffer[3] = static_cast<char>(data2 & 0xFF);
	buffer[4] = static_cast<char>(data2 >> 8);

	I2cSetup();

	FUNC_PREFIX(i2c_write(buffer, 5));
}
