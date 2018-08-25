/**
 * @file rdmsensors.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "rdmsensors.h"

#include "readconfigfile.h"
#include "sscan.h"

#if !defined (__CYGWIN__) && !defined (__APPLE__)
 #include "cputemperature.h"
#endif

#if defined (HAVE_I2C)
#include "i2c.h"

#include "sensorbh1750.h"
#include "sensormcp9808.h"
#include "sensorhtu21dhumidity.h"
#include "sensorhtu21dtemperature.h"
#include "sensorina219current.h"
#include "sensorina219power.h"
#include "sensorina219voltage.h"
#include "sensorsi7021humidity.h"
#include "sensorsi7021temperature.h"

static const char SENSORS_PARAMS_FILE_NAME[] ALIGNED = "sensors.txt";
#endif

RDMSensors::RDMSensors(void): m_pRDMSensor(0), m_nCount(0) {
}

RDMSensors::~RDMSensors(void) {
	for (unsigned i = 0; i < m_nCount; i++) {
		if (m_pRDMSensor[i] != 0) {
			delete m_pRDMSensor[i];
			m_pRDMSensor[i] = 0;
		}
	}

	delete [] m_pRDMSensor;

	m_nCount = 0;
}

void RDMSensors::Init(void) {
#if !defined (__CYGWIN__) && !defined (__APPLE__)
	m_pRDMSensor = new RDMSensor*[0xFE];
	assert(m_pRDMSensor != 0);

	Add(new CpuTemperature(m_nCount));
#endif
#if defined (HAVE_I2C)
	if(i2c_begin()) {	// We have I2C sensors only
		ReadConfigFile configfile(RDMSensors::staticCallbackFunction, this);
		(void) configfile.Read(SENSORS_PARAMS_FILE_NAME);
	}
#endif
#ifndef NDEBUG
	printf("Sensors added: %d\n", (int) m_nCount);
#endif
}

bool RDMSensors::Add(RDMSensor* pRDMSensor) {
	if (m_nCount == 0xFE) {
		return false;
	}

	assert(pRDMSensor != 0);

	if (!pRDMSensor->Initialize()) {
		delete pRDMSensor;
		pRDMSensor = 0;
		return false;
	}

	m_pRDMSensor[m_nCount++] = pRDMSensor;

	return true;
}

uint8_t RDMSensors::GetCount(void) const {
	return m_nCount;
}

const struct TRDMSensorDefintion* RDMSensors::GetDefintion(uint8_t nSensor) {
	assert(nSensor < m_nCount);

	assert(m_pRDMSensor[nSensor] != 0);
	return m_pRDMSensor[nSensor]->GetDefintion();
}

const struct TRDMSensorValues* RDMSensors::GetValues(uint8_t nSensor) {
	assert(nSensor < m_nCount);

	assert(m_pRDMSensor[nSensor] != 0);
	return m_pRDMSensor[nSensor]->GetValues();
}

void RDMSensors::SetSensorValues(uint8_t nSensor) {
	if (nSensor == (uint8_t) 0xFF) {
		for (uint8_t i = 0; i < m_nCount ; i++) {
			m_pRDMSensor[i]->SetValues();
		}
	} else {
		m_pRDMSensor[nSensor]->SetValues();
	}
}

void RDMSensors::SetSensorRecord(uint8_t nSensor) {
	if (nSensor == (uint8_t) 0xFF) {
		for (uint8_t i = 0; i < m_nCount ; i++) {
			m_pRDMSensor[i]->Record();
		}
	} else {
		m_pRDMSensor[nSensor]->Record();
	}
}

void RDMSensors::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((RDMSensors *) p)->callbackFunction(s);
}

void RDMSensors::callbackFunction(const char *pLine) {
	assert(pLine != 0);
#if defined (HAVE_I2C)
	int nReturnCode;
	char aSensorName[32];
	uint8_t nLength = sizeof(aSensorName) - 1;
	uint8_t nI2cAddress = 0;
	uint8_t nI2cChannel = 0; // TODO Replace with I2C name

	memset(aSensorName, 0, sizeof(aSensorName));

	nReturnCode = Sscan::I2c(pLine, aSensorName, &nLength, &nI2cAddress, &nI2cChannel);

	if ((nReturnCode != 0) && (aSensorName[0] != 0) && (nLength != (uint8_t) 0)) {
#ifndef NDEBUG
		printf("%s -> sensor_name={%.*s}:%d, address=%.2x, channel=%d\n", pLine, nLength, aSensorName, (int) nLength, nI2cAddress, (int) nI2cChannel);
#endif
		if (memcmp(aSensorName, "bh1750", 6) == 0) {						// BH1750
			Add(new SensorBH1750(m_nCount, nI2cAddress));
		} else if (memcmp(aSensorName, "htu21d", 6) == 0) {					// HTU21D
			if (Add(new SensorHTU21DHumidity(m_nCount, nI2cAddress))) {
				Add(new SensorHTU21DTemperature(m_nCount, nI2cAddress));
			}
		} else if (memcmp(aSensorName, "ina219", 6) == 0) {					// INA219
			if (Add(new SensorINA219Current(m_nCount, nI2cAddress))) {
				if (Add(new SensorINA219Power(m_nCount, nI2cAddress))) {
					Add(new SensorINA219Voltage(m_nCount, nI2cAddress));
				}
			}
		} else if (memcmp(aSensorName, "mcp9808", 7) == 0) {				// MCP9808
			Add(new SensorMCP9808(m_nCount, nI2cAddress));
		} else if (memcmp(aSensorName, "si7021", 6) == 0) {					// SI7021
			if (Add(new SensorSI7021Humidity(m_nCount, nI2cAddress))) {
				Add(new SensorSI7021Temperature(m_nCount, nI2cAddress));
			}
		}
	}
#endif
}
