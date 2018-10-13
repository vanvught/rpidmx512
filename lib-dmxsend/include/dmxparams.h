/**
 * @file dmxparams.h
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

#ifndef DMXPARAMS_H_
#define DMXPARAMS_H_

#include <stdint.h>

#if defined (__circle__)
 #include "circle/dmxsend.h"
#else
 #include "dmxsend.h"
 #if defined (H3)
  #include "h3/dmxsendmulti.h"
 #endif
#endif

#define DMX_PARAMS_MIN_BREAK_TIME		9	///<
#define DMX_PARAMS_DEFAULT_BREAK_TIME	9	///<
#define DMX_PARAMS_MAX_BREAK_TIME		127	///<

#define DMX_PARAMS_MIN_MAB_TIME			1	///<
#define DMX_PARAMS_DEFAULT_MAB_TIME		1	///<
#define DMX_PARAMS_MAX_MAB_TIME			127	///<

#define DMX_PARAMS_DEFAULT_REFRESH_RATE	40	///<

struct TDMXParams {
    uint32_t bSetList;
	uint8_t nBreakTime;	///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
	uint8_t nMabTime;		///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
	uint8_t nRefreshRate;	///< DMX output rate in packets per second. Valid range is 1 to 40.
};

class DMXParamsStore {
public:
	virtual ~DMXParamsStore(void);

	virtual void Update(const struct TDMXParams *pDmxParams)=0;
	virtual void Copy(struct TDMXParams *pDmxParams)=0;

private:
};

class DMXParams {
public:
	DMXParams(DMXParamsStore *pDMXParamsStore = 0);
	~DMXParams(void);

	bool Load(void);

	void Set(DMXSend *);
#if defined (H3)
	void Set(DMXSendMulti *);
#endif

	void Dump(void);

	inline uint8_t GetBreakTime(void) {
		return m_tDMXParams.nBreakTime;
	}

	inline uint8_t GetMabTime(void) {
		return m_tDMXParams.nMabTime;
	}

	inline uint8_t GetRefreshRate(void) {
		return m_tDMXParams.nRefreshRate;
	}

private:
	bool isMaskSet(uint32_t nMask) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    DMXParamsStore *m_pDMXParamsStore;
    struct TDMXParams m_tDMXParams;
};

#endif /* DMXPARAMS_H_ */
