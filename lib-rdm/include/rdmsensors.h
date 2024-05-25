/**
 * @file rdmsensors.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "rdmsensor.h"

#include "debug.h"

#if defined (__APPLE__) || (defined (__linux__) && !defined (RASPPI))
#else
# define RDM_SENSORS_ENABLE
#endif

#if !defined (__APPLE__)
# define RDMSENSOR_CPU_ENABLE
#endif

#if defined (NODE_RDMNET_LLRP_ONLY)
# undef RDM_SENSORS_ENABLE
# undef RDMSENSOR_CPU_ENABLE
#endif

#if defined (RDMSENSOR_CPU_ENABLE)
# include "sensor/cputemperature.h"
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
	RDMSensors() {
		DEBUG_ENTRY
		assert(s_pThis == nullptr);
		s_pThis = this;

#if defined (RDM_SENSORS_ENABLE) || defined (RDMSENSOR_CPU_ENABLE)
		m_pRDMSensor = new RDMSensor*[rdm::sensors::MAX];
		assert(m_pRDMSensor != nullptr);

# if defined (RDMSENSOR_CPU_ENABLE)
		Add(new CpuTemperature(m_nCount));
# endif
#endif
		DEBUG_EXIT
	}

	~RDMSensors() {
		DEBUG_ENTRY
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSensor[i] != nullptr) {
				delete m_pRDMSensor[i];
				m_pRDMSensor[i] = nullptr;
			}
		}

		delete [] m_pRDMSensor;
		DEBUG_EXIT
	}

	bool Add(RDMSensor *pRDMSensor) {
		DEBUG_ENTRY

		assert(m_pRDMSensor != nullptr);

		if (m_pRDMSensor == nullptr) {
			DEBUG_EXIT
			return false;
		}

		if (m_nCount == rdm::sensors::MAX) {
			DEBUG_EXIT
			return false;
		}

		assert(pRDMSensor != nullptr);
		m_pRDMSensor[m_nCount++] = pRDMSensor;

		DEBUG_PRINTF("m_nCount=%u", m_nCount);
		DEBUG_EXIT
		return true;
	}

	uint8_t GetCount() const {
		return m_nCount;
	}

	const struct rdm::sensor::Defintion *GetDefintion(const uint8_t nSensor) {
		assert(nSensor < m_nCount);
		assert(m_pRDMSensor[nSensor] != nullptr);
		return m_pRDMSensor[nSensor]->GetDefintion();
	}

	const struct rdm::sensor::Values *GetValues(const uint8_t nSensor) {
		assert(nSensor < m_nCount);

		assert(m_pRDMSensor[nSensor] != nullptr);
		return m_pRDMSensor[nSensor]->GetValues();
	}

	void SetValues(const uint8_t nSensor) {
		if (nSensor == 0xFF) {
			for (uint32_t i = 0; i < m_nCount; i++) {
				m_pRDMSensor[i]->SetValues();
			}
		} else {
			m_pRDMSensor[nSensor]->SetValues();
		}
	}

	void SetRecord(const uint8_t nSensor) {
		if (nSensor == 0xFF) {
			for (uint32_t i = 0; i < m_nCount; i++) {
				m_pRDMSensor[i]->Record();
			}
		} else {
			m_pRDMSensor[nSensor]->Record();
		}
	}

	RDMSensor *GetSensor(uint8_t nSensor) {
		return m_pRDMSensor[nSensor];
	}

	static RDMSensors* Get() {
		return s_pThis;
	}

private:
	RDMSensor **m_pRDMSensor { nullptr };
	uint8_t m_nCount { 0 };

	static RDMSensors *s_pThis;
};

#endif /* RDMSENSORS_H_ */
