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

class RDMResponder: DMXReceiver, public RDMDeviceResponder, RDMHandler  {
public:
	RDMResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet) : DMXReceiver(pLightSet), RDMDeviceResponder(pRDMPersonality, pLightSet) {
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

	static RDMResponder *Get() {
		return s_pThis;
	}

private:
	int HandleResponse(uint8_t *pResponse);

private:
	static RDMResponder *s_pThis;
	static TRdmMessage s_RdmCommand;
	static bool m_IsSubDeviceActive;
};

#endif /* RDMRESPONDER_H_ */
