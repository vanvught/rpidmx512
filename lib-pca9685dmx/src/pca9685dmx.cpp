/**
 * @file pca9685dmx.cpp
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

#include <cstdint>
#include <cassert>

#include "pca9685dmx.h"
#include "pca9685dmxled.h"
#include "pca9685dmxservo.h"

#include "debug.h"

PCA9685Dmx *PCA9685Dmx::s_pThis;

PCA9685Dmx::PCA9685Dmx () {
	DEBUG_EXIT
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_Configuration.nMode = 0;
	m_Configuration.nAddress = pca9685::I2C_ADDRESS_DEFAULT;
	m_Configuration.nChannelCount = pca9685::PWM_CHANNELS;
	m_Configuration.nDmxStartAddress = lightset::dmx::START_ADDRESS_DEFAULT;

	m_Configuration.led.nLedPwmFrequency = pca9685::pwmled::DEFAULT_FREQUENCY;
	m_Configuration.led.invert = pca9685::Invert::OUTPUT_NOT_INVERTED;
	m_Configuration.led.output = pca9685::Output::DRIVER_TOTEMPOLE;

	m_Configuration.servo.nLeftUs = pca9685::servo::LEFT_DEFAULT_US;
	m_Configuration.servo.nRightUs = pca9685::servo::RIGHT_DEFAULT_US;

	DEBUG_EXIT
}

PCA9685Dmx::~PCA9685Dmx() {
	DEBUG_ENTRY

	if (m_pLightSet == nullptr) {
		DEBUG_EXIT
		return;
	}

	if (m_Configuration.nMode != 0) {	// Servo
		m_pLightSet->Stop(0);
		delete m_pLightSet;
		m_pLightSet = nullptr;
	} else {							// Led
		m_pLightSet->Stop(0);
		delete m_pLightSet;
		m_pLightSet = nullptr;
	}

	DEBUG_EXIT
}

void PCA9685Dmx::Start () {
	DEBUG_EXIT

	assert(m_pLightSet == nullptr);

	if (m_Configuration.nMode != 0) {	// Servo
		auto *pServo = new PCA9685DmxServo(m_Configuration);
		assert(pServo != nullptr);
		m_pLightSet = pServo;

		pServo->Start(0);
	} else {							// Led
		auto *pLed = new PCA9685DmxLed(m_Configuration);
		assert(pLed != nullptr);
		m_pLightSet = pLed;

		pLed->Start(0);
	}

	DEBUG_EXIT
}
