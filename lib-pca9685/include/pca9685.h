/**
 * @file pca9685.h
 *
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

#ifndef PCA9685_H_
#define PCA9685_H_

#include <cstdint>

namespace pca9685 {
static constexpr uint8_t I2C_ADDRESS_DEFAULT = 0x40;
static constexpr uint8_t I2C_ADDRESS_FIXED = 0x70;
static constexpr uint8_t I2C_ADDRESSES_MAX = 62;
static constexpr uint32_t PWM_CHANNELS = 16;

enum class Output {
	DRIVER_OPENDRAIN,
	DRIVER_TOTEMPOLE
};

enum class Invert {
	OUTPUT_NOT_INVERTED,
	OUTPUT_INVERTED
};

enum class Och {
	PCA9685_OCH_STOP,
	PCA9685_OCH_ACK
};

struct Frequency {
	static constexpr uint32_t RANGE_MIN = 24;
	static constexpr uint32_t RANGE_MAX = 1526;
};
}  // namespace pca9685

#define CHANNEL(x)	(static_cast<uint8_t>(x))
#define VALUE(x)	(static_cast<uint16_t>(x))

#define PCA9685_VALUE_MIN	VALUE(0)
#define PCA9685_VALUE_MAX	VALUE(4096)



class PCA9685 {
public:
	PCA9685(const uint8_t nAddress = pca9685::I2C_ADDRESS_DEFAULT);
	~PCA9685() {};

	void SetPreScaller(uint8_t);
	uint8_t GetPreScaller();

	void SetFrequency(uint16_t);
	uint16_t GetFrequency();

	void SetOCH(pca9685::Och);
	pca9685::Och GetOCH();

	void SetInvert(const pca9685::Invert invert);
	pca9685::Invert GetInvert();

	void SetOutDriver(const pca9685::Output output);
	pca9685::Output GetOutDriver();

	void Read(const uint32_t nChannel, uint16_t *pOn, uint16_t *pOff);
	void Read(uint16_t *pOn, uint16_t *pOff);

	void Write(const uint32_t nChannel, const uint16_t nOn, const uint16_t nOff);
	void Write(const uint32_t nChannel, const uint16_t nValue);
	void Write(const uint16_t nOn, const uint16_t nOff);
	void Write(const uint16_t nValue);

	void SetFullOn(const uint32_t nChannel, const bool bMode);
	void SetFullOff(const uint32_t nChannel, const bool bMode);

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
	uint8_t m_nAddress;
};

#endif /* PCA9685_H_ */
