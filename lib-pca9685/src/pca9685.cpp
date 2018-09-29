/**
 * @file pca9685.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#if !defined(NDEBUG) || defined(__linux__) || defined(__circle__)
 #include <stdio.h>
#endif
#include <assert.h>

#include "bcm2835.h"

#if defined(__linux__)
#elif defined(__circle__)
#else
 #include "bcm2835_i2c.h"
#endif

#include "pca9685.h"

extern "C" {
#if defined(__linux__)
 extern void bcm2835_delayMicroseconds (uint64_t);
 #define udelay bcm2835_delayMicroseconds
#else
#endif
}

#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

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

PCA9685::PCA9685(uint8_t nAddress) : m_nAddress(nAddress) {
#if defined (__linux__) || defined(__circle__)
	if (bcm2835_init() == 0) {
		printf("Not able to init the bmc2835 library\n");
	}
#endif

	bcm2835_i2c_begin();

	AutoIncrement(true);

	for (uint8_t i = 0; i < 16; i ++) {
		Write(i, (uint16_t) 0, (uint16_t) 0x1000);
	}

	Sleep(false);
}

PCA9685::~PCA9685(void) {
}

void PCA9685::Sleep(bool bMode) {
	uint8_t Data = I2cReadReg(PCA9685_REG_MODE1);

	Data &= ~PCA9685_MODE1_SLEEP;

	if (bMode) {
		Data |= PCA9685_MODE1_SLEEP;
	}

	I2cWriteReg(PCA9685_REG_MODE1, Data);

	if (Data & ~PCA9685_MODE1_RESTART) {
		udelay(500);
		Data |= PCA9685_MODE1_RESTART;
	}
}

void PCA9685::SetPreScaller(uint8_t nPrescale) {
	nPrescale = nPrescale < PCA9685_PRE_SCALE_MIN ? PCA9685_PRE_SCALE_MIN : nPrescale;

	Sleep(true);
	I2cWriteReg(PCA9685_REG_PRE_SCALE, nPrescale);
	Sleep(false);
}

uint8_t PCA9685::GetPreScaller(void) {
	return 	I2cReadReg(PCA9685_REG_PRE_SCALE);
}

void PCA9685::SetFrequency(uint16_t nFreq) {
	SetPreScaller(CalcPresScale(nFreq));
}

uint16_t PCA9685::GetFrequency(void) {
	return CalcFrequency(GetPreScaller());
}

void PCA9685::SetOCH(TPCA9685Och enumTPCA9685Och) {
	uint8_t Data = I2cReadReg(PCA9685_REG_MODE2);

	Data &= ~PCA9685_MODE2_OCH;

	if (enumTPCA9685Och == PCA9685_OCH_ACK) {
		Data |= PCA9685_OCH_ACK;
	} // else, default Outputs change on STOP command

	I2cWriteReg(PCA9685_REG_MODE2, Data);
}

TPCA9685Och PCA9685::GetOCH(void) {
	const uint8_t Data = I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_OCH;

	return (TPCA9685Och) Data;
}

void PCA9685::SetInvert(bool bInvert) {
	uint8_t Data = I2cReadReg(PCA9685_REG_MODE2);

	Data &= ~PCA9685_MODE2_INVRT;

	if (bInvert) {
		Data |= PCA9685_MODE2_INVRT;
	}

	I2cWriteReg(PCA9685_REG_MODE2, Data);
}

bool PCA9685::GetInvert(void) {
	const uint8_t Data = I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_INVRT;

	return (Data == PCA9685_MODE2_INVRT);
}

void PCA9685::SetOutDriver(bool bOutDriver) {
	uint8_t Data = I2cReadReg(PCA9685_REG_MODE2);

	Data &= ~PCA9685_MODE2_OUTDRV;

	if (bOutDriver) {
		Data |= PCA9685_MODE2_OUTDRV;
	}

	I2cWriteReg(PCA9685_REG_MODE2, Data);
}

bool PCA9685::GetOutDriver(void) {
	const uint8_t Data = I2cReadReg(PCA9685_REG_MODE2) & PCA9685_MODE2_OUTDRV;

	return (Data == PCA9685_MODE2_OUTDRV);
}

void PCA9685::Write(uint8_t nChannel, uint16_t nOn, uint16_t nOff) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = PCA9685_REG_LED0_ON_L + (nChannel << 2);
	} else {
		reg = PCA9685_REG_ALL_LED_ON_L;
	}

	I2cWriteReg(reg, nOn, nOff);
}

void PCA9685::Write(uint8_t nChannel, uint16_t nValue) {
	Write (nChannel, (uint16_t) 0, nValue);
}

void PCA9685::Write(uint16_t nOn, uint16_t nOff) {
	Write((uint8_t) 16, nOn, nOff);
}

void PCA9685::Write(uint16_t nValue) {
	Write((uint8_t) 16, nValue);
}

void PCA9685::Read(uint8_t nChannel, uint16_t *pOn, uint16_t *pOff) {
	assert(pOn != 0);
	assert(pOff != 0);

	uint8_t reg;

	if (nChannel <= 15) {
		reg = PCA9685_REG_LED0_ON_L + (nChannel << 2);
	} else {
		reg = PCA9685_REG_ALL_LED_ON_L;
	}

	if (pOn != 0) {
		*pOn = I2cReadReg16(reg);
	}

	if (pOff) {
		*pOff = I2cReadReg16(reg + 2);
	}
}

void PCA9685::Read(uint16_t *pOn, uint16_t *pOff) {
	Read((uint8_t) 16, pOn, pOff);
}

void PCA9685::SetFullOn(uint8_t nChannel, bool bMode) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = PCA9685_REG_LED0_ON_H + (nChannel << 2);
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

void PCA9685::SetFullOff(uint8_t nChannel, bool bMode) {
	uint8_t reg;

	if (nChannel <= 15) {
		reg = PCA9685_REG_LED0_OFF_H + (nChannel << 2);
	} else {
		reg = PCA9685_REG_ALL_LED_OFF_H;
	}

	uint8_t Data = I2cReadReg(reg);

	Data = bMode ? (Data | 0x10) : (Data & 0xEF);

	I2cWriteReg(reg, Data);
}

uint8_t PCA9685::CalcPresScale(uint16_t nFreq) {
	nFreq = (nFreq > PCA9685_FREQUENCY_MAX ? PCA9685_FREQUENCY_MAX : (nFreq < PCA9685_FREQUENCY_MIN ? PCA9685_FREQUENCY_MIN : nFreq));

	const float f = (float) PCA9685_OSC_FREQ / 4096;

	const uint8_t Data = (uint8_t) DIV_ROUND_UP(f, nFreq) - 1;

	return Data;
}

uint16_t PCA9685::CalcFrequency(uint8_t nPreScale) {
	uint16_t f_min;
	uint16_t f_max;
	const float f = (float) PCA9685_OSC_FREQ / 4096;
	const uint16_t Data = (uint16_t) DIV_ROUND_UP(f, ((uint16_t) nPreScale + 1));

	for (f_min = Data; f_min > PCA9685_FREQUENCY_MIN; f_min--) {
		if (CalcPresScale(f_min) != nPreScale) {
			break;
		}
	}

	for (f_max = Data; f_max < PCA9685_FREQUENCY_MAX; f_max++) {
		if (CalcPresScale(f_max) != nPreScale) {
			break;
		}
	}

	return (f_max + f_min) / 2;
}

void PCA9685::Dump(void) {
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

	printf("\n");

	uint16_t on, off;

	for (uint8_t nLed = 0; nLed <= 15; nLed ++) {
		Read(nLed, &on, &off);
		printf("LED%d_ON  : %04x\n", nLed, on);
		printf("LED%d_OFF : %04x\n", nLed, off);
	}

	printf("\n");

	Read(16, &on, &off);
	printf("ALL_LED_ON  : %04x\n", on);
	printf("ALL_LED_OFF : %04x\n", off);
#endif
}

void PCA9685::AutoIncrement(bool bMode) {
	uint8_t Data = I2cReadReg(PCA9685_REG_MODE1);

	Data &= ~PCA9685_MODE1_AI;	// 0 Register Auto-Increment disabled. {default}

	if (bMode) {
		Data |= PCA9685_MODE1_AI;	// 1 Register Auto-Increment enabled.
	}

	I2cWriteReg(PCA9685_REG_MODE1, Data);
}

void PCA9685::I2cSetup(void) {
	bcm2835_i2c_setSlaveAddress(m_nAddress);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
}

void PCA9685::I2cWriteReg(uint8_t reg, uint8_t data) {
	uint8_t buffer[2];

	buffer[0] = reg;
	buffer[1] = data;

	I2cSetup();

	bcm2835_i2c_write((char *)buffer, 2);
}

uint8_t PCA9685::I2cReadReg(uint8_t reg) {
	char data = reg;

	I2cSetup();

	(void) bcm2835_i2c_write((char *)&data, 1);
	(void) bcm2835_i2c_read((char *)&data, 1);

	return data;
}

void PCA9685::I2cWriteReg(uint8_t reg, uint16_t data) {
	uint8_t buffer[3];

	buffer[0] = reg;
	buffer[1] = (uint8_t) (data & 0xFF);
	buffer[2] = (uint8_t) (data >> 8);

	I2cSetup();

	bcm2835_i2c_write((char *) buffer, 3);
}

uint16_t PCA9685::I2cReadReg16(uint8_t reg) {
	char data = reg;
	char buffer[2] = { 0, 0 };

	I2cSetup();

	(void) bcm2835_i2c_write((char *)&data, 1);
	(void) bcm2835_i2c_read((char *)&buffer, 2);

	return (uint16_t) ((uint16_t) buffer[1] << 8 | (uint16_t) buffer[0]);
}

void PCA9685::I2cWriteReg(uint8_t reg, uint16_t data, uint16_t data2) {
	uint8_t buffer[5];

	buffer[0] = reg;
	buffer[1] = (uint8_t) (data & 0xFF);
	buffer[2] = (uint8_t) (data >> 8);
	buffer[3] = (uint8_t) (data2 & 0xFF);
	buffer[4] = (uint8_t) (data2 >> 8);

	I2cSetup();

	bcm2835_i2c_write((char *) buffer, 5);
}

