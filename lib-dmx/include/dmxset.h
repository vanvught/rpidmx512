/**
 * @file dmxset.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXSET_H_
#define DMXSET_H_

#include <cstdint>

namespace dmx {
enum class PortDirection {
	OUTP,
	INP
};
}  // namespace dmx

class DmxSet {
public:
	DmxSet();
	virtual ~DmxSet() {}

	virtual void SetPortDirection(uint32_t nPort, dmx::PortDirection tPortDirection, bool bEnableData)=0;

	virtual void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength)=0;

	virtual const uint8_t *RdmReceive(uint32_t nPort)=0;
	virtual const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut)=0;

	virtual uint32_t RdmGetDateReceivedEnd()=0;

	static DmxSet* Get() {
		return s_pThis;
	}

private:
	static DmxSet *s_pThis;
};

#endif /* IDMXSET_H_ */
