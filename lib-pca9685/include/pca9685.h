/**
 * @file pca9685.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PCA9685_H_
#define PCA9685_H_

#include <cstdint>

namespace pca9685
{
static constexpr uint8_t I2C_ADDRESS_DEFAULT = 0x40;
static constexpr uint8_t I2C_ADDRESS_FIXED = 0x70;
static constexpr uint8_t I2C_ADDRESSES_MAX = 62;
static constexpr uint32_t PWM_CHANNELS = 16;

enum class Output
{
    kDriverOpendrain,
    kDriverTotempole
};

enum class Invert
{
    kOutputNotInverted,
    kOutputInverted
};

enum class Och
{
    PCA9685_OCH_STOP,
    PCA9685_OCH_ACK
};

struct Frequency
{
    static constexpr uint32_t RANGE_MIN = 24;
    static constexpr uint32_t RANGE_MAX = 1526;
};
} // namespace pca9685

#define CHANNEL(x) (static_cast<uint32_t>(x))
#define VALUE(x) (static_cast<uint16_t>(x))

#define PCA9685_VALUE_MIN VALUE(0)
#define PCA9685_VALUE_MAX VALUE(4096)

class PCA9685
{
   public:
    explicit PCA9685(uint8_t address = pca9685::I2C_ADDRESS_DEFAULT);
    ~PCA9685() {};

    void SetPreScaller(uint8_t);
    uint8_t GetPreScaller();

    void SetFrequency(uint16_t);
    uint16_t GetFrequency();

    void SetOCH(pca9685::Och);
    pca9685::Och GetOCH();

    void SetInvert(pca9685::Invert invert);
    pca9685::Invert GetInvert();

    void SetOutDriver(pca9685::Output output);
    pca9685::Output GetOutDriver();

    void Read(uint32_t channel, uint16_t* on, uint16_t* off);
    void Read(uint16_t* on, uint16_t* off);

    void Write(uint32_t channel, uint16_t on, uint16_t off);
    void Write(uint32_t channel, uint16_t value);
    void Write(uint16_t on, uint16_t off);
    void Write(uint16_t value);

    void SetFullOn(uint32_t channel, bool mode);
    void SetFullOff(uint32_t channel, bool mode);

    void Dump();

   private:
    uint8_t CalcPresScale(uint32_t);
    uint16_t CalcFrequency(uint32_t);

   private:
    void Sleep(bool);
    void AutoIncrement(bool);

   private:
    void I2cSetup();

    void I2cWriteReg(uint8_t, uint8_t);
    uint8_t I2cReadReg(uint8_t);

    void I2cWriteReg(uint8_t, uint16_t);
    uint16_t I2cReadReg16(uint8_t);

    void I2cWriteReg(uint8_t, uint16_t, uint16_t);

   private:
    uint8_t address_;
};

#endif  // PCA9685_H_
