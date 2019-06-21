/**
 * @file tlc59711dmxparams.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef TLC59711DMXPARAMS_H_
#define TLC59711DMXPARAMS_H_

#include <stdint.h>

#include "tlc59711dmx.h"

struct TTLC59711DmxParams {
    uint32_t nSetList;
	TTLC59711Type LedType;
	uint8_t nLedCount;
	uint16_t nDmxStartAddress;
    uint32_t nSpiSpeedHz;
};

enum TTLC59711DmxParamsMask {
	TLC59711DMX_PARAMS_MASK_LED_TYPE = (1 << 0),
	TLC59711DMX_PARAMS_MASK_LED_COUNT = (1 << 1),
	TLC59711DMX_PARAMS_MASK_START_ADDRESS = (1 << 2),
	TLC59711DMX_PARAMS_MASK_SPI_SPEED = (1 << 3)
};

class TLC59711DmxParamsStore {
public:
	virtual ~TLC59711DmxParamsStore(void);

	virtual void Update(const struct TTLC59711DmxParams *pTLC59711DmxParams)=0;
	virtual void Copy(struct TTLC59711DmxParams *pTLC59711DmxParams)=0;
};

class TLC59711DmxParams {
public:
	TLC59711DmxParams(TLC59711DmxParamsStore *pTLC59711ParamsStore = 0);
	~TLC59711DmxParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TTLC59711DmxParams *ptTLC59711Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(TLC59711Dmx *);

	void Dump(void);

	TTLC59711Type GetLedType(void) {
		return m_tTLC59711Params.LedType;
	}

	uint16_t GetLedCount(void) {
		return m_tTLC59711Params.nLedCount;
	}

	bool IsSetLedType(void) const;
	bool IsSetLedCount(void) const;

public:
	static const char *GetLedTypeString(TTLC59711Type tTTLC59711Type);
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;

private:
	TLC59711DmxParamsStore *m_pLC59711ParamsStore;
    struct TTLC59711DmxParams m_tTLC59711Params;
};

#endif /* TLC59711DMXPARAMS_H_ */
