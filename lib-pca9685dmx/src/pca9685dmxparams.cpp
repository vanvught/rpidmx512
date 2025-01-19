/**
 * @file pca9685dmxparams.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cassert>

#include "pca9685dmxparams.h"
#include "pca9685dmxparamsconst.h"
#include "pca9685pwmled.h"
#include "pca9685servo.h"
#include "pca9685.h"
#include "pca9685dmx.h"

#include "lightset.h"
#include "lightsetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

namespace pca9685dmxparams {
static constexpr char MODE_LED[] = "led";
static constexpr char MODE_SERVO[] = "servo";

static const char *get_mode(const uint32_t nMode) {
	return nMode != 0 ?  MODE_SERVO : MODE_LED;
}

static uint32_t get_mode(const char *pMode) {
	if (strcasecmp(pMode, MODE_SERVO) == 0) {
		return 1;
	}

	return 0;
}
}  // namespace pca9685dmxparams

PCA9685DmxParams::PCA9685DmxParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;
	m_Params.nAddress = pca9685::I2C_ADDRESS_DEFAULT;
	m_Params.nChannelCount = pca9685::PWM_CHANNELS;
	m_Params.nDmxStartAddress = lightset::dmx::START_ADDRESS_DEFAULT;
	m_Params.nLedPwmFrequency = pca9685::pwmled::DEFAULT_FREQUENCY;
	m_Params.nServoLeftUs = pca9685::servo::LEFT_DEFAULT_US;
	m_Params.nServoCenterUs = pca9685::servo::CENTER_DEFAULT_US;
	m_Params.nServoRightUs = pca9685::servo::RIGHT_DEFAULT_US;

	DEBUG_EXIT
}

void PCA9685DmxParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(PCA9685DmxParams::StaticCallbackFunction, this);

	if (configfile.Read(PCA9685DmxParamsConst::FILE_NAME)) {
		PCA9685DmxParamsStore::Update(&m_Params);
	} else
#endif
		PCA9685DmxParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void PCA9685DmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(PCA9685DmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	PCA9685DmxParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void PCA9685DmxParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_Params.nSetList |= nMask;
	} else {
		m_Params.nSetList &= ~nMask;
	}
}

void PCA9685DmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::I2cAddress(pLine, PCA9685DmxParamsConst::I2C_ADDRESS, nValue8) == Sscan::OK) {
		if ((nValue8 < 0x7f) || (nValue8 != pca9685::I2C_ADDRESS_DEFAULT)) {
			m_Params.nAddress = nValue8;
			m_Params.nSetList |= pca9685dmxparams::Mask::ADDRESS;
		} else {
			m_Params.nDmxStartAddress = pca9685::I2C_ADDRESS_DEFAULT;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::ADDRESS;
		}
		return;
	}

	char aBuffer[8];
	uint32_t nLength = sizeof(pca9685dmxparams::MODE_SERVO) - 1;
	assert(nLength < sizeof(aBuffer));

	if (Sscan::Char(pLine, PCA9685DmxParamsConst::MODE, aBuffer, nLength) == Sscan::OK) {
		aBuffer[nLength] = '\0';
		SetBool(static_cast<uint8_t>(pca9685dmxparams::get_mode(aBuffer)), pca9685dmxparams::Mask::MODE);
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::CHANNEL_COUNT, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 != pca9685::PWM_CHANNELS) && (nValue16 <= lightset::dmx::UNIVERSE_SIZE)) {
			m_Params.nChannelCount = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::CHANNEL_COUNT;
		} else {
			m_Params.nChannelCount = pca9685::PWM_CHANNELS;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::CHANNEL_COUNT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LightSetParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && nValue16 <= (lightset::dmx::UNIVERSE_SIZE) && (nValue16 != lightset::dmx::START_ADDRESS_DEFAULT)) {
			m_Params.nDmxStartAddress = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::DMX_START_ADDRESS;
		} else {
			m_Params.nDmxStartAddress = lightset::dmx::START_ADDRESS_DEFAULT;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::DMX_START_ADDRESS;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PCA9685DmxParamsConst::USE_8BIT, nValue8) == Sscan::OK) {
		SetBool(nValue8, pca9685dmxparams::Mask::USE_8BIT);
		return;
	}


	/*
	 * LED specific
	 */

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::LED_PWM_FREQUENCY, nValue16) == Sscan::OK) {
		if ((nValue16 >= pca9685::Frequency::RANGE_MIN) && (nValue16 != pca9685::pwmled::DEFAULT_FREQUENCY)  && (nValue16 <= pca9685::Frequency::RANGE_MAX)) {
			m_Params.nLedPwmFrequency = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::LED_PWM_FREQUENCY;
		} else {
			m_Params.nLedPwmFrequency = pca9685::pwmled::DEFAULT_FREQUENCY;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::LED_PWM_FREQUENCY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN, nValue8) == Sscan::OK) {
		SetBool(nValue8, pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN);
		return;
	}

	if (Sscan::Uint8(pLine, PCA9685DmxParamsConst::LED_OUTPUT_INVERT, nValue8) == Sscan::OK) {
		SetBool(nValue8, pca9685dmxparams::Mask::LED_OUTPUT_INVERT);
		return;
	}

	/*
	 * Servo specific
	 */

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::SERVO_LEFT_US, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 != pca9685::servo::LEFT_DEFAULT_US)) {
			m_Params.nServoLeftUs = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::SERVO_LEFT_US;
		} else {
			m_Params.nServoLeftUs = pca9685::servo::LEFT_DEFAULT_US;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::SERVO_LEFT_US;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::SERVO_CENTER_US, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 != pca9685::servo::CENTER_DEFAULT_US)) {
			m_Params.nServoRightUs = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::SERVO_CENTER_US;
		} else {
			m_Params.nServoRightUs = pca9685::servo::CENTER_DEFAULT_US;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::SERVO_CENTER_US;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::SERVO_RIGHT_US, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 != pca9685::servo::RIGHT_DEFAULT_US)) {
			m_Params.nServoRightUs = nValue16;
			m_Params.nSetList |= pca9685dmxparams::Mask::SERVO_RIGHT_US;
		} else {
			m_Params.nServoRightUs = pca9685::servo::RIGHT_DEFAULT_US;
			m_Params.nSetList &= ~pca9685dmxparams::Mask::SERVO_RIGHT_US;
		}
		return;
	}
}

