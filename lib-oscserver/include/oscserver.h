/**
 * @file oscserver.h
 *
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

#ifndef OSCSERVER_H_
#define OSCSERVER_H_

#include <stdint.h>

#include "oscserverhandler.h"
#include "lightset.h"

struct OscServerDefaultPort {
	static constexpr auto INCOMING = 8000;
	static constexpr auto OUTGOING = 9000;
};

struct OscServerMax {
	static constexpr auto PATH_LENGTH = 128;
};

class OscServer {
public:
	OscServer();
	~OscServer();

	void SetOscServerHandler(OscServerHandler *pOscServerHandler);
	void SetOutput(LightSet *pLightSet);

	void SetPortIncoming(uint16_t nPortIncoming = OscServerDefaultPort::INCOMING);
	uint16_t GetPortIncoming() const;

	void SetPortOutgoing(uint16_t nPortOutgoing = OscServerDefaultPort::OUTGOING);
	uint16_t GetPortOutgoing() const;

	void SetPath(const char *pPath);
	const char *GetPath();

	void SetPathInfo(const char *pPathInfo);
	const char *GetPathInfo();

	void SetPathBlackOut(const char *pPathBlackOut);
	const char *GetPathBlackOut();

	void SetPartialTransmission(bool bPartialTransmission = false);
	bool IsPartialTransmission() const;

	void SetEnableNoChangeUpdate(bool bEnableNoChangeUpdate) {
		m_bEnableNoChangeUpdate = bEnableNoChangeUpdate;
	}
	bool GetEnableNoChangeUpdate() {
		return m_bEnableNoChangeUpdate;
	}

	void Print();

	void Start();
	void Stop();

	void Run();

private:
	int GetChannel(const char *p);
	bool IsDmxDataChanged(const uint8_t *pData, uint16_t nStartChannel, uint16_t nLength);

private:
	uint16_t m_nPortIncoming = OscServerDefaultPort::INCOMING;
	uint16_t m_nPortOutgoing = OscServerDefaultPort::OUTGOING;
	int32_t m_nHandle = -1;
	bool m_bPartialTransmission = false;
	bool m_bEnableNoChangeUpdate = false;
	uint16_t m_nLastChannel;
	char m_aPath[OscServerMax::PATH_LENGTH];
	char m_aPathSecond[OscServerMax::PATH_LENGTH];
	char m_aPathInfo[OscServerMax::PATH_LENGTH];
	char m_aPathBlackOut[OscServerMax::PATH_LENGTH];
	OscServerHandler *m_pOscServerHandler = nullptr;
	LightSet *m_pLightSet = nullptr;
	bool m_bIsRunning{false};
	char *m_pBuffer = nullptr;
	uint8_t *m_pData = nullptr;
	uint8_t *m_pOsc = nullptr;
	char m_Os[32];
	const char *m_pModel;
	const char *m_pSoC;
};

#endif /* OSCSERVER_H_ */
