/**
 * @file rdmsubdevices.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSUBDEVICES_H_
#define RDMSUBDEVICES_H_

#include <cstdint>
#include <cassert>

#include "rdmsubdevice.h"

#ifndef NDEBUG
# include "subdevice/rdmsubdevicedummy.h"
#endif

#include "rdmpersonality.h"

#include "debug.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# undef CONFIG_RDM_ENABLE_SUBDEVICES
#endif

namespace rdm {
namespace subdevices {
static constexpr auto MAX = 8;
static constexpr auto STORE = 96;	///< Configuration store in bytes
}  // namespace subdevices
}  // namespace rdm

class RDMSubDevices {
public:
	RDMSubDevices()  {
		DEBUG_ENTRY
		assert(s_pThis == nullptr);
		s_pThis = this;

#if defined(CONFIG_RDM_ENABLE_SUBDEVICES)
		m_pRDMSubDevice = new RDMSubDevice*[rdm::subdevices::MAX];
		assert(m_pRDMSubDevice != nullptr);

# ifndef NDEBUG
		Add(new RDMSubDeviceDummy);
# endif
#endif
		DEBUG_EXIT
	}

	~RDMSubDevices() {
		DEBUG_ENTRY
		for (unsigned i = 0; i < m_nCount; i++) {
			delete m_pRDMSubDevice[i];
			m_pRDMSubDevice[i] = nullptr;
		}

		delete [] m_pRDMSubDevice;

		m_nCount = 0;
		DEBUG_EXIT
	}

#undef UNUSED
#if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
# define UNUSED
#else
# define UNUSED  [[maybe_unused]]
#endif

	bool Add(UNUSED RDMSubDevice *pRDMSubDevice) {
		DEBUG_ENTRY
#if defined(CONFIG_RDM_ENABLE_SUBDEVICES)
		assert(m_pRDMSubDevice != nullptr);

		if (m_pRDMSubDevice == nullptr) {
			return false;
		}

		if (m_nCount == rdm::subdevices::MAX) {
			DEBUG_EXIT
			return false;
		}

		assert(pRDMSubDevice != nullptr);
		m_pRDMSubDevice[m_nCount++] = pRDMSubDevice;
#endif
		DEBUG_EXIT
		return true;
	}

	uint16_t GetCount() const {
		return m_nCount;
	}

	struct TRDMSubDevicesInfo *GetInfo(const uint16_t nSubDevice) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetInfo();
	}

	uint16_t GetDmxFootPrint(const uint16_t nSubDevice) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetDmxFootPrint();
	}

	RDMPersonality *GetPersonality(const uint16_t nSubDevice, const uint8_t nPersonality) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetPersonality(nPersonality);
	}

	uint8_t GetPersonalityCount(const uint16_t nSubDevice) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCount();
	}

	uint8_t GetPersonalityCurrent(const uint16_t nSubDevice) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCurrent();
	}

	void SetPersonalityCurrent(const uint16_t nSubDevice, const uint8_t nPersonality) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		m_pRDMSubDevice[nSubDevice - 1]->SetPersonalityCurrent(nPersonality);
	}

	// E120_DEVICE_LABEL			0x0082
	void GetLabel(const uint16_t nSubDevice, struct TRDMDeviceInfoData* pInfoData) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(pInfoData != nullptr);

		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		m_pRDMSubDevice[nSubDevice - 1]->GetLabel(pInfoData);
	}

	void SetLabel(const uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(pLabel != nullptr);

		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		m_pRDMSubDevice[nSubDevice - 1]->SetLabel(pLabel,nLabelLength);
	}

	// E120_FACTORY_DEFAULTS		0x0090
	bool GetFactoryDefaults() {
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSubDevice[i] != nullptr) {
				if (!m_pRDMSubDevice[i]->GetFactoryDefaults()) {
					return false;
				}
			}
		}

		return true;
	}

	void SetFactoryDefaults() {
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSubDevice[i] != nullptr) {
				m_pRDMSubDevice[i]->SetFactoryDefaults();
			}
		}
	}

	// E120_DMX_START_ADDRESS		0x00F0
	uint16_t GetDmxStartAddress(const uint16_t nSubDevice) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		return m_pRDMSubDevice[nSubDevice - 1]->GetDmxStartAddress();
	}

	void SetDmxStartAddress(const uint16_t nSubDevice, const uint16_t nDmxStartAddress) {
		assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
		assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
		m_pRDMSubDevice[nSubDevice - 1]->SetDmxStartAddress(nDmxStartAddress);
	}


	void Start() {
		DEBUG_ENTRY
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSubDevice[i] != nullptr) {
				m_pRDMSubDevice[i]->Start();
			}
		}
		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSubDevice[i] != nullptr) {
				m_pRDMSubDevice[i]->Stop();
			}
		}
		DEBUG_ENTRY
	}

	void SetData(const uint8_t *pData, const uint32_t nLength) {
		for (uint32_t i = 0; i < m_nCount; i++) {
			if (m_pRDMSubDevice[i] != nullptr) {
				if (nLength >= (static_cast<uint16_t>(m_pRDMSubDevice[i]->GetDmxStartAddress() + m_pRDMSubDevice[i]->GetDmxFootPrint()) - 1U)) {
					m_pRDMSubDevice[i]->Data(pData, nLength);
				}
			}
		}
	}

	static RDMSubDevices* Get() {
		return s_pThis;
	}

private:
	RDMSubDevice **m_pRDMSubDevice { nullptr };
	uint16_t m_nCount { 0 };

	static RDMSubDevices *s_pThis;
};

#if defined (UNUSED)
# undef UNUSED
#endif
#endif /* RDMSUBDEVICES_H_ */
