/**
 * @file showfileosc.h
 *
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

#ifndef SHOWFILEOSC_H_
#define SHOWFILEOSC_H_

#include <stdint.h>

#include "showfiledisplay.h"

struct ShowFileOSCPortDefault {
	static constexpr auto INCOMING = 8000;
	static constexpr auto OUTGOING = 9000;
};

struct ShowFileOSCMax {
	static constexpr auto CMD_LENGTH = 128;
	static constexpr auto FILES_ENTRIES = 20;
};

class ShowFileOSC {
public:
	ShowFileOSC(void);
	~ShowFileOSC(void);

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

	void SetPortOutgoing(uint16_t nPortOutgoing) {
		m_nPortOutgoing = nPortOutgoing;
	}
	uint16_t GetPortOutgoing(void) {
		return m_nPortOutgoing;
	}

	static ShowFileOSC *Get(void) {
		return s_pThis;
	}

private:
	void SendStatus(void);
	void Reload(void);

private:
	uint16_t m_nPortIncoming = ShowFileOSCPortDefault::INCOMING;
	uint16_t m_nPortOutgoing = ShowFileOSCPortDefault::OUTGOING;
	int32_t m_nHandle = -1;
	uint32_t m_nRemoteIp = 0;
	uint16_t m_nRemotePort = 0;
	char *m_pBuffer = 0;
	int32_t m_aFileIndex[ShowFileOSCMax::FILES_ENTRIES];

	static ShowFileOSC *s_pThis;
};

#endif /* SHOWFILEOSC_H_ */
