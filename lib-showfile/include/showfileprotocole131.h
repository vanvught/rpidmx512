/**
 * @file showfileprotocole131.h
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

#ifndef SHOWFILEPROTOCOLE131_H_
#define SHOWFILEPROTOCOLE131_H_

#include <stdint.h>
#include <stdio.h>

#include "e131controller.h"

#include "showfileprotocolhandler.h"

class ShowFileProtocolE131: public ShowFileProtocolHandler {
public:
	ShowFileProtocolE131() {
	}
	~ShowFileProtocolE131() {
		m_E131Controller.Stop();
	}

	void SetSynchronizationAddress(uint16_t SynchronizationAddress) {
		m_E131Controller.SetSynchronizationAddress(SynchronizationAddress);
	}

	void Start() {
		m_E131Controller.Start();
	}

	void Stop() {
		m_E131Controller.Stop();
	}

	void DmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength) {
		m_E131Controller.HandleDmxOut(nUniverse, pDmxData, nLength);
	}

	void DmxSync() {
		m_E131Controller.HandleSync();
	}

	void DmxBlackout() {
		m_E131Controller.HandleBlackout();
	}

	void DmxMaster(uint32_t nMaster) {
		m_E131Controller.SetMaster(nMaster);
	}

	void DoRunCleanupProcess(__attribute__((unused)) bool bDoRun) {}

	void Run() {
		m_E131Controller.Run();
	}

	bool IsSyncDisabled() {
		return (m_E131Controller.GetSynchronizationAddress() == 0);
	}

	void Print() {
		puts("ShowFileProtocolE131");
		m_E131Controller.Print();
	}

private:
	E131Controller m_E131Controller;
};

#endif /* SHOWFILEPROTOCOLE131_H_ */
