/**
 * @file showfileprotocole131.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLE131_H_
#define PROTOCOLS_SHOWFILEPROTOCOLE131_H_

#include <cstdint>
#include <cstdio>

#include "e131controller.h"

#include "debug.h"

class ShowFileProtocol {
public:
	ShowFileProtocol() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void SetSynchronizationAddress(const uint16_t SynchronizationAddress) {
		m_E131Controller.SetSynchronizationAddress(SynchronizationAddress);
	}

	void Start() {
		DEBUG_ENTRY

		m_E131Controller.Start();

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		m_E131Controller.Stop();

		DEBUG_EXIT
	}

	void Record() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void DmxOut(const uint16_t nUniverse, const uint8_t *pDmxData, const uint32_t nLength) {
		m_E131Controller.HandleDmxOut(nUniverse, pDmxData, nLength);
	}

	void DmxSync() {
		m_E131Controller.HandleSync();
	}

	void DmxBlackout() {
		m_E131Controller.HandleBlackout();
	}

	void DmxMaster(const uint32_t nMaster) {
		m_E131Controller.SetMaster(nMaster);
	}

	void DoRunCleanupProcess([[maybe_unused]] bool bDoRun) {
	}

	void Run() {
		m_E131Controller.Run();
	}

	bool IsSyncDisabled() {
		return (m_E131Controller.GetSynchronizationAddress() == 0);
	}

	void Print() {
		m_E131Controller.Print();
	}

private:
	E131Controller m_E131Controller;
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLE131_H_ */
