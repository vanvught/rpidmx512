/**
 * @file oscserver.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "lightset.h"

#define OSCSERVER_DEFAULT_PORT_INCOMING	8000
#define OSCSERVER_DEFAULT_PORT_OUTGOING	9000

class OscServer {
public:
	OscServer(void);
	~OscServer(void);

	void SetOutput(LightSet *);

	uint16_t GetPortIncoming(void) const;
	void SetPortIncoming(uint16_t nPortIncoming);

	uint16_t GetPortOutgoing(void) const;
	void SetPortOutgoing(uint16_t nPortOutgoing);

	void Start(void);
	void Stop(void);

	int Run(void);

private:
	int GetChannel(const char *p);
	const bool IsDmxDataChanged(const uint8_t *pData, uint16_t nStart, uint16_t nLength);

private:
	uint16_t m_nPortIncoming;
	uint16_t m_nPortOutgoing;
	LightSet *m_pLightSet;
	uint8_t *m_pBuffer;
	uint8_t *m_pData;
	uint8_t *m_pOsc;
	bool m_IsBlackout;
};



#endif /* OSCSERVER_H_ */
