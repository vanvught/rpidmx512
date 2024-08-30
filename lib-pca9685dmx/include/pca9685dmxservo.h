/**
 * @file pca9685dmxservo.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PWMDMXPCA9685SERVO_H_
#define PWMDMXPCA9685SERVO_H_

#include <cstdint>

#include "lightset.h"

#include "pca9685dmx.h"
#include "pca9685dmxstore.h"
#include "pca9685servo.h"

class PCA9685DmxServo final: public LightSet {
public:
	PCA9685DmxServo(const pca9685dmx::Configuration &configuration);
	~PCA9685DmxServo() override;

	void Start([[maybe_unused]] const uint32_t nPortIndex = 0) override {};
	void Stop([[maybe_unused]] const uint32_t nPortIndex = 0) override {};

	void SetData(const uint32_t nPortIndex, const uint8_t *pDmxData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync([[maybe_unused]] const uint32_t nPortIndex) override {};
	void Sync() override {};

	bool SetDmxStartAddress(const uint16_t nDmxStartAddress) override {
		assert((nDmxStartAddress != 0) && (nDmxStartAddress <= lightset::dmx::UNIVERSE_SIZE));

		if ((nDmxStartAddress != 0) && (nDmxStartAddress <= lightset::dmx::UNIVERSE_SIZE)) {
			m_nDmxStartAddress = nDmxStartAddress;
			PCA9685DmxStore::SaveDmxStartAddress(m_nDmxStartAddress);
			return true;
		}

		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint() override {
		return m_nDmxFootprint;
	}

	void Print() override;

private:
	uint16_t m_nBoardInstances;
	uint16_t m_nDmxFootprint;
	uint16_t m_nDmxStartAddress;
	uint16_t m_nChannelCount;
	bool m_bUse8Bit;
	uint8_t m_DmxData[lightset::dmx::UNIVERSE_SIZE];
	PCA9685Servo **m_pServo;
};

#endif /* PWMDMXPCA9685SERVO_H_ */