void PCA9685DmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<PCA9685DmxParams*>(p))->callbackFunction(s);
}

void PCA9685DmxParams::Builder(const struct pca9685dmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct pca9685dmxparams::Params));
	} else {
		PCA9685DmxParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(PCA9685DmxParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(PCA9685DmxParamsConst::MODE, pca9685dmxparams::get_mode(isMaskSet(pca9685dmxparams::Mask::MODE)), isMaskSet(pca9685dmxparams::Mask::MODE));
	builder.Add(PCA9685DmxParamsConst::CHANNEL_COUNT, m_Params.nChannelCount, isMaskSet(pca9685dmxparams::Mask::CHANNEL_COUNT));
	builder.Add(LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress, isMaskSet(pca9685dmxparams::Mask::DMX_START_ADDRESS));
	builder.Add(PCA9685DmxParamsConst::USE_8BIT, isMaskSet(pca9685dmxparams::Mask::USE_8BIT), isMaskSet(pca9685dmxparams::Mask::USE_8BIT));

	builder.AddComment("mode=led");
	builder.Add(PCA9685DmxParamsConst::LED_PWM_FREQUENCY, m_Params.nLedPwmFrequency, isMaskSet(pca9685dmxparams::Mask::LED_PWM_FREQUENCY));
	builder.Add(PCA9685DmxParamsConst::LED_OUTPUT_INVERT, isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT), isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT));
	builder.Add(PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN, isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN), isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN));

	builder.AddComment("mode=servo");
	builder.Add(PCA9685DmxParamsConst::SERVO_LEFT_US, m_Params.nServoLeftUs, isMaskSet(pca9685dmxparams::Mask::SERVO_LEFT_US));
	builder.Add(PCA9685DmxParamsConst::SERVO_CENTER_US, m_Params.nServoCenterUs, isMaskSet(pca9685dmxparams::Mask::SERVO_CENTER_US));
	builder.Add(PCA9685DmxParamsConst::SERVO_RIGHT_US, m_Params.nServoRightUs, isMaskSet(pca9685dmxparams::Mask::SERVO_RIGHT_US));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void PCA9685DmxParams::Set(PCA9685Dmx *pPCA9685Dmx) {
	DEBUG_ENTRY
	assert(pPCA9685Dmx != nullptr);

	/*
	 * Generic
	 */

	pPCA9685Dmx->SetAddress(m_Params.nAddress);
	pPCA9685Dmx->SetMode(isMaskSet(pca9685dmxparams::Mask::MODE));
	pPCA9685Dmx->SetChannelCount(m_Params.nChannelCount);
	pPCA9685Dmx->SetDmxStartAddress(m_Params.nDmxStartAddress);
	pPCA9685Dmx->SetUse8Bit(isMaskSet(pca9685dmxparams::Mask::USE_8BIT));

	/*
	 * LED specific
	 */

	pPCA9685Dmx->SetLedPwmFrequency(m_Params.nLedPwmFrequency);
	pPCA9685Dmx->SetLedOutputInvert(isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT) ? pca9685::Invert::OUTPUT_INVERTED : pca9685::Invert::OUTPUT_NOT_INVERTED);
	pPCA9685Dmx->SetLedOutputDriver(isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN) ? pca9685::Output::DRIVER_OPENDRAIN : pca9685::Output::DRIVER_TOTEMPOLE);

	/*
	 * Servo specific
	 */

	pPCA9685Dmx->SetServoLeftUs(m_Params.nServoLeftUs);
	pPCA9685Dmx->SetServoCenterUs(m_Params.nServoCenterUs);
	pPCA9685Dmx->SetServoRightUs(m_Params.nServoRightUs);

	DEBUG_EXIT
}

