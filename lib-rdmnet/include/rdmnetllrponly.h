/**
 * @file rdmnetllrponly.h
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

#ifndef RDMNETLLRPONLY_H_
#define RDMNETLLRPONLY_H_

#include "rdmnetdevice.h"
#include "rdmidentify.h"
#include "rdmfactorydefaults.h"

#include "lightsetllrponly.h"

class RDMNetLLRPOnly: public RDMIdentify  {
public:
	RDMNetLLRPOnly(const char *pLabel = nullptr);
	~RDMNetLLRPOnly() override;

	void Init();
	void Start();
	void Stop();

	void Run() {
		m_RDMNetDevice.Run();
	}

	void Print() {
		m_RDMNetDevice.Print();
	}

	void SetMode(TRdmIdentifyMode nMode) override;

	RDMNetDevice *GetRDMNetDevice() {
		return &m_RDMNetDevice;
	}

private:
	char *m_pLabel;
	LightSetLLRPOnly m_LightSetLLRPOnly;
	RDMNetDevice m_RDMNetDevice;
};

#endif /* RDMNETLLRPONLY_H_ */
