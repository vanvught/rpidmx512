/**
 * @file sparkfundmxparams.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#ifndef NDEBUG
 #include <cstdio>
#endif
#include <cassert>

#include "sparkfundmxparams.h"
#include "sparkfundmxparamsconst.h"

#include "l6470dmxconst.h"

#include "sparkfundmx.h"
#include "sparkfundmx_internal.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

SparkFunDmxParams::SparkFunDmxParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;
	m_Params.nSpiCs = SPI_CS0;
	m_Params.nResetPin = GPIO_RESET_OUT;
	m_Params.nBusyPin = GPIO_BUSY_IN;

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));

	DEBUG_EXIT
}

void SparkFunDmxParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(SparkFunDmxParams::StaticCallbackFunction, this);

	if (configfile.Read(SparkFunDmxParamsConst::FILE_NAME)) {
		SparkFunDmxParamsStore::Update(&m_Params);
	} else
#endif
		SparkFunDmxParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void SparkFunDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(SparkFunDmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	SparkFunDmxParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void SparkFunDmxParams::Load(uint32_t nMotorIndex) {
	DEBUG_ENTRY
	assert(nMotorIndex < SPARKFUN_DMX_MAX_MOTORS);

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	m_Params.nSetList = 0;

	ReadConfigFile configfile(SparkFunDmxParams::StaticCallbackFunction, this);

#if !defined(DISABLE_FS)
	if (configfile.Read(m_aFileName)) {
		SparkFunDmxParamsStore::Update(nMotorIndex, &m_Params);
	} else
#endif
		SparkFunDmxParamsStore::Copy(nMotorIndex, &m_Params);

#ifndef NDEBUG
	Dump(nMotorIndex);
#endif
	DEBUG_EXIT
}

void SparkFunDmxParams::Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(SparkFunDmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	SparkFunDmxParamsStore::Update(nMotorIndex, &m_Params);

#ifndef NDEBUG
	Dump(nMotorIndex);
#endif
	DEBUG_EXIT
}

void SparkFunDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::POSITION, nValue8) == Sscan::OK) {
		if (nValue8 < SPARKFUN_DMX_MAX_MOTORS) {
			m_Params.nPosition = nValue8;
			m_Params.nSetList |= sparkfundmxparams::Mask::POSITION;
		}
		return;
	}

#if !defined (H3)
	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::SPI_CS, nValue8) == Sscan::OK) {
		m_Params.nSpiCs = nValue8;
		m_Params.nSetList |= sparkfundmxparams::Mask::SPI_CS;
		return;
	}
#endif

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::RESET_PIN, nValue8) == Sscan::OK) {
		m_Params.nResetPin = nValue8;
		m_Params.nSetList |= sparkfundmxparams::Mask::RESET_PIN;
		return;
	}

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::BUSY_PIN, nValue8) == Sscan::OK) {
		m_Params.nBusyPin = nValue8;
		m_Params.nSetList |= sparkfundmxparams::Mask::BUSY_PIN;
		return;
	}
}

void SparkFunDmxParams::Builder(const struct sparkfundmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize, uint32_t nMotorIndex) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct sparkfundmxparams::Params));
	} else {
		if (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) {
			SparkFunDmxParamsStore::Copy(nMotorIndex, &m_Params);
			m_aFileName[5] = static_cast<char>(nMotorIndex + '0');
		} else {
			SparkFunDmxParamsStore::Copy(&m_Params);
		}
	}

	const char *pFileName = (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) ? m_aFileName : SparkFunDmxParamsConst::FILE_NAME;

	PropertiesBuilder builder(pFileName, pBuffer, nLength);

	if (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) {
		builder.Add(SparkFunDmxParamsConst::POSITION, m_Params.nPosition, isMaskSet(sparkfundmxparams::Mask::POSITION));
	}

	builder.Add(SparkFunDmxParamsConst::RESET_PIN, m_Params.nResetPin, isMaskSet(sparkfundmxparams::Mask::RESET_PIN));
	builder.Add(SparkFunDmxParamsConst::BUSY_PIN, m_Params.nBusyPin, isMaskSet(sparkfundmxparams::Mask::BUSY_PIN));

#if !defined (H3)
	builder.Add(SparkFunDmxParamsConst::SPI_CS, m_Params.nSpiCs, isMaskSet(sparkfundmxparams::Mask::SPI_CS));
#endif

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void SparkFunDmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize, uint32_t nMotorIndex) {
	DEBUG_ENTRY

	Builder(nullptr, pBuffer, nLength, nSize, nMotorIndex);

	DEBUG_EXIT
}

void SparkFunDmxParams::SetGlobal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != nullptr);

#if !defined (H3)
	if (isMaskSet(sparkfundmxparams::Mask::SPI_CS)) {
		pSparkFunDmx->SetGlobalSpiCs(m_Params.nSpiCs);
	}
#endif

	if (isMaskSet(sparkfundmxparams::Mask::RESET_PIN)) {
		pSparkFunDmx->SetGlobalResetPin(m_Params.nResetPin);
	}

	if (isMaskSet(sparkfundmxparams::Mask::BUSY_PIN)) {
		pSparkFunDmx->SetGlobalBusyPin(m_Params.nBusyPin);
	}
}

void SparkFunDmxParams::SetLocal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != nullptr);

	if (isMaskSet(sparkfundmxparams::Mask::POSITION)) {
		pSparkFunDmx->SetLocalPosition(m_Params.nPosition);
	}

#if !defined (H3)
	if (isMaskSet(sparkfundmxparams::Mask::SPI_CS)) {
		pSparkFunDmx->SetLocalSpiCs(m_Params.nSpiCs);
	}
#endif

	if (isMaskSet(sparkfundmxparams::Mask::RESET_PIN)) {
		pSparkFunDmx->SetLocalResetPin(m_Params.nResetPin);
	}

	if (isMaskSet(sparkfundmxparams::Mask::BUSY_PIN)) {
		pSparkFunDmx->SetLocalBusyPin(m_Params.nBusyPin);
	}
}

void SparkFunDmxParams::Dump([[maybe_unused]] uint32_t nMotorIndex) {
	assert(SPARKFUN_DMX_MAX_MOTORS <= 9);

	if (nMotorIndex >= SPARKFUN_DMX_MAX_MOTORS) {
		printf("%s::%s \'%s\' (global settings):\n", __FILE__, __FUNCTION__, SparkFunDmxParamsConst::FILE_NAME);
	} else {
		m_aFileName[5] = nMotorIndex + '0';
		printf("%s::%s \'%s\' :\n", __FILE__, __FUNCTION__, m_aFileName);
		printf(" %s=%d [%u]\n", SparkFunDmxParamsConst::POSITION, m_Params.nPosition, isMaskSet(sparkfundmxparams::Mask::POSITION));
	}

#if !defined (H3)
	if (isMaskSet(sparkfundmxparams::Mask::SPI_CS)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::SPI_CS, m_Params.nSpiCs);
	}
#endif

	printf(" %s=%d [%u]\n", SparkFunDmxParamsConst::RESET_PIN, m_Params.nResetPin, isMaskSet(sparkfundmxparams::Mask::RESET_PIN));
	printf(" %s=%d [%u]\n", SparkFunDmxParamsConst::BUSY_PIN, m_Params.nBusyPin, isMaskSet(sparkfundmxparams::Mask::BUSY_PIN));
}

void SparkFunDmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<SparkFunDmxParams*>(p))->callbackFunction(s);
}

