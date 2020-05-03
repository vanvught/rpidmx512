/**
 * @file showfileprotocolartnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#ifndef SHOWFILEPROTOCOLARTNET_H_
#define SHOWFILEPROTOCOLARTNET_H_

#include <stdint.h>
#include <stdio.h>

#include "artnetcontroller.h"
#include "../../lib-artnet/include/artnettrigger.h"

#include "showfileprotocolhandler.h"

class ShowFileProtocolArtNet: public ShowFileProtocolHandler, ArtNetTrigger {
public:
	ShowFileProtocolArtNet(void) {
		m_ArtNetController.SetArtNetTrigger(static_cast<ArtNetTrigger *>(this));
	}

	~ShowFileProtocolArtNet(void) {
		m_ArtNetController.Stop();
	}

	void Start(void) {
		m_ArtNetController.Start();
	}

	void Stop(void) {
		m_ArtNetController.Stop();
	}

	void DmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength) {
		m_ArtNetController.HandleDmxOut(nUniverse, pDmxData, nLength);
	}

	void DmxSync(void) {
		m_ArtNetController.HandleSync();
	}

	void DmxBlackout(void) {
		m_ArtNetController.HandleBlackout();
	}

	void DmxMaster(uint32_t nMaster) {
		m_ArtNetController.SetMaster(nMaster);
	}

	void DoRunCleanupProcess(bool bDoRun) {
		m_ArtNetController.SetRunTableCleanup(bDoRun);
	}

	void Run(void) {
		m_ArtNetController.Run();
	}

	bool IsSyncDisabled(void) {
		return !m_ArtNetController.GetSynchronization();
	}

	void Print(void) {
		puts("ShowFileProtocolArtNet");
		m_ArtNetController.Print();
	}

	// ArtNetTrigger
	void Handler(const struct TArtNetTrigger *ptArtNetTrigger);

private:
	ArtNetController m_ArtNetController;
};

#endif /* SHOWFILEPROTOCOLARTNET_H_ */
