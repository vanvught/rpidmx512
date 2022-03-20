/**
 * @file rdmresponder.h
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMRESPONDER_H_
#define RDMRESPONDER_H_

#include <cstdint>
#include <cassert>

#include "dmxreceiver.h"

#include "rdmhandler.h"
#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "lightset.h"

namespace rdm {
namespace responder {
static constexpr auto NO_DATA = 0;
static constexpr auto DISCOVERY_RESPONSE = -1;
static constexpr auto INVALID_DATA_RECEIVED = -2;
static constexpr auto INVALID_RESPONSE = -3;
}  // namespace responder
}  // namespace rdm

class RDMResponder final : DMXReceiver, public RDMDeviceResponder, RDMHandler {
public:
	RDMResponder(RDMPersonality **pRDMPersonalities, uint32_t nPersonalityCount) :
			DMXReceiver(pRDMPersonalities[rdm::device::responder::DEFAULT_CURRENT_PERSONALITY - 1]->GetLightSet()),
			RDMDeviceResponder(pRDMPersonalities, nPersonalityCount)
	{
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	~RDMResponder() {}

	void Init() {
		RDMDeviceResponder::Init();
		// There is no DMXReceiver::Init()
	}

	int Run();

	void Print() {
		RDMDeviceResponder::Print();
		DMXReceiver::Print();
	}

	void Start() {
		// There is no RDMDeviceResponder::Start()
		DMXReceiver::Start();
	}

	void DmxDisableOutput(bool bDisable) {
		DMXReceiver::SetDisableOutput(bDisable);
	}

	uint16_t GetDmxStartAddress(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		return RDMDeviceResponder::GetDmxStartAddress(nSubDevice);
	}

	uint16_t GetDmxFootPrint(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		return RDMDeviceResponder::GetDmxFootPrint(nSubDevice);
	}

	static RDMResponder* Get() {
		return s_pThis;
	}

	void PersonalityUpdate(uint32_t nPersonality) __attribute__((weak));
	void DmxStartAddressUpdate(uint16_t nDmxStartAddress) __attribute__((weak));

private:
	int HandleResponse(uint8_t *pResponse);

	void PersonalityUpdate(LightSet *pLightSet) override {
		DMXReceiver::SetLightSet(pLightSet);
		PersonalityUpdate(static_cast<uint32_t>(RDMDeviceResponder::GetPersonalityCurrent(RDM_ROOT_DEVICE)));
	}

	void DmxStartAddressUpdate() override {
		DmxStartAddressUpdate(RDMDeviceResponder::GetDmxStartAddress(RDM_ROOT_DEVICE));
	}

private:
	static RDMResponder *s_pThis;
	static TRdmMessage s_RdmCommand;
	static bool m_IsSubDeviceActive;
};

#endif /* RDMRESPONDER_H_ */
