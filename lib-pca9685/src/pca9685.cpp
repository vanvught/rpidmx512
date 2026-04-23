/**
 * @file pca9685.cpp
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "pca9685.h"

#define DIV_ROUND_UP(n, d) (((n) + static_cast<float>(d) - 1) / static_cast<float>(d))

#define PCA9685_OSC_FREQ 25000000L

enum TPCA9685Reg {
    kPcA9685RegModE1 = 0x00,
    kPcA9685RegModE2 = 0x01,
    kPcA9685RegAllcalladr = 0x05,
    kPcA9685RegLeD0OnL = 0x06,
    kPcA9685RegLeD0OnH = 0x07,
    kPcA9685RegLeD0OffL = 0x08,
    kPcA9685RegLeD0OffH = 0x09,
    kPcA9685RegAllLedOnL = 0xFA,
    kPcA9685RegAllLedOnH = 0xFB,
    kPcA9685RegAllLedOffL = 0xFC,
    kPcA9685RegAllLedOffH = 0xFD,
    kPcA9685RegPreScale = 0xFE
};

#define PCA9685_PRE_SCALE_MIN 0x03
#define PCA9685_PRE_SCALE_MAX 0xFF

/*
 * 7.3.1 Mode register 1, MODE1
 */
enum TPCA9685Mode1 {
    kPcA9685ModE1Allcall = 1U << 0,
    kPcA9685ModE1SuB3 = 1U << 1,
    kPcA9685ModE1SuB2 = 1U << 2,
    kPcA9685ModE1SuB1 = 1U << 3,
    kPcA9685ModE1Sleep = 1U << 4,
    kPcA9685ModE1Ai = 1U << 5,
    kPcA9685ModE1Extclk = 1U << 6,
    kPcA9685ModE1Restart = 1U << 7
};

/*
 * 7.3.2 Mode register 2, MODE2
 */
enum TPCA9685Mode2 { 
	kPcA9685ModE2Outdrv = 1U << 2, 
	kPcA9685ModE2Och = 1U << 3, 
	kPcA9685ModE2Invrt = 1U << 4 
};

enum TPCA9685Och { 
	kPcA9685OchStop = 0, 
	kPcA9685OchAck = 1U << 3
};

PCA9685::PCA9685(uint8_t address) : address_(address) {
    FUNC_PREFIX(I2cBegin());

    AutoIncrement(true);

    for (uint32_t i = 0; i < 16; i++) {
        Write(static_cast<uint8_t>(i), static_cast<uint16_t>(0), static_cast<uint16_t>(0x1000));
    }

    Sleep(false);
}

void PCA9685::Sleep(bool mode) {
    auto data = I2cReadReg(kPcA9685RegModE1);

    data &= static_cast<uint8_t>(~kPcA9685ModE1Sleep);

    if (mode) {
        data |= kPcA9685ModE1Sleep;
    }

    I2cWriteReg(kPcA9685RegModE1, data);

    //	if (data & ~PCA9685_MODE1_RESTART) {
    //		udelay(500);
    //		data |= PCA9685_MODE1_RESTART;
    //	}
}

void PCA9685::SetPreScaller(uint8_t prescale) {
    prescale = prescale < PCA9685_PRE_SCALE_MIN ? PCA9685_PRE_SCALE_MIN : prescale;

    Sleep(true);
    I2cWriteReg(kPcA9685RegPreScale, prescale);
    Sleep(false);
}

uint8_t PCA9685::GetPreScaller() {
    return I2cReadReg(kPcA9685RegPreScale);
}

void PCA9685::SetFrequency(uint16_t freq) {
    SetPreScaller(CalcPresScale(freq));
}

uint16_t PCA9685::GetFrequency() {
    return CalcFrequency(GetPreScaller());
}

void PCA9685::SetOCH(pca9685::Och och) {
    auto data = I2cReadReg(kPcA9685RegModE2);

    data &= static_cast<uint8_t>(~kPcA9685ModE2Och);

    if (och == pca9685::Och::kOchAck) {
        data |= kPcA9685OchAck;
    } // else, default Outputs change on STOP command

    I2cWriteReg(kPcA9685RegModE2, data);
}

pca9685::Och PCA9685::GetOCH() {
    const auto kIsOchAck = (I2cReadReg(kPcA9685RegModE2) & kPcA9685ModE2Och) == kPcA9685ModE2Och;
    return kIsOchAck ? pca9685::Och::kOchAck : pca9685::Och::kOchStop;
}

void PCA9685::SetInvert(pca9685::Invert invert) {
    auto data = I2cReadReg(kPcA9685RegModE2);
    data &= static_cast<uint8_t>(~kPcA9685ModE2Invrt);

    if (invert == pca9685::Invert::kOutputInverted) {
        data |= kPcA9685ModE2Invrt;
    }

    I2cWriteReg(kPcA9685RegModE2, data);
}

