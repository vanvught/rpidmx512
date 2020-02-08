/**
 * @file displayudfparams.h
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

#ifndef DISPLAYUDFPARAMS_H_
#define DISPLAYUDFPARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "displayudf.h"

#define DISPLAYUDF_PARAMS_SLEEP_TIMEOUT_DEFAULT		5

struct TDisplayUdfParams {
    uint32_t nSetList;
    uint8_t nLabelIndex[28];
    uint8_t nSleepTimeout;
}__attribute__((packed));

enum TDisplayUdfParamsMask {
	DISPLAY_UDF_PARAMS_MASK_LABEL_TITLE = (1 << DISPLAY_UDF_LABEL_TITLE),
	DISPLAY_UDF_PARAMS_MASK_LABEL_BOARDNAME = (1 << DISPLAY_UDF_LABEL_BOARDNAME),
	DISPLAY_UDF_PARAMS_MASK_LABEL_IP = (1 << DISPLAY_UDF_LABEL_IP),
	DISPLAY_UDF_PARAMS_MASK_LABEL_VERSION = (1 << DISPLAY_UDF_LABEL_VERSION),
	DISPLAY_UDF_PARAMS_MASK_LABEL_UNIVERSE = (1 << DISPLAY_UDF_LABEL_UNIVERSE),
	DISPLAY_UDF_PARAMS_MASK_LABEL_AP = (1 << DISPLAY_UDF_LABEL_AP),
	DISPLAY_UDF_PARAMS_MASK_LABEL_NODE_NAME = (1 << DISPLAY_UDF_LABEL_NODE_NAME),
	DISPLAY_UDF_PARAMS_MASK_LABEL_NETMASK = (1 << DISPLAY_UDF_LABEL_NETMASK),
	DISPLAY_UDF_PARAMS_MASK_LABEL_DMX_START_ADDRESS = (1 << DISPLAY_UDF_LABEL_DMX_START_ADDRESS),
	DISPLAY_UDF_PARAMS_MASK_SLEEP_TIMEOUT = (1 << 28)
}; // Not used

class DisplayUdfParamsStore {
public:
	virtual ~DisplayUdfParamsStore(void) {}

	virtual void Update(const struct TDisplayUdfParams *ptDisplayUdfParams)=0;
	virtual void Copy(struct TDisplayUdfParams *ptDisplayUdfParams)=0;
};

class DisplayUdfParams {
public:
	DisplayUdfParams(DisplayUdfParamsStore *pDisplayUdfParamsStore = 0);
	~DisplayUdfParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDisplayUdfParams *ptDisplayUdfParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(DisplayUdf *pDisplayUdf);

	void Dump(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) {
    	return (m_tDisplayUdfParams.nSetList & nMask) == nMask;
    }

    void Set(void);

private:
    DisplayUdfParamsStore *m_pDisplayUdfParamsStore;
    struct TDisplayUdfParams m_tDisplayUdfParams;
};

#endif /* DISPLAYUDFPARAMS_H_ */
