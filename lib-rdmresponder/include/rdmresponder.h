/**
 * @file rdmresponder.h
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

#ifndef RDMRESPONDER_H_
#define RDMRESPONDER_H_

#include <stdint.h>

#include "dmxreceiver.h"
#include "gpio.h"

#include "rdmhandler.h"
#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"

#include "lightset.h"

#define RDM_RESPONDER_NO_DATA				0
#define RDM_RESPONDER_DISCOVERY_RESPONSE	-1
#define RDM_RESPONDER_INVALID_DATA_RECEIVED	-2
#define RDM_RESPONDER_INVALID_RESPONSE		-3

class RDMResponder: public DMXReceiver, public RDMDeviceResponder  {
public:
	RDMResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet, uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION);
	~RDMResponder(void);

	void Init(void);

	int Run(void);

	void Print(void);

private:
	int HandleResponse(uint8_t *pResponse);

private:
	struct TRdmMessage *m_pRdmCommand;
	RDMHandler *m_RDMHandler;
	bool m_IsSubDeviceActive;
};

#endif /* RDMRESPONDER_H_ */
