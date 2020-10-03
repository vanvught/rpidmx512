/**
 * @file rdmnetdevice.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMNETDEVICE_H_
#define RDMNETDEVICE_H_

#include <stdint.h>

#include "rdmdeviceresponder.h"
#include "llrpdevice.h"

#include "rdmhandler.h"

#include "e131.h"
#include "e131uuid.h"

class RDMNetDevice: public RDMDeviceResponder, public LLRPDevice {
public:
	RDMNetDevice(RDMPersonality *pRDMPersonality);
	~RDMNetDevice() override;

	void Start();
	void Stop();
	void Run();

	void Print();

	void CopyUID(uint8_t *pUID) override;
	void CopyCID(uint8_t *pCID) override;

	uint8_t *LLRPHandleRdmCommand(const uint8_t *pRdmDataNoSC) override;

private:
	RDMHandler *m_RDMHandler;
	struct TRdmMessage *m_pRdmCommand;
	uint8_t m_Cid[E131_CID_LENGTH];
	E131Uuid m_E131Uuid;
};

#endif /* RDMNETDEVICE_H_ */
