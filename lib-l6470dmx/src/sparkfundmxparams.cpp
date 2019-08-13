/**
 * @file sparkfundmxparams.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "sparkfundmxparams.h"
#include "sparkfundmxparamsconst.h"

#include "l6470dmxconst.h"

#include "sparkfundmx.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

SparkFunDmxParams::SparkFunDmxParams(SparkFunDmxParamsStore *pSparkFunDmxParamsStore): m_pSparkFunDmxParamsStore(pSparkFunDmxParamsStore) {
	m_tSparkFunDmxParams.nSpiCs = SPI_CS0;
	m_tSparkFunDmxParams.nResetPin = GPIO_RESET_OUT;
	m_tSparkFunDmxParams.nBusyPin = GPIO_BUSY_IN;

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	const char *src = (char *)L6470DmxConst::FILE_NAME_MOTOR;
	strncpy(m_aFileName, src, sizeof(m_aFileName));
}

SparkFunDmxParams::~SparkFunDmxParams(void) {
	m_tSparkFunDmxParams.nSetList = 0;
}

bool SparkFunDmxParams::Load(void) {
	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile configfile(SparkFunDmxParams::staticCallbackFunction, this);

	if (configfile.Read(SparkFunDmxParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pSparkFunDmxParamsStore != 0) {
			m_pSparkFunDmxParamsStore->Update(&m_tSparkFunDmxParams);
		}
	} else if (m_pSparkFunDmxParamsStore != 0) {
		m_pSparkFunDmxParamsStore->Copy(&m_tSparkFunDmxParams);
	} else {
		return false;
	}

	return true;
}

void SparkFunDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pSparkFunDmxParamsStore != 0);

	if (m_pSparkFunDmxParamsStore == 0) {
		return;
	}

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile config(SparkFunDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pSparkFunDmxParamsStore->Update(&m_tSparkFunDmxParams);
}

bool SparkFunDmxParams::Load(uint8_t nMotorIndex) {
	assert(nMotorIndex < SPARKFUN_DMX_MAX_MOTORS);

	m_aFileName[5] = (char) nMotorIndex + '0';

	m_tSparkFunDmxParams.nSetList = 0;

	ReadConfigFile configfile(SparkFunDmxParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pSparkFunDmxParamsStore != 0) {
			m_pSparkFunDmxParamsStore->Update(&m_tSparkFunDmxParams);
		}
	} else if (m_pSparkFunDmxParamsStore != 0) {
		m_pSparkFunDmxParamsStore->Copy(&m_tSparkFunDmxParams);
	} else {
		return false;
	}

	return true;
}

void SparkFunDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::POSITION, &value8) == SSCAN_OK) {
		if (value8 <= 9) {
			m_tSparkFunDmxParams.nPosition = value8;
			m_tSparkFunDmxParams.nSetList |= SPARKFUN_DMX_PARAMS_MASK_POSITION;
		}
		return;
	}

#if !defined (H3)
	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::SPI_CS, &value8) == SSCAN_OK) {
		m_tSparkFunDmxParams.nSpiCs = value8;
		m_tSparkFunDmxParams.nSetList |= SPARKFUN_DMX_PARAMS_MASK_SPI_CS;
		return;
	}
#endif

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::RESET_PIN, &value8) == SSCAN_OK) {
		m_tSparkFunDmxParams.nResetPin = value8;
		m_tSparkFunDmxParams.nSetList |= SPARKFUN_DMX_PARAMS_MASK_RESET_PIN;
		return;
	}

	if (Sscan::Uint8(pLine, SparkFunDmxParamsConst::BUSY_PIN, &value8) == SSCAN_OK) {
		m_tSparkFunDmxParams.nBusyPin = value8;
		m_tSparkFunDmxParams.nSetList |= SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN;
		return;
	}
}

bool SparkFunDmxParams::Builder(const struct TSparkFunDmxParams *ptSparkFunDmxParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	if (ptSparkFunDmxParams != 0) {
		memcpy(&m_tSparkFunDmxParams, ptSparkFunDmxParams, sizeof(struct TSparkFunDmxParams));
	} else {
		m_pSparkFunDmxParamsStore->Copy(&m_tSparkFunDmxParams);
	}

	PropertiesBuilder builder(SparkFunDmxParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(SparkFunDmxParamsConst::RESET_PIN, (uint32_t) m_tSparkFunDmxParams.nResetPin, isMaskSet(SPARKFUN_DMX_PARAMS_MASK_RESET_PIN));
	isAdded &= builder.Add(SparkFunDmxParamsConst::BUSY_PIN, (uint32_t) m_tSparkFunDmxParams.nBusyPin, isMaskSet(SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN));

#if !defined (H3)
	isAdded &= builder.Add(SparkFunDmxParamsConst::SPI_CS, (uint32_t) m_tSparkFunDmxParams.nSpiCs, isMaskSet(SPARKFUN_DMX_PARAMS_MASK_SPI_CS));
#endif

	nSize = builder.GetSize();

	return isAdded;
}

bool SparkFunDmxParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {

	if (m_pSparkFunDmxParamsStore == 0) {
		nSize = 0;
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}

void SparkFunDmxParams::SetGlobal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != 0);

#if !defined (H3)
	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_SPI_CS)) {
		pSparkFunDmx->SetGlobalSpiCs(m_tSparkFunDmxParams.nSpiCs);
	}
#endif

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_RESET_PIN)) {
		pSparkFunDmx->SetGlobalResetPin(m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN)) {
		pSparkFunDmx->SetGlobalBusyPin(m_tSparkFunDmxParams.nBusyPin);
	}
}

void SparkFunDmxParams::SetLocal(SparkFunDmx *pSparkFunDmx) {
	assert(pSparkFunDmx != 0);

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_POSITION)) {
		pSparkFunDmx->SetLocalPosition(m_tSparkFunDmxParams.nPosition);
	}

#if !defined (H3)
	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_SPI_CS)) {
		pSparkFunDmx->SetLocalSpiCs(m_tSparkFunDmxParams.nSpiCs);
	}
#endif

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_RESET_PIN)) {
		pSparkFunDmx->SetLocalResetPin(m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN)) {
		pSparkFunDmx->SetLocalBusyPin(m_tSparkFunDmxParams.nBusyPin);
	}
}

void SparkFunDmxParams::Dump(uint8_t nMotorIndex) {
#ifndef NDEBUG
	assert(SPARKFUN_DMX_MAX_MOTORS <= 9);

	if (m_tSparkFunDmxParams.nSetList == 0) {
		return;
	}

	if (nMotorIndex >= SPARKFUN_DMX_MAX_MOTORS) {
		printf("%s::%s \'%s\' (global settings):\n", __FILE__, __FUNCTION__, SparkFunDmxParamsConst::FILE_NAME);
	} else {
		m_aFileName[5] = (char) nMotorIndex + '0';
		printf("%s::%s \'%s\' :\n", __FILE__, __FUNCTION__, m_aFileName);
	}

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_POSITION)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::POSITION, m_tSparkFunDmxParams.nPosition);
	}

#if !defined (H3)
	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_SPI_CS)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::SPI_CS, m_tSparkFunDmxParams.nSpiCs);
	}
#endif
	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_RESET_PIN)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::RESET_PIN, m_tSparkFunDmxParams.nResetPin);
	}

	if (isMaskSet(SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN)) {
		printf(" %s=%d\n", SparkFunDmxParamsConst::BUSY_PIN, m_tSparkFunDmxParams.nBusyPin);
	}
#endif
}

void SparkFunDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((SparkFunDmxParams *) p)->callbackFunction(s);
}

bool SparkFunDmxParams::isMaskSet(uint16_t nMask) const {
	return (m_tSparkFunDmxParams.nSetList & nMask) == nMask;
}
