/**
 * @file rdmnetllrponly.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMNETLLRPONLY_H_
#define RDMNETLLRPONLY_H_

#include <cstring>

#include "rdmnetdevice.h"
#include "rdmidentify.h"

namespace rdmnetllrponly {
static constexpr char LABEL[] = "RDMNet LLRP Only";
static constexpr auto LABEL_LENGTH = sizeof(LABEL) - 1;
}  // namespace rdmnetllrponly

class RDMNetLLRPOnly {
public:
	RDMNetLLRPOnly(const char *pLabel = nullptr) :
		m_pLabel(const_cast<char*>(pLabel)),
		m_pRDMPersonality(new RDMPersonality(rdmnetllrponly::LABEL, nullptr)),
		m_RDMNetDevice(&m_pRDMPersonality, 1)
	{
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void Init() {
		if (m_pLabel == nullptr) {
			m_RDMNetDevice.SetLabel(RDM_ROOT_DEVICE, rdmnetllrponly::LABEL, rdmnetllrponly::LABEL_LENGTH);
		} else {
			m_RDMNetDevice.SetLabel(RDM_ROOT_DEVICE, m_pLabel, static_cast<uint8_t>(strlen(m_pLabel)));
		}
		m_RDMNetDevice.Init();
	}

	void Run() {
		m_RDMNetDevice.Run();
	}

	void Print() {
		m_RDMNetDevice.Print();
	}

	RDMNetDevice* GetRDMNetDevice() {
		return &m_RDMNetDevice;
	}

private:
	char *m_pLabel;
	RDMPersonality *m_pRDMPersonality;
	RDMNetDevice m_RDMNetDevice;
};

#endif /* RDMNETLLRPONLY_H_ */