pca9685::Invert PCA9685::GetInvert() {
    const auto kData = I2cReadReg(kPcA9685RegModE2) & kPcA9685ModE2Invrt;
    return (kData == kPcA9685ModE2Invrt) ? pca9685::Invert::kOutputInverted : pca9685::Invert::kOutputNotInverted;
}

void PCA9685::SetOutDriver(pca9685::Output output) {
    auto data = I2cReadReg(kPcA9685RegModE2);
    data &= static_cast<uint8_t>(~kPcA9685ModE2Outdrv);

    if (output == pca9685::Output::kDriverTotempole) {
        data |= kPcA9685ModE2Outdrv;
    }

    I2cWriteReg(kPcA9685RegModE2, data);
}

pca9685::Output PCA9685::GetOutDriver() {
    const auto kData = I2cReadReg(kPcA9685RegModE2) & kPcA9685ModE2Outdrv;
    return (kData == kPcA9685ModE2Outdrv) ? pca9685::Output::kDriverTotempole : pca9685::Output::kDriverOpendrain;
}

void PCA9685::Write(uint32_t channel, uint16_t on, uint16_t off) {
    uint8_t reg;

    if (channel <= 15) {
        reg = static_cast<uint8_t>(kPcA9685RegLeD0OnL + (channel << 2));
    } else {
        reg = kPcA9685RegAllLedOnL;
    }

    I2cWriteReg(reg, on, off);
}

void PCA9685::Write(uint32_t channel, uint16_t value) {
    Write(channel, static_cast<uint16_t>(0), value);
}

void PCA9685::Write(uint16_t on, uint16_t off) {
    Write(static_cast<uint32_t>(16), on, off);
}

void PCA9685::Write(uint16_t value) {
    Write(static_cast<uint32_t>(16), value);
}

void PCA9685::Read(uint32_t channel, uint16_t* on, uint16_t* off) {
    assert(pOn != nullptr);
    assert(pOff != nullptr);

    uint8_t reg;

    if (channel <= 15) {
        reg = static_cast<uint8_t>(kPcA9685RegLeD0OnL + (channel << 2));
    } else {
        reg = kPcA9685RegAllLedOnL;
    }

    if (on != nullptr) {
        *on = I2cReadReg16(reg);
    }

    if (off) {
        *off = I2cReadReg16(static_cast<uint8_t>(reg + 2));
    }
}

void PCA9685::Read(uint16_t* on, uint16_t* off) {
    Read(static_cast<uint8_t>(16), on, off);
}

void PCA9685::SetFullOn(uint32_t channel, bool mode) {
    uint8_t reg;

    if (channel <= 15) {
        reg = static_cast<uint8_t>(kPcA9685RegLeD0OnH + (channel << 2));
    } else {
        reg = kPcA9685RegAllLedOnH;
    }

    uint8_t data = I2cReadReg(reg);

    data = mode ? (data | 0x10) : (data & 0xEF);

    I2cWriteReg(reg, data);

    if (mode) {
        SetFullOff(channel, false);
    }
}

void PCA9685::SetFullOff(uint32_t channel, bool mode) {
    uint8_t reg;

    if (channel <= 15) {
        reg = static_cast<uint8_t>(kPcA9685RegLeD0OffH + (channel << 2));
    } else {
        reg = kPcA9685RegAllLedOffH;
    }

    uint8_t data = I2cReadReg(reg);

    data = mode ? (data | 0x10) : (data & 0xEF);

    I2cWriteReg(reg, data);
}

uint8_t PCA9685::CalcPresScale(uint32_t frequency) {
    frequency = (frequency > pca9685::Frequency::kRangeMax ? pca9685::Frequency::kRangeMax : (frequency < pca9685::Frequency::kRangeMin ? pca9685::Frequency::kRangeMin : frequency));

    constexpr auto kF = static_cast<float>(PCA9685_OSC_FREQ) / 4096U;
    const auto kData = DIV_ROUND_UP(kF, frequency) - 1U;

    return static_cast<uint8_t>(kData);
}

uint16_t PCA9685::CalcFrequency(uint32_t pre_scale) {
    constexpr auto kF = static_cast<float>(PCA9685_OSC_FREQ) / 4096U;
    const auto kData = static_cast<uint32_t>(DIV_ROUND_UP(kF, (pre_scale) + 1U));

    uint32_t frequency_min;

    for (frequency_min = kData; frequency_min > pca9685::Frequency::kRangeMin; frequency_min--) {
        if (CalcPresScale(frequency_min) != pre_scale) {
            break;
        }
    }

    uint32_t frequency_max;

    for (frequency_max = kData; frequency_max < pca9685::Frequency::kRangeMax; frequency_max++) {
        if (CalcPresScale(frequency_max) != pre_scale) {
            break;
        }
    }

    return static_cast<uint16_t>(frequency_max + frequency_min) / 2;
}

