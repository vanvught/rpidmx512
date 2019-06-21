/**
 * @file ltcparams.h
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

#ifndef LTCPARAMS_H_
#define LTCPARAMS_H_

#include <stdint.h>

#include "ltc.h"
#include "displaymax7219.h"

enum TLtcReaderSource {
	LTC_READER_SOURCE_LTC,
	LTC_READER_SOURCE_ARTNET,
	LTC_READER_SOURCE_MIDI,
	LTC_READER_SOURCE_TCNET,
	LTC_READER_SOURCE_UNDEFINED
};

enum TLtcParamsMax7219Type {
	LTC_PARAMS_MAX7219_TYPE_MATRIX,
	LTC_PARAMS_MAX7219_TYPE_7SEGMENT
};

enum TLtcParamsMaskDisabledOutputs { //FIXEM refactor with _MASK
	LTC_PARAMS_DISABLE_DISPLAY = (1 << 0),
	LTC_PARAMS_DISABLE_MAX7219 = (1 << 1),
	LTC_PARAMS_DISABLE_MIDI = (1 << 2),
	LTC_PARAMS_DISABLE_ARTNET = (1 << 3),
	LTC_PARAMS_DISABLE_TCNET = (1 << 4),
	LTC_PARAMS_DISABLE_LTC = (1 << 5)
};

struct TLtcParams {
	uint32_t nSetList;
	uint8_t tSource;
	uint8_t tMax7219Type;
	uint8_t nMax7219Intensity;
	uint8_t nDisabledOutputs;
	uint8_t nShowSysTime;
	uint8_t nDisableTimeSync;
};

enum TLtcParamsMask {
	LTC_PARAMS_MASK_SOURCE = (1 << 0),
	LTC_PARAMS_MASK_MAX7219_TYPE = (1 << 1),
	LTC_PARAMS_MASK_MAX7219_INTENSITY = (1 << 2),
	LTC_PARAMS_MASK_DISABLED_OUTPUTS = (1 << 3),
	LTC_PARAMS_MASK_SHOW_SYSTIME = (1 << 4),
	LTC_PARAMS_MASK_DISABLE_TIMESYNC = (1 << 5)
};

class LtcParamsStore {
public:
	virtual ~LtcParamsStore(void);

	virtual void Update(const struct TLtcParams *pTLtcParams)=0;
	virtual void Copy(struct TLtcParams *pTLtcParams)=0;
};

class LtcParams {
public:
	LtcParams(LtcParamsStore *pLtcParamsStore = 0);
	~LtcParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TLtcParams *ptLtcParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	TLtcReaderSource GetSource(void) {
		return (TLtcReaderSource) m_tLtcParams.tSource;
	}

	const char *GetSourceType(enum TLtcReaderSource tSource);
	enum TLtcReaderSource GetSourceType(const char *pType);

	uint8_t GetMax7219Intensity(void) {
		return m_tLtcParams.nMax7219Intensity;
	}

	tMax7219Types GetMax7219Type(void) {
		return (tMax7219Types) m_tLtcParams.tMax7219Type;
	}

	void CopyDisabledOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs);

	bool IsShowSysTime(void) {
		return (m_tLtcParams.nShowSysTime != 0);
	}

	bool IsTimeSyncDisabled(void) {
		return  (m_tLtcParams.nDisableTimeSync == 1);
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const;
	bool isDisabledOutputMaskSet(uint8_t nMask) const;

private:
    LtcParamsStore 	*m_pLTcParamsStore;
    struct TLtcParams m_tLtcParams;
};

#endif /* LTCPARAMS_H_ */
