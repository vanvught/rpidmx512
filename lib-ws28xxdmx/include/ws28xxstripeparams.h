/**
 * @file devicesparams.h
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WS28XXSTRIPEPARAMS_H_
#define WS28XXSTRIPEPARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "ws28xxstripedmx.h"
#include "ws28xxstripedmxgrouping.h"

struct TWS28XXStripeParams {
    uint32_t nSetList;
	TWS28XXType tLedType;
	uint16_t nLedCount;
	uint16_t nDmxStartAddress;
	bool bLedGrouping;
};

class WS28XXStripeParamsStore {
public:
	virtual ~WS28XXStripeParamsStore(void);

	virtual void Update(const struct TWS28XXStripeParams *pWS28XXStripeParams)=0;
	virtual void Copy(struct TWS28XXStripeParams *pWS28XXStripeParams)=0;

private:
};

class WS28XXStripeParams {
public:
	WS28XXStripeParams(WS28XXStripeParamsStore *pWS28XXStripeParamsStore = 0);
	~WS28XXStripeParams(void);

	bool Load(void);
	void Set(SPISend *);
	void Dump(void);

	inline TWS28XXType GetLedType(void) {
		return m_tWS28XXStripeParams.tLedType;
	}

	inline uint16_t GetLedCount(void) {
		return m_tWS28XXStripeParams.nLedCount;
	}

	inline uint16_t GetDmxStartAddress(void) {
		return m_tWS28XXStripeParams.nDmxStartAddress;
	}

	inline bool IsLedGrouping(void) {
		return m_tWS28XXStripeParams.bLedGrouping;
	}

public:
	static const char *GetLedTypeString(TWS28XXType);
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const;

private:
    WS28XXStripeParamsStore *m_pWS28XXStripeParamsStore;
    struct TWS28XXStripeParams m_tWS28XXStripeParams;
};

#endif /* WS28XXSTRIPEPARAMS_H_ */