void PCA9685::Dump() {
#ifndef NDEBUG
    uint8_t reg = I2cReadReg(kPcA9685RegModE1);

    printf("MODE1 - Mode register 1 (address 00h) : %02Xh\n", reg);
    printf("\tbit 7 - RESTART : Restart %s\n", reg & kPcA9685ModE1Restart ? "enabled" : "disabled");
    printf("\tbit 6 - EXTCLK  : %s\n", reg & kPcA9685ModE1Extclk ? "Use EXTCLK pin clock" : "Use internal clock");
    printf("\tbit 5 - AI      : Register Auto-Increment %s\n", reg & kPcA9685ModE1Ai ? "enabled" : "disabled");
    printf("\tbit 4 - SLEEP   : %s\n", reg & kPcA9685ModE1Sleep ? "Low power mode. Oscillator off" : "Normal mode");
    printf("\tbit 3 - SUB1    : PCA9685 %s to I2C-bus subaddress 1\n", reg & kPcA9685ModE1SuB1 ? "responds" : "does not respond");
    printf("\tbit 2 - SUB1    : PCA9685 %s to I2C-bus subaddress 2\n", reg & kPcA9685ModE1SuB2 ? "responds" : "does not respond");
    printf("\tbit 1 - SUB1    : PCA9685 %s to I2C-bus subaddress 3\n", reg & kPcA9685ModE1SuB3 ? "responds" : "does not respond");
    printf("\tbit 0 - ALLCALL : PCA9685 %s to LED All Call I2C-bus address\n", reg & kPcA9685ModE1Allcall ? "responds" : "does not respond");

    reg = I2cReadReg(kPcA9685RegModE2);

    printf("\nMODE2 - Mode register 2 (address 01h) : %02Xh\n", reg);
    printf("\tbit 7 to 5      : Reserved\n");
    printf("\tbit 4 - INVRT   : Output logic state %sinverted\n", reg & kPcA9685ModE2Invrt ? "" : "not ");
    printf("\tbit 3 - OCH     : Outputs change on %s\n", reg & kPcA9685ModE2Och ? "ACK" : "STOP command");
    printf("\tbit 2 - OUTDRV  : The 16 LEDn outputs are configured with %s structure\n", reg & kPcA9685ModE2Outdrv ? "a totem pole" : "an open-drain");
    printf("\tbit 10- OUTNE   : %01x\n", reg & 0x3);

    reg = I2cReadReg(kPcA9685RegPreScale);

    printf("\nPRE_SCALE register (address FEh) : %02Xh\n", reg);
    printf("\t Frequency : %d Hz\n", CalcFrequency(reg));

    puts("");

    uint16_t on, off;

    for (uint32_t led = 0; led <= 15; led++) {
        Read(led, &on, &off);
        printf("LED%d_ON  : %04x\n", led, on);
        printf("LED%d_OFF : %04x\n", led, off);
    }

    puts("");

    Read(16, &on, &off);
    printf("ALL_LED_ON  : %04x\n", on);
    printf("ALL_LED_OFF : %04x\n", off);
#endif
}

void PCA9685::AutoIncrement(bool mode) {
    auto data = I2cReadReg(kPcA9685RegModE1);

    data &= static_cast<uint8_t>(~kPcA9685ModE1Ai); // 0 Register Auto-Increment disabled. {default}

    if (mode) {
        data |= kPcA9685ModE1Ai; // 1 Register Auto-Increment enabled.
    }

    I2cWriteReg(kPcA9685RegModE1, data);
}

void PCA9685::I2cSetup() {
    FUNC_PREFIX(I2cSetAddress(address_));
    FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
}

void PCA9685::I2cWriteReg(uint8_t reg, uint8_t data) {
    char buffer[2];

    buffer[0] = reg;
    buffer[1] = data;

    I2cSetup();

    FUNC_PREFIX(I2cWrite(buffer, 2));
}

uint8_t PCA9685::I2cReadReg(uint8_t reg) {
    char data = reg;

    I2cSetup();

    FUNC_PREFIX(I2cWrite(&data, 1));
    FUNC_PREFIX(I2cRead(&data, 1));

    return data;
}

void PCA9685::I2cWriteReg(uint8_t reg, uint16_t data) {
    char buffer[3];

    buffer[0] = reg;
    buffer[1] = static_cast<char>(data & 0xFF);
    buffer[2] = static_cast<char>(data >> 8);

    I2cSetup();

    FUNC_PREFIX(I2cWrite(buffer, 3));
}

uint16_t PCA9685::I2cReadReg16(uint8_t reg) {
    char data = reg;
    char buffer[2] = {0, 0};

    I2cSetup();

    FUNC_PREFIX(I2cWrite(&data, 1));
    FUNC_PREFIX(I2cRead(reinterpret_cast<char*>(&buffer), 2));

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

    FUNC_PREFIX(I2cWrite(buffer, 5));
}