void PCA9685DmxParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PCA9685DmxParamsConst::FILE_NAME);

	printf(" %s=0x%.2x\n", PCA9685DmxParamsConst::I2C_ADDRESS, m_Params.nAddress);

	const auto IsModeSet = isMaskSet(pca9685dmxparams::Mask::MODE);
	printf(" %s=%s [%d]\n", PCA9685DmxParamsConst::MODE, pca9685dmxparams::get_mode(IsModeSet), IsModeSet);

	printf(" %s=%d\n", PCA9685DmxParamsConst::CHANNEL_COUNT, m_Params.nChannelCount);
	printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);

	/*
	 * LED specific
	 */

	printf(" %s=%d Hz\n", PCA9685DmxParamsConst::LED_PWM_FREQUENCY, m_Params.nLedPwmFrequency);

	const auto IsOutputInvertedSet = isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT);
	printf(" %s=%d [Output logic state %sinverted]\n", PCA9685DmxParamsConst::LED_OUTPUT_INVERT, IsOutputInvertedSet, IsOutputInvertedSet ? "" : "not ");

	const auto IsOutputOpendrainSet = isMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN);
	printf(" %s=%d [The 16 LEDn outputs are configured with %s structure]\n", PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN, IsOutputOpendrainSet, IsOutputOpendrainSet ? "an open-drain" : "a totem pole");

	const auto IsUse8BitSet = isMaskSet(pca9685dmxparams::Mask::USE_8BIT);
	printf(" %s=%d [%d-bit]\n", PCA9685DmxParamsConst::USE_8BIT, IsUse8BitSet, IsUse8BitSet ? 8 : 16);

	/*
	 * Servo specific
	 */

	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_LEFT_US, m_Params.nServoLeftUs);
	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_CENTER_US, m_Params.nServoCenterUs);
	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_RIGHT_US, m_Params.nServoRightUs);
}
