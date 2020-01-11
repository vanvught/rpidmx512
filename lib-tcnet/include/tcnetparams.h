/**
 * @file tcnetparams.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef TCNETPARAMS_H_
#define TCNETPARAMS_H_

#include <stdint.h>

#include "tcnet.h"
#include "tcnetpackets.h"

struct TTCNetParams {
	uint32_t nSetList;
	uint8_t aNodeName[TCNET_NODE_NAME_LENGTH];
	uint8_t nLayer;
	uint8_t nTimeCodeType;
	uint8_t nUseTimeCode;
};

enum TTCNetParamsMask {
	TCNET_PARAMS_MASK_NODE_NAME = (1 << 0),
	TCNET_PARAMS_MASK_LAYER = (1 << 1),
	TCNET_PARAMS_MASK_TIMECODE_TYPE = (1 << 2),
	TCNET_PARAMS_MASK_USE_TIMECODE = (1 << 3)
};

class TCNetParamsStore {
public:
	virtual ~TCNetParamsStore(void) {}

	virtual void Update(const struct TTCNetParams *pTCNetParams)=0;
	virtual void Copy(struct TTCNetParams *pTCNetParams)=0;
};

class TCNetParams {
public:
	TCNetParams(TCNetParamsStore *pTCNetParamsStore = 0);
	~TCNetParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TTCNetParams *pTTCNetParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(TCNet *pTCNet);

	void Dump(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) {
		return (m_tTTCNetParams.nSetList & nMask) == nMask;
	}

private:
	TCNetParamsStore *m_pTCNetParamsStore;
    struct TTCNetParams	m_tTTCNetParams;
};

#endif /* TCNETPARAMS_H_ */
