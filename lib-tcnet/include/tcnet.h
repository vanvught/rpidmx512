/**
 * @file tcnet.h
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

#ifndef TCNET_H_
#define TCNET_H_

#include <stdint.h>

#include "tcnetpackets.h"

#include "tcnettimecode.h"

enum TTCNetBroadcastPorts {
	TCNET_BROADCAST_PORT_0 = 60000,
	TCNET_BROADCAST_PORT_1 = 60001,
	TCNET_BROADCAST_PORT_2 = 60002
};

enum TTCNETUnicastPort {
	TCNET_UNICAST_PORT = 65023
};

//#define USE_PORT_UNICAST

enum TTCNetLayers {
	TCNET_LAYER_1 = 0,
	TCNET_LAYER_2,
	TCNET_LAYER_3,
	TCNET_LAYER_4,
	TCNET_LAYER_A,
	TCNET_LAYER_B,
	TCNET_LAYER_M,
	TCNET_LAYER_C,
	TCNET_LAYER_UNDEFINED
};

struct TTCNetNodeIP {
	uint32_t nIPAddressLocal;
	uint32_t nIPAddressBroadcast;
};

class TCNet {
public:
	TCNet(TTCNetNodeType tNodeType = TCNET_TYPE_SLAVE);
	~TCNet(void);

	void Start(void);
	void Stop(void);

	void Run(void);

	void Print(void);

	TTCNetNodeType GetNodeType(void) {
		return static_cast<TTCNetNodeType>(m_tOptIn.ManagementHeader.NodeType);
	}

	void SetNodeName(const char *pNodeName);
	const char *GetNodeName(void) {
		return reinterpret_cast<char*>(m_tOptIn.ManagementHeader.NodeName);
	}

	void SetLayer(TTCNetLayers tLayer);
	TTCNetLayers GetLayer(void) {
		return m_tLayer;
	}

	void SetUseTimeCode(bool bUseTimeCode) {
		m_bUseTimeCode = bUseTimeCode;
	}
	bool GetUseTimeCode(void) {
		return m_bUseTimeCode;
	}

	void SetTimeCodeType(TTCNetTimeCodeType tType);
	TTCNetTimeCodeType GetTimeCodeType(void) {
		return m_tTimeCodeType;
	}

	void SetTimeCodeHandler(TCNetTimeCode *pTCNetTimeCode) {
		m_pTCNetTimeCode = pTCNetTimeCode;
	}

public:
	static char GetLayerName(TTCNetLayers tLayer);
	static TTCNetLayers GetLayer(uint8_t nChar);

	static TCNet *Get(void) {
		return s_pThis;
	}

private:
	void HandlePort60000Incoming(void);
	void HandlePort60001Incoming(void);
	void HandlePort60002Incoming(void);
	void HandlePortUnicastIncoming(void);
	void HandleOptInOutgoing(void);

	void DumpManagementHeader(void);
	void DumpOptIn(void);

private:
	struct TTCNetNodeIP m_tNode;
	uint32_t m_aHandles[4];

	struct TTCNetPacketOptIn m_tOptIn;

	struct TTCNet m_TTCNet;

	uint32_t m_nCurrentMillis;
	uint32_t m_nPreviousMillis;

	TTCNetLayers m_tLayer;
	uint32_t *m_pLTime;
	struct TTCNetPacketTimeTimeCode *m_pLTimeCode;
	bool m_bUseTimeCode;

	TCNetTimeCode *m_pTCNetTimeCode;

	float m_fTypeDivider;
	TTCNetTimeCodeType m_tTimeCodeType;

	uint8_t m_nSeqTimeMessage;

	static TCNet *s_pThis;
};

#endif /* TCNET_H_ */
