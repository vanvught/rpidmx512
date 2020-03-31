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

struct TArtNetController {
	uint32_t nIPAddressLocal;
	uint32_t nIPAddressBroadcast;
	uint8_t Oem[2];
};

class ArtNetController: public ArtNetPollTable {
public:
	ArtNetController(void);
	~ArtNetController(void);

	void Start(void);
	void Stop(void);
	void Run(void);

	void Print(void);

	void HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength, uint8_t nPortIndex = 0);
	void HandleSync(void);
	void HandleBlackout(void);

	void SetRunTableCleanup(bool bDoTableCleanup) {
		m_bDoTableCleanup = bDoTableCleanup;
	}

	void SetSynchronization(bool bSynchronization) {
		m_bSynchronization = bSynchronization;
	}
	bool GetSynchronization(void) {
		return m_bSynchronization;
	}

	void SetUnicast(bool bUnicast) {
		m_bUnicast = bUnicast;
	}
	bool GetUnicast(void) {
		return m_bUnicast;
	}

	// Handler
	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}
	ArtNetTrigger *GetArtNetTrigger(void) {
		return m_pArtNetTrigger;
	}

	const uint8_t *GetSoftwareVersion(void);

private:
	void HandlePoll(void);
	void HandlePollReply(void);
	void HandleTrigger(void);
	void ActiveUniversesAdd(uint16_t nUniverse);
	void ActiveUniversesClear(void);

private:
	struct TArtNetController m_tArtNetController;
	bool m_bSynchronization;
	bool m_bUnicast;
	int32_t m_nHandle;
	struct TArtNetPacket *m_pArtNetPacket;
	struct TArtPoll m_ArtNetPoll;
	struct TArtDmx *m_pArtDmx;
	struct TArtSync *m_pArtSync;
	ArtNetTrigger *m_pArtNetTrigger; // Trigger handler
	uint32_t m_nLastPollMillis;
	bool m_bDoTableCleanup;
	bool m_bDmxHandled;
	uint32_t m_nActiveUniverses;

public:
	static ArtNetController *Get(void) {
		return s_pThis;
	}

private:
	static ArtNetController *s_pThis;
};

#endif /* ARTNETCONTROLLER_H_ */
