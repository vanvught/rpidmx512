/**
 * @file showfileprotocolhandler.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SHOWFILEPROTOCOLHANDLER_H_
#define SHOWFILEPROTOCOLHANDLER_H_

#include <stdint.h>

class ShowFileProtocolHandler {
public:
	virtual ~ShowFileProtocolHandler() {
	}

	virtual void DmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength)=0;
	virtual void DmxSync()=0;
	virtual void DmxBlackout()=0;
	virtual void DmxMaster(uint32_t nMaster)=0;

	virtual void DoRunCleanupProcess(bool bDoRun)=0;

	virtual void Start()=0;
	virtual void Stop()=0;
	virtual void Run()=0;

	virtual bool IsSyncDisabled()=0;

	virtual void Print()=0;
};

#endif /* SHOWFILEPROTOCOLHANDLER_H_ */
