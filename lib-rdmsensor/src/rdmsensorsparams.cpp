/**
 * @file rdmsensorsparams.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>
#include <cassert>

#include "rdmsensorsparams.h"
#include "rdmsensors.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#if defined (RDM_SENSORS_ENABLE)
# include "rdmsensorbh1750.h"
# include "rdmsensormcp9808.h"
# include "rdmsensorhtu21dhumidity.h"
# include "rdmsensorhtu21dtemperature.h"
# include "rdmsensorina219current.h"
# include "rdmsensorina219power.h"
# include "rdmsensorina219voltage.h"
# include "rdmsensorsi7021humidity.h"
# include "rdmsensorsi7021temperature.h"
#endif

#include "debug.h"

using namespace rdm::sensors;

RDMSensorsParams::RDMSensorsParams(RDMSensorsParamsStore *pRDMSensorsParamsStore): m_pRDMSensorsParamsStore(pRDMSensorsParamsStore) {
	DEBUG_PRINTF("sizeof(struct TRDMSensorsParams)=%d", static_cast<int>(sizeof(struct TRDMSensorsParams)));
}

bool RDMSensorsParams::Load() {
	DEBUG_ENTRY

	m_tRDMSensorsParams.nCount = 0;

	ReadConfigFile configfile(RDMSensorsParams::staticCallbackFunction, this);

	if (configfile.Read(RDMSensorsConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pRDMSensorsParamsStore != nullptr) {
			m_pRDMSensorsParamsStore->Update(&m_tRDMSensorsParams);
		}
	} else if (m_pRDMSensorsParamsStore != nullptr) {
		m_pRDMSensorsParamsStore->Copy(&m_tRDMSensorsParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void RDMSensorsParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pRDMSensorsParamsStore != nullptr);

	if (m_pRDMSensorsParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_tRDMSensorsParams.nCount = 0;

	ReadConfigFile config(RDMSensorsParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRDMSensorsParamsStore->Update(&m_tRDMSensorsParams);

	DEBUG_EXIT
}

void RDMSensorsParams::Builder(const struct TRDMSensorsParams *pRDMSensorsParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pRDMSensorsParams != nullptr) {
		memcpy(&m_tRDMSensorsParams, pRDMSensorsParams, sizeof(struct TRDMSensorsParams));
	} else {
		assert(m_pRDMSensorsParamsStore != nullptr);
		m_pRDMSensorsParamsStore->Copy(&m_tRDMSensorsParams);
	}

	PropertiesBuilder builder(RDMSensorsConst::PARAMS_FILE_NAME, pBuffer, nLength);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void RDMSensorsParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pRDMSensorsParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}

void RDMSensorsParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RDMSensorsConst::PARAMS_FILE_NAME);

	for (uint32_t i = 0; i < m_tRDMSensorsParams.nCount; i++) {
		printf(" %s 0x%.2x\n", RDMSensors::GetTypeString(static_cast<type>(m_tRDMSensorsParams.Entry[i].nType)), m_tRDMSensorsParams.Entry[i].nAddress);
	}
#endif
}

bool RDMSensorsParams::Add(RDMSensor *pRDMSensor) {
	DEBUG_ENTRY

	if (pRDMSensor->Initialize()) {
		RDMSensors::Get()->Add(pRDMSensor);
		DEBUG_EXIT
		return true;
	}

	delete pRDMSensor;

	DEBUG_EXIT
	return false;
}

void RDMSensorsParams::Set() {
#if defined (RDM_SENSORS_ENABLE)
	for (uint32_t i = 0; i < m_tRDMSensorsParams.nCount; i++) {
		auto nSensorNumber = RDMSensors::Get()->GetCount();
		const auto nAddress = m_tRDMSensorsParams.Entry[i].nAddress;

		switch (static_cast<type>(m_tRDMSensorsParams.Entry[i].nType)) {
			case type::BH170:
				Add(new RDMSensorBH170(nSensorNumber, nAddress));
				break;
			case type::HTU21D:
				if (Add(new RDMSensorHTU21DHumidity(nSensorNumber++, nAddress))){
					Add(new RDMSensorHTU21DTemperature(nSensorNumber, nAddress));
				}
				break;
			case type::INA219:
				if(Add(new RDMSensorINA219Current(nSensorNumber++, nAddress))) {
					Add(new RDMSensorINA219Power(nSensorNumber++, nAddress));
					Add(new RDMSensorINA219Voltage(nSensorNumber, nAddress));
				}
				break;
			case type::MCP9808:
				Add(new RDMSensorMCP9808(nSensorNumber, nAddress));
				break;
			case type::SI7021:
				if(Add(new RDMSensorSI7021Humidity(nSensorNumber++, nAddress))) {
					Add(new RDMSensorSI7021Temperature(nSensorNumber, nAddress));
				}
				break;
			default:
				break;
		}
	}
#endif
}

void RDMSensorsParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aSensorName[32];
	memset(aSensorName, 0, sizeof(aSensorName));

	uint8_t nLength = sizeof(aSensorName) - 1;
	uint8_t nI2cAddress = 0;
	uint8_t nI2cChannel = 0;

	const auto nReturnCode = Sscan::I2c(pLine, aSensorName, nLength, nI2cAddress, nI2cChannel);

	if ((nReturnCode == Sscan::OK) && (aSensorName[0] != 0) && (nLength != 0)) {
		DEBUG_PRINTF("{%.*s}:%d, address=0x%.2x, channel=%d", nLength, aSensorName, nLength, nI2cAddress, nI2cChannel);

		type sensorType;

		if ((sensorType = RDMSensors::GetTypeString(aSensorName)) == type::UNDEFINED) {
			return;
		}

		uint32_t i;

		for (i = 0; i < m_tRDMSensorsParams.nCount; i++) {
			if ((nI2cAddress != 0) && (m_tRDMSensorsParams.Entry[i].nAddress == nI2cAddress)) {
				m_tRDMSensorsParams.Entry[i].nType = sensorType;
				return;
			}
			if ((nI2cAddress == 0) && (m_tRDMSensorsParams.Entry[i].nType == sensorType)) {
				return;
			}
		}

		if (m_tRDMSensorsParams.nCount == rdm::sensors::max) {
			return;
		}

		m_tRDMSensorsParams.nCount++;
		m_tRDMSensorsParams.Entry[i].nType = sensorType;
		m_tRDMSensorsParams.Entry[i].nAddress = nI2cAddress;
	}
}

void RDMSensorsParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RDMSensorsParams*>(p))->callbackFunction(s);
}
