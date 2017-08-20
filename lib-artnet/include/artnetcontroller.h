/**
 * @file artnetcontroller.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
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

#ifndef ARTNETCONTROLLER_H_
#define ARTNETCONTROLLER_H_

#include <time.h>

#include "packets.h"

#include "artnetpolltable.h"
#include "artnetipprog.h"

class ArtNetController: public ArtNetPollTable {
public:
	ArtNetController(void);
	~ArtNetController(void);

	void Start(void);
	void Stop(void);

	void SetPollInterval(const uint8_t);
	const uint8_t GetPollInterval(void);

	int Run(void);

	void SendIpProg(const uint32_t, const struct TArtNetIpProg *);

private:
	void SendPoll(void);
	void HandlePollReply(void);
	void SendIpProg(void);
	void HandleIpProgReply(void);

private:
	struct TArtNetPacket	*m_pArtNetPacket;
	struct TArtPoll			m_ArtNetPoll;
	struct TArtIpProg 		m_ArtIpProg;
	time_t 					m_nLastPollTime;
	uint32_t				m_IPAddressLocal;
	uint32_t				m_IPAddressBroadcast;
	uint8_t					m_nPollInterVal;
};

#endif /* ARTNETCONTROLLER_H_ */
