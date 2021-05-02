/**
 * @file artnetcontroller.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETCONTROLLER_H_
#define ARTNETCONTROLLER_H_

#include <time.h>
#include <stdint.h>

#include "packets.h"
#include "artnettrigger.h"

#include "artnetpolltable.h"

#ifndef DMX_MAX_VALUE
#define DMX_MAX_VALUE 255
#endif

struct TArtNetController {
	uint32_t nIPAddressLocal;
	uint32_t nIPAddressBroadcast;
	uint8_t Oem[2];
};

class ArtNetController: public ArtNetPollTable {
public:
	ArtNetController();
	~ArtNetController();

	void Start();
	void Stop();
	void Run();

	void Print();

	void HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength, uint8_t nPortIndex = 0);
	void HandleSync();
	void HandleBlackout();

	void SetRunTableCleanup(bool bDoTableCleanup) {
		m_bDoTableCleanup = bDoTableCleanup;
	}

	void SetSynchronization(bool bSynchronization) {
		m_bSynchronization = bSynchronization;
	}
	bool GetSynchronization() {
		return m_bSynchronization;
	}

	void SetUnicast(bool bUnicast) {
		m_bUnicast = bUnicast;
	}
	bool GetUnicast() {
		return m_bUnicast;
	}

	void SetMaster(uint32_t nMaster = DMX_MAX_VALUE) {
		if (nMaster < DMX_MAX_VALUE) {
			m_nMaster = nMaster;
		} else {
			m_nMaster = DMX_MAX_VALUE;
		}
	}
	uint32_t GetMaster() {
		return m_nMaster;
	}

	// Handler
	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}
	ArtNetTrigger *GetArtNetTrigger() {
		return m_pArtNetTrigger;
	}

	const uint8_t *GetSoftwareVersion();

private:
	void HandlePoll();
	void HandlePollReply();
	void HandleTrigger();
	void ActiveUniversesAdd(uint16_t nUniverse);
	void ActiveUniversesClear();

private:
	struct TArtNetController m_tArtNetController;
	bool m_bSynchronization{true};
	bool m_bUnicast{true};
	int32_t m_nHandle{-1};
	struct TArtNetPacket *m_pArtNetPacket;
	struct TArtPoll m_ArtNetPoll;
	struct TArtDmx *m_pArtDmx;
	struct TArtSync *m_pArtSync;
	ArtNetTrigger *m_pArtNetTrigger{nullptr}; // Trigger handler
	uint32_t m_nLastPollMillis{0};
	bool m_bDoTableCleanup{true};
	bool m_bDmxHandled{false};
	uint32_t m_nActiveUniverses{0};
	uint32_t m_nMaster{DMX_MAX_VALUE};

public:
	static ArtNetController *Get() {
		return s_pThis;
	}

private:
	static ArtNetController *s_pThis;
};

#endif /* ARTNETCONTROLLER_H_ */
