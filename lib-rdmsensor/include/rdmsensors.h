/**
 * @file rdmsensors.h
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

#ifndef RDMSENSORS_H_
#define RDMSENSORS_H_

#include <stdint.h>

#include "rdmsensor.h"
#include "rdmsensorsconst.h"

#if defined (RASPPI) || defined(BARE_METAL)
# define RDM_SENSORS_ENABLE
#endif

#if !defined (__CYGWIN__) && !defined (__APPLE__)
# define RDMSENSOR_CPU_ENABLE
#endif

#if defined (RDMNET_LLRP_ONLY)
# undef RDM_SENSORS_ENABLE
# undef RDMSENSOR_CPU_ENABLE
#endif

namespace rdm {
namespace sensors {
static constexpr uint32_t max = 16;
static constexpr uint32_t store = 64;	// bytes
}  // namespace sensors
}  // namespace rdm

class RDMSensors {
public:
	RDMSensors();
	~RDMSensors();

	bool Add(RDMSensor *pRDMSensor);
	uint8_t GetCount() {
		return m_nCount;
	}

	const struct TRDMSensorDefintion* GetDefintion(uint8_t nSensor);
	const struct TRDMSensorValues* GetValues(uint8_t nSensor);
	void SetValues(uint8_t nSensor);
	void SetRecord(uint8_t nSensor);

	static const char *GetTypeString(rdm::sensors::type tType);
	static rdm::sensors::type GetTypeString(const char *pValue);

	static RDMSensors* Get() {
		return s_pThis;
	}

private:
	RDMSensor **m_pRDMSensor{nullptr};
	uint8_t m_nCount{0};

	static RDMSensors *s_pThis;
};

#endif /* RDMSENSORS_H_ */
