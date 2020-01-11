/**
 * @file oscserver.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ltcdisplayws28xx.h"

#define OSCSERVER_PATH_LENGTH_MAX	128

class OSCServer {
public:
	OSCServer(void);
	~OSCServer(void);

	void Start(void);
	void Stop(void);
	void Run(void);

	void Print(void);

	void SetPortIncoming(uint16_t nPortIncoming) {
		m_nPortIncoming = nPortIncoming;
	}
	uint16_t GetPortIncoming(void) {
		return m_nPortIncoming;
	}

private:
	void SetWS28xxRGB(uint32_t nSize, TLtcDisplayWS28xxColourIndex tIndex);

private:
	uint16_t m_nPortIncoming;
	int32_t m_nHandle;
	uint32_t m_nRemoteIp;
	uint16_t m_nRemotePort;
	char m_aPath[OSCSERVER_PATH_LENGTH_MAX];
	uint32_t m_nPathLength;
	uint8_t *m_pBuffer;
};

#endif /* OSCSERVER_H_ */
