/**
 * @file artnetrdmresponder.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ARTNETRDMRESPONDER_H_
#define ARTNETRDMRESPONDER_H_

#include <stdint.h>

#include "artnetrdm.h"

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmhandler.h"
#include "rdm.h"

#include "lightset.h"

class ArtNetRdmResponder: public ArtNetRdm {
public:
	ArtNetRdmResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet);
	~ArtNetRdmResponder(void);

	void Full(uint8_t nPort);
	const uint8_t GetUidCount(uint8_t nPort);
	void Copy(uint8_t nPort, uint8_t *);
	const uint8_t *Handler(uint8_t nPort, const uint8_t *);

	inline RDMDeviceResponder *GetRDMDeviceResponder(void) {
		return &m_Responder;
	}

private:
	RDMDeviceResponder m_Responder;
	struct TRdmMessage *m_pRdmCommand;
	RDMHandler *m_RDMHandler;
};

#endif /* ARTNETRDMRESPONDER_H_ */
