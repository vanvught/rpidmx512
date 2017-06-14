/**
 * @file rdmmessage.h
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

#ifndef RDMMESSAGE_H_
#define RDMMESSAGE_H_

#include "rdm.h"

class RDMMessage {
public:
	RDMMessage(void);
	~RDMMessage(void);

	void SetSrcUid(const uint8_t *);
	void SetDstUid(const uint8_t *);

	void SetCc(const uint8_t);
	void SetPid(const uint16_t);

	void SetSubDevice(const uint16_t);

	void SetPd(const uint8_t *, const uint8_t);

	void Send(void);

	static void SendRaw(const uint8_t *, const uint8_t);
	static const uint8_t *Receive(void);
	static const uint8_t *ReceiveTimeOut(const uint32_t);

	static void Print(const uint8_t *);

public:
	static uint8_t m_TransactionNumber;

private:
	_rdm_command *m_pRdmCommand;
};

#endif /* RDMMESSAGE_H_ */
