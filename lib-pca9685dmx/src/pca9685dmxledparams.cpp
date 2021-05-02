/**
 * @file pca9685dmxledparams.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "pca9685dmxledparams.h"
#include "pca9685dmxled.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_PWM_FREQUENCY_MASK	(1 << 0)
#define SET_OUTPUT_INVERT_MASK	(1 << 1)
#define SET_OUTPUT_DRIVER_MASK	(1 << 2)
#define I2C_SLAVE_ADDRESS_MASK	(1 << 3)

constexpr char PARAMS_FILE_NAME[] = "pwmled.txt";
constexpr char PARAMS_I2C_SLAVE_ADDRESS[] = "i2c_slave_address";
constexpr char PARAMS_PWM_FREQUENCY[] = "pwm_frequency";
constexpr char PARAMS_OUTPUT_INVERT[] = "output_invert";
constexpr char PARAMS_OUTPUT_DRIVER[] = "output_driver";

PCA9685DmxLedParams::PCA9685DmxLedParams() :
	PCA9685DmxParams(PARAMS_FILE_NAME)
{
}

PCA9685DmxLedParams::~PCA9685DmxLedParams() {
}

bool PCA9685DmxLedParams::Load() {
	ReadConfigFile configfile(PCA9685DmxLedParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void PCA9685DmxLedParams::Set(PCA9685DmxLed* pDmxLed) {
	assert(pDmxLed != nullptr);

	bool isSet;

	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	if(isMaskSet(I2C_SLAVE_ADDRESS_MASK)) {
		pDmxLed->SetI2cAddress(m_nI2cAddress);
	}

	if(isMaskSet(SET_PWM_FREQUENCY_MASK)) {
		pDmxLed->SetPwmfrequency(m_nPwmFrequency);
	}

	if(isMaskSet(SET_OUTPUT_INVERT_MASK)) {
		pDmxLed->SetInvert(m_bOutputInvert);
	}

	if(isMaskSet(SET_OUTPUT_DRIVER_MASK)) {
		pDmxLed->SetOutDriver(m_bOutputDriver);
	}

	const uint16_t DmxStartAddress = GetDmxStartAddress(isSet);
	if (isSet) {
		pDmxLed->SetDmxStartAddress(DmxStartAddress);
	}

	const uint8_t BoardInstances = GetBoardInstances(isSet);
	if (isSet) {
		pDmxLed->SetBoardInstances(BoardInstances);
	}

	const char *p = GetDmxSlotInfoRaw(isSet);
	if (isSet) {
		pDmxLed->SetSlotInfoRaw(p);
	}

	// Footprint overwrites board instances!
	const uint16_t DmxFootprint = GetDmxFootprint(isSet);
	if (isSet) {
		pDmxLed->SetDmxFootprint(DmxFootprint);
	}
}

void PCA9685DmxLedParams::Dump() {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(I2C_SLAVE_ADDRESS_MASK)) {
		printf(" %s=0x%2x\n", PARAMS_I2C_SLAVE_ADDRESS, m_nI2cAddress);
	}

	if(isMaskSet(SET_PWM_FREQUENCY_MASK)) {
		printf(" %s=%d Hz\n", PARAMS_PWM_FREQUENCY, m_nPwmFrequency);
	}

	if(isMaskSet(SET_OUTPUT_INVERT_MASK)) {
		printf(" %s=%d [Output logic state %sinverted]\n", PARAMS_OUTPUT_INVERT, (int) m_bOutputInvert, m_bOutputInvert ? "" : "not ");
	}

	if(isMaskSet(SET_OUTPUT_DRIVER_MASK)) {
		printf(" %s=%d [The 16 LEDn outputs are configured with %s structure]\n", PARAMS_OUTPUT_DRIVER, (int) m_bOutputDriver, m_bOutputDriver ? "a totem pole" : "an open-drain");
	}

	PCA9685DmxParams::Dump();
#endif
}

void PCA9685DmxLedParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t value8;
	uint16_t value16;

	if (Sscan::I2cAddress(pLine, PARAMS_I2C_SLAVE_ADDRESS, value8) == Sscan::OK) {
		if ((value8 >= PCA9685_I2C_ADDRESS_DEFAULT) && (value8 != PCA9685_I2C_ADDRESS_FIXED)) {
			m_nI2cAddress = value8;
			m_bSetList |= I2C_SLAVE_ADDRESS_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_PWM_FREQUENCY, value16) == Sscan::OK) {
		if ((value16 >= TPCA9685FrequencyRange::MIN) && (value16 <= TPCA9685FrequencyRange::MAX)) {
			m_nPwmFrequency = value16;
			m_bSetList |= SET_PWM_FREQUENCY_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_OUTPUT_INVERT, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_bOutputInvert = true;
			m_bSetList |= SET_OUTPUT_INVERT_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_OUTPUT_DRIVER, value8) == Sscan::OK) {
		if (value8 == 0) {
			m_bOutputDriver = false;
			m_bSetList |= SET_OUTPUT_DRIVER_MASK;
		}
		return;
	}
}

void PCA9685DmxLedParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<PCA9685DmxLedParams*>(p))->callbackFunction(s);
}
