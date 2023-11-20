/**
 * @file pca9685dmx.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PCA9685DMX_H_
#define PCA9685DMX_H_

#include <cstdint>
#include <cassert>

#include "pca9685.h"
#include "pca9685pwmled.h"
#include "pca9685servo.h"

#include "lightset.h"

namespace pca9685dmx {
static constexpr uint8_t BOARD_INSTANCES_DEFAULT 	= 1;
static constexpr uint8_t BOARD_INSTANCES_MAX		= 32;
static constexpr auto DMX_FOOTPRINT_DEFAULT			= pca9685::PWM_CHANNELS;

struct Configuration {
	uint8_t nMode;
	uint8_t nAddress;
	uint16_t nChannelCount;
	uint16_t nDmxStartAddress;
	bool bUse8Bit;

	struct {
		uint16_t nLedPwmFrequency;
		pca9685::Invert invert;
		pca9685::Output output;
	} led;

	struct {
		uint16_t nLeftUs;
		uint16_t nRightUs;
	} servo;
};
}  // namespace pca9685

class PCA9685Dmx {
public:
	PCA9685Dmx();
	~PCA9685Dmx();

	void SetMode(const uint32_t nMode) {
		m_Configuration.nMode = (nMode != 0) ? 1 : 0;
	}

	void SetAddress(const uint8_t nAddress) {
		m_Configuration.nAddress = nAddress;
	}

	void SetChannelCount(const uint16_t nChannelCount) {
		m_Configuration.nChannelCount = nChannelCount;
	}

	void SetDmxStartAddress(const uint16_t nDmxStartAddress) {
		m_Configuration.nDmxStartAddress = nDmxStartAddress;
	}

	void SetUse8Bit(const bool bUse8Bit) {
		m_Configuration.bUse8Bit = bUse8Bit;
	}

	void SetLedPwmFrequency(const uint16_t nLedPwmFrequency) {
		m_Configuration.led.nLedPwmFrequency = nLedPwmFrequency;
	}

	void SetLedOutputInvert(const pca9685::Invert invert) {
		m_Configuration.led.invert = invert;
	}

	void SetLedOutputDriver(const pca9685::Output output) {
		m_Configuration.led.output = output;
	}

	void SetServoLeftUs(const uint16_t nLeftUs) {
		m_Configuration.servo.nLeftUs = nLeftUs;
	}

	void SetServoRightUs(const uint16_t nRightUs) {
		m_Configuration.servo.nRightUs = nRightUs;
	}

	LightSet *GetLightSet() {
		if (m_pLightSet == nullptr) {
			Start();
		}
		return m_pLightSet;
	}

	void Print() {
		if (m_pLightSet != nullptr) {
			m_pLightSet->Print();
		} else {
			assert(0);
		}
	}

	static PCA9685Dmx *Get() {
		return s_pThis;
	}

private:
	void Start();

private:
	pca9685dmx::Configuration m_Configuration;

	LightSet *m_pLightSet { nullptr };

	static PCA9685Dmx *s_pThis;
};

#endif /* PCA9685DMX_H_ */
