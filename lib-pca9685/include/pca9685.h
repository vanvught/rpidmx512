/**
 * @file pca9685.h
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

#ifndef PCA9685_H_
#define PCA9685_H_

#include <stdint.h>
#include <stdbool.h>

#define PCA9685_I2C_ADDRESS_DEFAULT	0x40
#define PCA9685_I2C_ADDRESS_FIXED	0x70
#define PCA9685_I2C_ADDRESSES_MAX	62

#define CHANNEL(x)	((uint8_t)(x))
#define VALUE(x)	((uint16_t)(x))

#define PCA9685_VALUE_MIN	VALUE(0)
#define PCA9685_VALUE_MAX	VALUE(4096)

#define PCA9685_PWM_CHANNELS	16

enum TPCA9685FrequencyRange {
	PCA9685_FREQUENCY_MIN = 24,
	PCA9685_FREQUENCY_MAX = 1526
};

enum TPCA9685Och {
	PCA9685_OCH_STOP = 0,
	PCA9685_OCH_ACK = 1 << 3
};

class PCA9685 {
public:
	PCA9685(uint8_t nAddress = PCA9685_I2C_ADDRESS_DEFAULT);
	~PCA9685(void);

	void SetPreScaller(uint8_t);
	uint8_t GetPreScaller(void);

	void SetFrequency(uint16_t);
	uint16_t GetFrequency(void);

	void SetOCH(TPCA9685Och);
	TPCA9685Och GetOCH(void);

	void SetInvert(bool);
	bool GetInvert(void);

	void SetOutDriver(bool);
	bool GetOutDriver(void);

	void Write(uint8_t, uint16_t, uint16_t);
	void Read(uint8_t, uint16_t *, uint16_t *);

	void Write(uint16_t, uint16_t);
	void Read(uint16_t *, uint16_t *);

	void Write(uint8_t, uint16_t);
	void Write(uint16_t);

	void SetFullOn(uint8_t, bool);
	void SetFullOff(uint8_t, bool);

	void Dump(void);

private:
	uint8_t CalcPresScale(uint16_t);
	uint16_t CalcFrequency(uint8_t);

private:
	void Sleep(bool);
	void AutoIncrement(bool);

private:
	void I2cSetup(void);

	void I2cWriteReg(uint8_t, uint8_t);
	uint8_t I2cReadReg(uint8_t);

	void I2cWriteReg(uint8_t, uint16_t);
	uint16_t I2cReadReg16(uint8_t);

	void I2cWriteReg(uint8_t, uint16_t, uint16_t);

private:
	uint8_t m_nAddress;
};

#endif /* PCA9685_H_ */
