/**
 * @file nextionparams.h
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

#ifndef NEXTIONPARAMS_H_
#define NEXTIONPARAMS_H_

#include <stdint.h>

#include "nextion.h"

struct TNextionParams {
    uint32_t nSetList;
	uint32_t nBaud;
	uint32_t nOnBoardCrystal;
};

enum TNextionParamsMask {
	NEXTION_PARAMS_BAUD = (1 << 0),
	NEXTION_PARAMS_CRYSTAL = (1 << 1)
};

class NextionParamsStore {
public:
	virtual ~NextionParamsStore(void);

	virtual void Update(const struct TNextionParams *pNextionParams)=0;
	virtual void Copy(struct TNextionParams *pNextionParams)=0;
};

class NextionParams {
public:
	NextionParams(NextionParamsStore *pNextionParamsStore = 0);
	~NextionParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TNextionParams *pNextionParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(Nextion *pNextion);

	void Dump(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;

private:
	NextionParamsStore *m_pNextionParamsStore;
    struct TNextionParams m_tNextionParams;
};

#endif /* NEXTIONPARAMS_H_ */
