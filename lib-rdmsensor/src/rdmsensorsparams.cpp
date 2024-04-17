/**
 * @file rdmsensorsparams.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "rdmsensorsparams.h"
#include "rdmsensors.h"
#include "rdmsensorsconst.h"
#include "rdm_sensors.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#if defined (RDM_SENSORS_ENABLE)
# if !defined (CONFIG_RDM_SENSORS_DISABLE_BH170)
#  include "rdmsensorbh1750.h"
# endif
# if !defined (CONFIG_RDM_SENSORS_DISABLE_MCP9808)
#  include "rdmsensormcp9808.h"
# endif
# if !defined (CONFIG_RDM_SENSORS_DISABLE_HTU21D)
#  include "rdmsensorhtu21dhumidity.h"
#  include "rdmsensorhtu21dtemperature.h"
# endif
# if !defined (CONFIG_RDM_SENSORS_DISABLE_INA219)
#  include "rdmsensorina219current.h"
#  include "rdmsensorina219power.h"
#  include "rdmsensorina219voltage.h"
# endif
# if !defined (CONFIG_RDM_SENSORS_DISABLE_SI7021)
#  include "rdmsensorsi7021humidity.h"
#  include "rdmsensorsi7021temperature.h"
# endif
# if !defined (CONFIG_RDM_SENSORS_DISABLE_THERMISTOR)
#  include "rdmsensorthermistor.h"
# endif
#endif

#include "debug.h"

RDMSensorsParams::RDMSensorsParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct rdm::sensorsparams::Params));

	DEBUG_EXIT
}

void RDMSensorsParams::Load() {
	DEBUG_ENTRY

	m_Params.nDevices = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(RDMSensorsParams::staticCallbackFunction, this);

	if (configfile.Read(RDMSensorsConst::PARAMS_FILE_NAME)) {
		RDMSensorsParamsStore::Update(&m_Params);
	} else
#endif
		RDMSensorsParamsStore::Copy(&m_Params);

	// Sanity check
	if (m_Params.nDevices >= rdm::sensors::devices::MAX) {
		memset(&m_Params, 0, sizeof(struct rdm::sensorsparams::Params));
	}

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RDMSensorsParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nDevices = 0;

	ReadConfigFile config(RDMSensorsParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	RDMSensorsParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RDMSensorsParams::Builder(const rdm::sensorsparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct rdm::sensorsparams::Params));
	} else {
		RDMSensorsParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(RDMSensorsConst::PARAMS_FILE_NAME, pBuffer, nLength);

	for (uint32_t i = 0; i < static_cast<uint32_t>(rdm::sensors::Types::UNDEFINED); i++) {
		builder.AddHex8(rdm::sensors::get_type_string(static_cast<rdm::sensors::Types>(i)), 0xFF, false);
	}

	for (uint32_t i = 0; i < m_Params.nDevices; i++) {
		const auto type = static_cast<rdm::sensors::Types>(m_Params.Entry[i].nType);
		if (type < rdm::sensors::Types::UNDEFINED) {
			builder.AddHex8(rdm::sensors::get_type_string(type), m_Params.Entry[i].nAddress, true);
		}
	}

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
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
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_Params.nDevices; i++) {
		auto nSensorNumber = RDMSensors::Get()->GetCount();
		const auto nAddress = m_Params.Entry[i].nAddress;

		switch (static_cast<rdm::sensors::Types>(m_Params.Entry[i].nType)) {
#if !defined (CONFIG_RDM_SENSORS_DISABLE_BH170)
		case rdm::sensors::Types::BH170:
			Add(new RDMSensorBH170(nSensorNumber, nAddress));
			break;
#endif
#if !defined (CONFIG_RDM_SENSORS_DISABLE_HTU21D)
		case rdm::sensors::Types::HTU21D:
			if (!Add(new RDMSensorHTU21DHumidity(nSensorNumber++, nAddress))) {
				continue;
			}
			Add(new RDMSensorHTU21DTemperature(nSensorNumber, nAddress));
			break;
#endif
#if !defined (CONFIG_RDM_SENSORS_DISABLE_INA219)
		case rdm::sensors::Types::INA219:
			if (!Add(new RDMSensorINA219Current(nSensorNumber++, nAddress))) {
				continue;
			}
			if (!Add(new RDMSensorINA219Power(nSensorNumber++, nAddress))) {
				continue;
			}
			Add(new RDMSensorINA219Voltage(nSensorNumber, nAddress));
			break;
#endif
#if !defined (CONFIG_RDM_SENSORS_DISABLE_MCP9808)
		case rdm::sensors::Types::MCP9808:
			Add(new RDMSensorMCP9808(nSensorNumber, nAddress));
			break;
#endif
#if !defined (CONFIG_RDM_SENSORS_DISABLE_SI7021)
		case rdm::sensors::Types::SI7021:
			if (!Add(new RDMSensorSI7021Humidity(nSensorNumber++, nAddress))) {
				continue;
			}
			Add(new RDMSensorSI7021Temperature(nSensorNumber, nAddress));
			break;
#endif
#if !defined (CONFIG_RDM_SENSORS_DISABLE_THERMISTOR)
		case rdm::sensors::Types::MCP3424:
			if (!Add(new RDMSensorThermistor(nSensorNumber, nAddress, 0, m_Params.nCalibrate[nSensorNumber]))) {
				continue;
			}
			nSensorNumber++;
			if (!Add(new RDMSensorThermistor(nSensorNumber, nAddress, 1, m_Params.nCalibrate[nSensorNumber]))) {
				continue;
			}
			nSensorNumber++;
			if (!Add(new RDMSensorThermistor(nSensorNumber, nAddress, 2, m_Params.nCalibrate[nSensorNumber]))) {
				continue;
			}
			nSensorNumber++;
			Add(new RDMSensorThermistor(nSensorNumber, nAddress, 3, m_Params.nCalibrate[nSensorNumber]));
			break;
#endif
		default:
			break;
		}
	}

	DEBUG_EXIT
#endif
}

void RDMSensorsParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	if (m_Params.nDevices == rdm::sensors::devices::MAX) {
		return;
	}

	char aSensorName[32];
	memset(aSensorName, 0, sizeof(aSensorName));

	uint8_t nLength = sizeof(aSensorName) - 1;
	uint8_t nI2cAddress = 0;
	uint8_t nReserved;

	const auto nReturnCode = Sscan::I2c(pLine, aSensorName, nLength, nI2cAddress, nReserved);

	if ((nReturnCode == Sscan::OK) && (aSensorName[0] != 0) && (nLength != 0)) {
		DEBUG_PRINTF("{%.*s}:%d, address=0x%.2x", nLength, aSensorName, nLength, nI2cAddress);

		rdm::sensors::Types sensorType;

		if ((sensorType = rdm::sensors::get_type_string(aSensorName)) == rdm::sensors::Types::UNDEFINED) {
			return;
		}

		uint32_t i;

		for (i = 0; i < m_Params.nDevices; i++) {
			if ((nI2cAddress != 0) && (m_Params.Entry[i].nAddress == nI2cAddress)) {
				m_Params.Entry[i].nType = static_cast<uint8_t>(sensorType);
				return;
			}
			if ((nI2cAddress == 0) && (m_Params.Entry[i].nType == static_cast<uint8_t>(sensorType))) {
				return;
			}
		}

		m_Params.nDevices++;
		m_Params.Entry[i].nType = static_cast<uint8_t>(sensorType);
		m_Params.Entry[i].nAddress = nI2cAddress;
	}
}

void RDMSensorsParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RDMSensorsParams*>(p))->callbackFunction(s);
}

void RDMSensorsParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RDMSensorsConst::PARAMS_FILE_NAME);

	for (uint32_t i = 0; i < m_Params.nDevices; i++) {
		printf(" %s 0x%.2x\n", rdm::sensors::get_type_string(static_cast<rdm::sensors::Types>(m_Params.Entry[i].nType)), m_Params.Entry[i].nAddress);
	}

	for (uint32_t i = 0; i < rdm::sensors::MAX; i++) {
		printf("%2u %u\n", static_cast<unsigned int>(i), static_cast<unsigned int>(m_Params.nCalibrate[i]));
	}
}
