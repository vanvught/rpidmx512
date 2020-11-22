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

enum class TCNetLayer {
	LAYER_1,
	LAYER_2,
	LAYER_3,
	LAYER_4,
	LAYER_A,
	LAYER_B,
	LAYER_M,
	LAYER_C,
	LAYER_UNDEFINED
};

class TCNet {
public:
	TCNet(TTCNetNodeType tNodeType = TCNET_TYPE_SLAVE);
	~TCNet();

	void Start();
	void Stop();

	void Run();

	void Print();

	TTCNetNodeType GetNodeType() {
		return static_cast<TTCNetNodeType>(m_tOptIn.ManagementHeader.NodeType);
	}

	void SetNodeName(const char *pNodeName);
	const char *GetNodeName() {
		return reinterpret_cast<char*>(m_tOptIn.ManagementHeader.NodeName);
	}

	void SetLayer(TCNetLayer tLayer);
	TCNetLayer GetLayer() {
		return m_tLayer;
	}

	void SetUseTimeCode(bool bUseTimeCode) {
		m_bUseTimeCode = bUseTimeCode;
	}
	bool GetUseTimeCode() {
		return m_bUseTimeCode;
	}

	void SetTimeCodeType(TTCNetTimeCodeType tType);
	TTCNetTimeCodeType GetTimeCodeType() {
		return m_tTimeCodeType;
	}

	void SetTimeCodeHandler(TCNetTimeCode *pTCNetTimeCode) {
		m_pTCNetTimeCode = pTCNetTimeCode;
	}

public:
	static char GetLayerName(TCNetLayer tLayer);
	static TCNetLayer GetLayer(char nChar);

	static TCNet *Get() {
		return s_pThis;
	}

private:
	void HandlePort60000Incoming();
	void HandlePort60001Incoming();
	void HandlePort60002Incoming();
	void HandlePortUnicastIncoming();
	void HandleOptInOutgoing();

	void DumpManagementHeader();
	void DumpOptIn();

private:
	struct TCNetBroadcast {
		static constexpr auto PORT_0 = 60000;
		static constexpr auto PORT_1 = 60001;
		static constexpr auto PORT_2 = 60002;
	};
	struct TCNETUnicast {
		static constexpr auto PORT = 65023;
	};
	struct TTCNetNodeIP {
		uint32_t nIPAddressLocal;
		uint32_t nIPAddressBroadcast;
	};
	TTCNetNodeIP m_tNode;
	int32_t m_aHandles[4];
	TTCNetPacketOptIn m_tOptIn;
	TTCNet m_TTCNet;
	uint32_t m_nCurrentMillis{0};
	uint32_t m_nPreviousMillis{0};
	TCNetLayer m_tLayer = TCNetLayer::LAYER_M;
	uint32_t *m_pLTime{nullptr};
	TTCNetPacketTimeTimeCode *m_pLTimeCode{nullptr};
	bool m_bUseTimeCode = false;
	TCNetTimeCode *m_pTCNetTimeCode{nullptr};
	float m_fTypeDivider{1000.0F / 30};
	TTCNetTimeCodeType m_tTimeCodeType;
	uint8_t m_nSeqTimeMessage{0};

	static TCNet *s_pThis;
};

#endif /* TCNET_H_ */
