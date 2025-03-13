/**
 * @file pca9685dmxparams.cpp
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

#if defined (DEBUG_PCA9685DMX)
# undef NDEBUG
#endif

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

#include "dmxnodeparamsconst.h"

#include "configstore.h"
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
namespace store {
static void Update(const struct pca9685dmxparams::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
}

static void Copy(struct pca9685dmxparams::Params *pParams) {
	ConfigStore::Get()->Copy(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
}
}  // namespace store
}  // namespace pca9685dmxparams

PCA9685DmxParams::PCA9685DmxParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void PCA9685DmxParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(PCA9685DmxParams::StaticCallbackFunction, this);

	if (configfile.Read(PCA9685DmxParamsConst::FILE_NAME)) {
		pca9685dmxparams::store::Update(&m_Params);
	} else
#endif
		pca9685dmxparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void PCA9685DmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(m_Params));

	ReadConfigFile config(PCA9685DmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	pca9685dmxparams::store::Update(&m_Params);

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

void PCA9685DmxParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::I2cAddress(pLine, PCA9685DmxParamsConst::I2C_ADDRESS, nValue8) == Sscan::OK) {
		m_Params.nAddress = nValue8;
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
		if ((nValue16 != 0) && (nValue16 <= dmxnode::UNIVERSE_SIZE)) {
			m_Params.nChannelCount = nValue16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DmxNodeParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 <= dmxnode::UNIVERSE_SIZE)) {
			m_Params.nDmxStartAddress = nValue16;
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
		if ((nValue16 >= pca9685::Frequency::RANGE_MIN) && (nValue16 <= pca9685::Frequency::RANGE_MAX)) {
			m_Params.nLedPwmFrequency = nValue16;
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
		m_Params.nServoLeftUs = nValue16;
		return;
	}

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::SERVO_CENTER_US, nValue16) == Sscan::OK) {
		m_Params.nServoRightUs = nValue16;
		return;
	}

	if (Sscan::Uint16(pLine, PCA9685DmxParamsConst::SERVO_RIGHT_US, nValue16) == Sscan::OK) {
		m_Params.nServoRightUs = nValue16;
		return;
	}
}

void PCA9685DmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<PCA9685DmxParams*>(p))->CallbackFunction(s);
}

void PCA9685DmxParams::Builder(const struct pca9685dmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);

	auto& pca9685Dmx = PCA9685Dmx::Get();

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct pca9685dmxparams::Params));
	} else {
		pca9685dmxparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(PCA9685DmxParamsConst::FILE_NAME, pBuffer, nLength);

	if(m_Params.nAddress == 0) {
		m_Params.nAddress = pca9685Dmx.GetAddress();
	}

	builder.AddHex8(PCA9685DmxParamsConst::I2C_ADDRESS, m_Params.nAddress);

	builder.Add(PCA9685DmxParamsConst::MODE, pca9685dmxparams::get_mode(IsMaskSet(pca9685dmxparams::Mask::MODE)));

	if (m_Params.nChannelCount == 0) {
		m_Params.nChannelCount = pca9685Dmx.GetChannelCount();
	}

	builder.Add(PCA9685DmxParamsConst::CHANNEL_COUNT, m_Params.nChannelCount);

	if (m_Params.nDmxStartAddress == 0) {
		m_Params.nDmxStartAddress = pca9685Dmx.GetDmxStartAddress();
	}

	builder.Add(DmxNodeParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);

	builder.Add(PCA9685DmxParamsConst::USE_8BIT, IsMaskSet(pca9685dmxparams::Mask::USE_8BIT));

	builder.AddComment("mode=led");

	if (m_Params.nLedPwmFrequency < pca9685::Frequency::RANGE_MIN) {
		m_Params.nLedPwmFrequency = pca9685Dmx.GetLedPwmFrequency();
	}

	builder.Add(PCA9685DmxParamsConst::LED_PWM_FREQUENCY, m_Params.nLedPwmFrequency);
	builder.Add(PCA9685DmxParamsConst::LED_OUTPUT_INVERT, IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT), IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT));
	builder.Add(PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN, IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN), IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN));

	builder.AddComment("mode=servo");

	if (m_Params.nServoLeftUs == 0) {
		m_Params.nServoLeftUs = pca9685Dmx.GetServoLeftUs();
	}

	builder.Add(PCA9685DmxParamsConst::SERVO_LEFT_US, m_Params.nServoLeftUs);

	if (m_Params.nServoCenterUs == 0) {
		m_Params.nServoCenterUs = pca9685Dmx.GetServoCenterUs();
	}

	builder.Add(PCA9685DmxParamsConst::SERVO_CENTER_US, m_Params.nServoCenterUs);

	if (m_Params.nServoRightUs == 0) {
		m_Params.nServoRightUs = pca9685Dmx.GetServoRightUs();
	}

	builder.Add(PCA9685DmxParamsConst::SERVO_RIGHT_US, m_Params.nServoRightUs);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void PCA9685DmxParams::Set() {
	DEBUG_ENTRY
	auto& pca9685Dmx = PCA9685Dmx::Get();

	/*
	 * Generic
	 */

	pca9685Dmx.SetAddress(m_Params.nAddress);
	pca9685Dmx.SetMode(IsMaskSet(pca9685dmxparams::Mask::MODE));
	pca9685Dmx.SetChannelCount(m_Params.nChannelCount);
	pca9685Dmx.SetDmxStartAddress(m_Params.nDmxStartAddress);
	pca9685Dmx.SetUse8Bit(IsMaskSet(pca9685dmxparams::Mask::USE_8BIT));

	/*
	 * LED specific
	 */

	pca9685Dmx.SetLedPwmFrequency(m_Params.nLedPwmFrequency);
	pca9685Dmx.SetLedOutputInvert(IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT) ? pca9685::Invert::OUTPUT_INVERTED : pca9685::Invert::OUTPUT_NOT_INVERTED);
	pca9685Dmx.SetLedOutputDriver(IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN) ? pca9685::Output::DRIVER_OPENDRAIN : pca9685::Output::DRIVER_TOTEMPOLE);

	/*
	 * Servo specific
	 */

	pca9685Dmx.SetServoLeftUs(m_Params.nServoLeftUs);
	pca9685Dmx.SetServoCenterUs(m_Params.nServoCenterUs);
	pca9685Dmx.SetServoRightUs(m_Params.nServoRightUs);

	DEBUG_EXIT
}

