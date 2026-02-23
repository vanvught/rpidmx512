/**
 * @file showfileprotocolartnet.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLARTNET_H_
#define PROTOCOLS_SHOWFILEPROTOCOLARTNET_H_

#include <cstdint>
#include <cstdio>

#include "artnetcontroller.h"

 #include "firmware/debug/debug_debug.h"

class ShowFileProtocol {
public:
	ShowFileProtocol() {
		DEBUG_ENTRY();

		DEBUG_EXIT();
	}

	~ShowFileProtocol() {
		m_ArtNetController.Stop();
	}

	void Start() {
		DEBUG_ENTRY();

		m_ArtNetController.Start();

		DEBUG_EXIT();
	}

	void Stop() {
		DEBUG_ENTRY();
	
		m_ArtNetController.Stop();

		DEBUG_EXIT();
	}

	void Record() {
		DEBUG_ENTRY();

		DEBUG_EXIT();
	}

	void DmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength) {
		m_ArtNetController.HandleDmxOut(nUniverse, pDmxData, nLength);
	}

	void DmxSync() {
		m_ArtNetController.HandleSync();
	}

	void DmxBlackout() {
		m_ArtNetController.HandleBlackout();
	}

	void DmxMaster([[maybe_unused]] const uint32_t nMaster) {
#if defined(CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER)
		m_ArtNetController.SetMaster(nMaster);
#endif
	}

	void DoRunCleanupProcess(const bool bDoRun) {
		m_ArtNetController.SetRunTableCleanup(bDoRun);
	}

	void Run() {
		m_ArtNetController.Run();
	}

	bool IsSyncDisabled() {
		return !m_ArtNetController.GetSynchronization();
	}

	void Print() {
		m_ArtNetController.Print();
	}

private:
	ArtNetController m_ArtNetController;
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLARTNET_H_ */
