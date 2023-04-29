/**
 * @file rdmsensors.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "rdmsensor.h"
#include "rdmsensorsconst.h"

#if defined (RASPPI) || defined(BARE_METAL)
# define RDM_SENSORS_ENABLE
#endif

#if !defined (__CYGWIN__) && !defined (__APPLE__)
# define RDMSENSOR_CPU_ENABLE
#endif

#if defined (NODE_RDMNET_LLRP_ONLY)
# undef RDM_SENSORS_ENABLE
# undef RDMSENSOR_CPU_ENABLE
#endif

namespace rdm {
namespace sensors {
static constexpr auto MAX = 16;
static constexpr auto STORE = 64;	///< Configuration store in bytes
namespace devices {
static constexpr auto MAX = 8;
}  // namespace devices
}  // namespace sensors
}  // namespace rdm

class RDMSensors {
public:
	RDMSensors();
	~RDMSensors();

	bool Add(RDMSensor *pRDMSensor);

	uint8_t GetCount() const {
		return m_nCount;
	}

	const struct rdm::sensor::Defintion* GetDefintion(uint8_t nSensor);
	const struct rdm::sensor::Values* GetValues(uint8_t nSensor);
	void SetValues(uint8_t nSensor);
	void SetRecord(uint8_t nSensor);

	RDMSensor *GetSensor(uint8_t nSensor) {
		return m_pRDMSensor[nSensor];
	}

	static const char *GetTypeString(rdm::sensors::Types type);
	static rdm::sensors::Types GetTypeString(const char *pValue);

	static RDMSensors* Get() {
		return s_pThis;
	}

private:
	RDMSensor **m_pRDMSensor { nullptr };
	uint8_t m_nCount { 0 };

	static RDMSensors *s_pThis;
};

#endif /* RDMSENSORS_H_ */