void PCA9685DmxParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PCA9685DmxParamsConst::FILE_NAME);

	printf(" %s=0x%.2x\n", PCA9685DmxParamsConst::I2C_ADDRESS, m_Params.nAddress);

	const auto IsModeSet = IsMaskSet(pca9685dmxparams::Mask::MODE);
	printf(" %s=%s [%d]\n", PCA9685DmxParamsConst::MODE, pca9685dmxparams::get_mode(IsModeSet), IsModeSet);

	printf(" %s=%d\n", PCA9685DmxParamsConst::CHANNEL_COUNT, m_Params.nChannelCount);
	printf(" %s=%d\n", DmxNodeParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);

	/*
	 * LED specific
	 */

	printf(" %s=%d Hz\n", PCA9685DmxParamsConst::LED_PWM_FREQUENCY, m_Params.nLedPwmFrequency);

	const auto IsOutputInvertedSet = IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_INVERT);
	printf(" %s=%d [Output logic state %sinverted]\n", PCA9685DmxParamsConst::LED_OUTPUT_INVERT, IsOutputInvertedSet, IsOutputInvertedSet ? "" : "not ");

	const auto IsOutputOpendrainSet = IsMaskSet(pca9685dmxparams::Mask::LED_OUTPUT_OPENDRAIN);
	printf(" %s=%d [The 16 LEDn outputs are configured with %s structure]\n", PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN, IsOutputOpendrainSet, IsOutputOpendrainSet ? "an open-drain" : "a totem pole");

	const auto IsUse8BitSet = IsMaskSet(pca9685dmxparams::Mask::USE_8BIT);
	printf(" %s=%d [%d-bit]\n", PCA9685DmxParamsConst::USE_8BIT, IsUse8BitSet, IsUse8BitSet ? 8 : 16);

	/*
	 * Servo specific
	 */

	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_LEFT_US, m_Params.nServoLeftUs);
	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_CENTER_US, m_Params.nServoCenterUs);
	printf(" %s=%d\n", PCA9685DmxParamsConst::SERVO_RIGHT_US, m_Params.nServoRightUs);
}
