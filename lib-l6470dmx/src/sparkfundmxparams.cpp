/**
 * @file sparkfundmxparams.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
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

SparkFunDmxParams::SparkFunDmxParams(SparkFunDmxParamsStore *pSparkFunDmxParamsStore): m_pSparkFunDmxParamsStore(pSparkFunDmxParamsStore) {
	DEBUG_ENTRY

	m_tSparkFunDmxParams.nSetList = 0;
	m_tSparkFunDmxParams.nSpiCs = SPI_CS0;
	m_tSparkFunDmxParams.nResetPin = GPIO_RESET_OUT;
	m_tSparkFunDmxParams.nBusyPin = GPIO_BUSY_IN;

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));

	DEBUG_EXIT
}

bool SparkFunDmxParams::Load() {
	DEBUG_ENTRY

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile configfile(SparkFunDmxParams::staticCallbackFunction, this);

	if (configfile.Read(SparkFunDmxParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pSparkFunDmxParamsStore != nullptr) {
			m_pSparkFunDmxParamsStore->Update(&m_tSparkFunDmxParams);
		}
	} else if (m_pSparkFunDmxParamsStore != nullptr) {
		m_pSparkFunDmxParamsStore->Copy(&m_tSparkFunDmxParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void SparkFunDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pSparkFunDmxParamsStore != nullptr);

	if (m_pSparkFunDmxParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile config(SparkFunDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pSparkFunDmxParamsStore->Update(&m_tSparkFunDmxParams);

	DEBUG_EXIT
}

bool SparkFunDmxParams::Load(uint8_t nMotorIndex) {
	DEBUG_ENTRY
	assert(nMotorIndex < SPARKFUN_DMX_MAX_MOTORS);

	m_aFileName[5] = nMotorIndex + '0';

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile configfile(SparkFunDmxParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pSparkFunDmxParamsStore != nullptr) {
			m_pSparkFunDmxParamsStore->Update(nMotorIndex, &m_tSparkFunDmxParams);
		}
	} else if (m_pSparkFunDmxParamsStore != nullptr) {
		m_pSparkFunDmxParamsStore->Copy(nMotorIndex, &m_tSparkFunDmxParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void SparkFunDmxParams::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pSparkFunDmxParamsStore != nullptr);

	if (m_pSparkFunDmxParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile config(SparkFunDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pSparkFunDmxParamsStore->Update(nMotorIndex, &m_tSparkFunDmxParams);

	DEBUG_EXIT
}

void SparkFunDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::POSITION, nValue8) == Sscan::OK) {
		if (nValue8 < SPARKFUN_DMX_MAX_MOTORS) {
			m_tSparkFunDmxParams.nPosition = nValue8;
			m_tSparkFunDmxParams.nSetList |= SparkFunDmxParamsMask::POSITION;
		}
		return;
	}

#if !defined (H3)
	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::SPI_CS, nValue8) == Sscan::OK) {
		m_tSparkFunDmxParams.nSpiCs = nValue8;
		m_tSparkFunDmxParams.nSetList |= SparkFunDmxParamsMask::SPI_CS;
		return;
	}
#endif

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::RESET_PIN, nValue8) == Sscan::OK) {
		m_tSparkFunDmxParams.nResetPin = nValue8;
		m_tSparkFunDmxParams.nSetList |= SparkFunDmxParamsMask::RESET_PIN;
		return;
	}

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::BUSY_PIN, nValue8) == Sscan::OK) {
		m_tSparkFunDmxParams.nBusyPin = nValue8;
		m_tSparkFunDmxParams.nSetList |= SparkFunDmxParamsMask::BUSY_PIN;
		return;
	}
}

void SparkFunDmxParams::Builder(const struct TSparkFunDmxParams *ptSparkFunDmxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize, uint8_t nMotorIndex) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptSparkFunDmxParams != nullptr) {
		memcpy(&m_tSparkFunDmxParams, ptSparkFunDmxParams, sizeof(struct TSparkFunDmxParams));
	} else {
		if (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) {
			m_pSparkFunDmxParamsStore->Copy(nMotorIndex, &m_tSparkFunDmxParams);
			m_aFileName[5] = nMotorIndex + '0';
		} else {
			m_pSparkFunDmxParamsStore->Copy(&m_tSparkFunDmxParams);
		}
	}

	const char *pFileName = (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) ? m_aFileName : SparkFunDmxParamsConst::FILE_NAME;

	PropertiesBuilder builder(pFileName, pBuffer, nLength);

	if (nMotorIndex < SPARKFUN_DMX_MAX_MOTORS) {
		builder.Add(SparkFunDmxParamsConst::POSITION, m_tSparkFunDmxParams.nPosition, isMaskSet(SparkFunDmxParamsMask::POSITION));
	}

	builder.Add(SparkFunDmxParamsConst::RESET_PIN, m_tSparkFunDmxParams.nResetPin, isMaskSet(SparkFunDmxParamsMask::RESET_PIN));
	builder.Add(SparkFunDmxParamsConst::BUSY_PIN, m_tSparkFunDmxParams.nBusyPin, isMaskSet(SparkFunDmxParamsMask::BUSY_PIN));

#if !defined (H3)
	builder.Add(SparkFunDmxParamsConst::SPI_CS, m_tSparkFunDmxParams.nSpiCs, isMaskSet(SparkFunDmxParamsMask::SPI_CS));
#endif

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void SparkFunDmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize, uint8_t nMotorIndex) {
	DEBUG_ENTRY

	if (m_pSparkFunDmxParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize, nMotorIndex);

	DEBUG_EXIT
}

void SparkFunDmxParams::SetGlobal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != nullptr);

#if !defined (H3)
	if (isMaskSet(SparkFunDmxParamsMask::SPI_CS)) {
		pSparkFunDmx->SetGlobalSpiCs(m_tSparkFunDmxParams.nSpiCs);
	}
#endif

	if (isMaskSet(SparkFunDmxParamsMask::RESET_PIN)) {
		pSparkFunDmx->SetGlobalResetPin(m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SparkFunDmxParamsMask::BUSY_PIN)) {
		pSparkFunDmx->SetGlobalBusyPin(m_tSparkFunDmxParams.nBusyPin);
	}
}

void SparkFunDmxParams::SetLocal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != nullptr);

	if (isMaskSet(SparkFunDmxParamsMask::POSITION)) {
		pSparkFunDmx->SetLocalPosition(m_tSparkFunDmxParams.nPosition);
	}

#if !defined (H3)
	if (isMaskSet(SparkFunDmxParamsMask::SPI_CS)) {
		pSparkFunDmx->SetLocalSpiCs(m_tSparkFunDmxParams.nSpiCs);
	}
#endif

	if (isMaskSet(SparkFunDmxParamsMask::RESET_PIN)) {
		pSparkFunDmx->SetLocalResetPin(m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SparkFunDmxParamsMask::BUSY_PIN)) {
		pSparkFunDmx->SetLocalBusyPin(m_tSparkFunDmxParams.nBusyPin);
	}
}

void SparkFunDmxParams::Dump(__attribute__((unused)) uint8_t nMotorIndex) {
#ifndef NDEBUG
	assert(SPARKFUN_DMX_MAX_MOTORS <= 9);

	if (m_tSparkFunDmxParams.nSetList == 0) {
		return;
	}

	if (nMotorIndex >= SPARKFUN_DMX_MAX_MOTORS) {
		printf("%s::%s \'%s\' (global settings):\n", __FILE__, __FUNCTION__, SparkFunDmxParamsConst::FILE_NAME);
	} else {
		m_aFileName[5] = nMotorIndex + '0';
		printf("%s::%s \'%s\' :\n", __FILE__, __FUNCTION__, m_aFileName);
	}

	if (isMaskSet(SparkFunDmxParamsMask::POSITION)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::POSITION, m_tSparkFunDmxParams.nPosition);
	}

#if !defined (H3)
	if (isMaskSet(SparkFunDmxParamsMask::SPI_CS)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::SPI_CS, m_tSparkFunDmxParams.nSpiCs);
	}
#endif
	if (isMaskSet(SparkFunDmxParamsMask::RESET_PIN)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::RESET_PIN, m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SparkFunDmxParamsMask::BUSY_PIN)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::BUSY_PIN, m_tSparkFunDmxParams.nBusyPin);
	}
#endif
}

void SparkFunDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<SparkFunDmxParams*>(p))->callbackFunction(s);
}

