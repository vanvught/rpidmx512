/**
 * @file artnetcontroller.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdint>

#include "artnet.h"
#include "artnettrigger.h"

#include "artnetpolltable.h"

#ifndef DMX_MAX_VALUE
#define DMX_MAX_VALUE 255
#endif

struct State {
	uint32_t ArtPollIpAddress;
	uint32_t ArtPollReplyCount;
	uint32_t ArtPollReplyDelayMillis;
	artnet::ArtPollQueue ArtPollReplyQueue[4];
	artnet::ReportCode reportCode;
	artnet::Status status;
};

struct TArtNetController {
	uint32_t nIPAddressLocal;
	uint32_t nIPAddressBroadcast;
	uint8_t Oem[2];
};

class ArtNetController: public ArtNetPollTable {
public:
	ArtNetController();
	~ArtNetController();

	void GetShortNameDefault(char *pShortName);
	void SetShortName(const char *pShortName);

	void GetLongNameDefault(char *pLongName);
	void SetLongName(const char *pLongName);

	void Start();
	void Stop();
	void Run();

	void Print();

	void HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength, uint8_t nPortIndex = 0);
	void HandleSync();
	void HandleBlackout();

	void SetRunTableCleanup(const bool bDoTableCleanup) {
		m_bDoTableCleanup = bDoTableCleanup;
	}

	void SetSynchronization(const bool bSynchronization) {
		m_bSynchronization = bSynchronization;
	}
	bool GetSynchronization() const {
		return m_bSynchronization;
	}

	void SetUnicast(const bool bUnicast) {
		m_bUnicast = bUnicast;
	}
	bool GetUnicast() const {
		return m_bUnicast;
	}

	void SetForceBroadcast(const bool bForceBroadcast) {
		m_bForceBroadcast = bForceBroadcast;
	}
	bool GetForceBroadcast() const {
		return m_bForceBroadcast;
	}

#ifdef CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER
	void SetMaster(uint32_t nMaster = DMX_MAX_VALUE) {
		if (nMaster < DMX_MAX_VALUE) {
			m_nMaster = nMaster;
		} else {
			m_nMaster = DMX_MAX_VALUE;
		}
	}
	uint32_t GetMaster() const {
		return m_nMaster;
	}
#endif

	// Handler
	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}
	ArtNetTrigger *GetArtNetTrigger() const {
		return m_pArtNetTrigger;
	}

	const uint8_t *GetSoftwareVersion();

	static ArtNetController *Get() {
		return s_pThis;
	}

private:
	void ProcessPoll();
	void HandlePoll();
	void HandlePollReply();
	void HandleTrigger();
	void ActiveUniversesAdd(uint16_t nUniverse);
	void ActiveUniversesClear();

private:
	TArtNetController m_ArtNetController;
	State m_State;

	struct TArtNetPacket {
		union artnet::UArtPacket ArtPacket;
		uint32_t IPAddressFrom;
		uint16_t nLength;
		artnet::OpCodes OpCode;
	};

	artnet::ArtPoll m_ArtNetPoll;
	artnet::ArtPollReply m_ArtPollReply;

	TArtNetPacket *m_pArtNetPacket;

	artnet::ArtDmx *m_pArtDmx;
	artnet::ArtSync *m_pArtSync;
	ArtNetTrigger *m_pArtNetTrigger { nullptr }; // Trigger handler

	bool m_bSynchronization { true };
	bool m_bUnicast { true };
	bool m_bForceBroadcast { false };
	bool m_bDoTableCleanup { true };
	bool m_bDmxHandled { false };

	int32_t m_nHandle { -1 };
	uint32_t m_nLastPollMillis { 0 };
	uint32_t m_nActiveUniverses { 0 };
#ifdef CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER
	uint32_t m_nMaster { DMX_MAX_VALUE };
#endif
	static ArtNetController *s_pThis;
};

#endif /* ARTNETCONTROLLER_H_ */
