/**
 * @file oscserver.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define OSCSERVER_DEFAULT_PORT_INCOMING	8000
#define OSCSERVER_DEFAULT_PORT_OUTGOING	9000

#define OSCSERVER_PATH_LENGTH_MAX	128

class OscServer {
public:
	OscServer(void);
	~OscServer(void);

	void SetOscServerHandler(OscServerHandler *pOscServerHandler);
	void SetOutput(LightSet *pLightSet);

	void SetPortIncoming(uint16_t nPortIncoming = OSCSERVER_DEFAULT_PORT_INCOMING);
	uint16_t GetPortIncoming(void) const;

	void SetPortOutgoing(uint16_t nPortOutgoing = OSCSERVER_DEFAULT_PORT_OUTGOING);
	uint16_t GetPortOutgoing(void) const;

	void SetPath(const char *pPath);
	const char *GetPath(void);

	void SetPathInfo(const char *pPathInfo);
	const char *GetPathInfo(void);

	void SetPathBlackOut(const char *pPathBlackOut);
	const char *GetPathBlackOut(void);

	void SetPartialTransmission(bool bPartialTransmission = false);
	bool IsPartialTransmission(void) const;

	void Print(void);

	void Start(void);
	void Stop(void);

	int Run(void);

private:
	int GetChannel(const char *p);
	bool IsDmxDataChanged(const uint8_t *pData, uint16_t nStartChannel, uint16_t nLength);

private:
	uint16_t m_nPortIncoming;
	uint16_t m_nPortOutgoing;
	int32_t m_nHandle;
	bool m_bPartialTransmission;
	uint16_t m_nLastChannel;
	char m_aPath[OSCSERVER_PATH_LENGTH_MAX];
	char m_aPathSecond[OSCSERVER_PATH_LENGTH_MAX];
	char m_aPathInfo[OSCSERVER_PATH_LENGTH_MAX];
	char m_aPathBlackOut[OSCSERVER_PATH_LENGTH_MAX];
	OscServerHandler *m_pOscServerHandler;
	LightSet *m_pLightSet;
	uint8_t *m_pBuffer;
	uint8_t *m_pData;
	uint8_t *m_pOsc;
	char m_Os[32];
	const char *m_pModel;
	const char *m_pSoC;
};

#endif /* OSCSERVER_H_ */
