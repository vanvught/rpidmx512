/**
 * @file displayudfparams.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAYUDFPARAMS_H_
#define DISPLAYUDFPARAMS_H_

#include <stdint.h>

#include "displayudf.h"

#define DISPLAYUDF_PARAMS_SLEEP_TIMEOUT_DEFAULT		5

struct TDisplayUdfParams {
    uint32_t nSetList;
    uint8_t nLabelIndex[28];
    uint8_t nSleepTimeout;
}__attribute__((packed));

struct DisplayUdfParamsMask {
	static constexpr auto SLEEP_TIMEOUT = (1U << 28);
};

class DisplayUdfParamsStore {
public:
	virtual ~DisplayUdfParamsStore() {}

	virtual void Update(const struct TDisplayUdfParams *ptDisplayUdfParams)=0;
	virtual void Copy(struct TDisplayUdfParams *ptDisplayUdfParams)=0;
};

class DisplayUdfParams {
public:
	DisplayUdfParams(DisplayUdfParamsStore *pDisplayUdfParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDisplayUdfParams *ptDisplayUdfParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(DisplayUdf *pDisplayUdf);

	void Dump();

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tDisplayUdfParams.nSetList & nMask) == nMask;
    }

    void Set();

private:
    DisplayUdfParamsStore *m_pDisplayUdfParamsStore;
    struct TDisplayUdfParams m_tDisplayUdfParams;
};

#endif /* DISPLAYUDFPARAMS_H_ */
