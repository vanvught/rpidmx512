/**
 * @file widgetparams.h
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

#ifndef WIDGETPARAMS_H_
#define WIDGETPARAMS_H_

#include <stdint.h>

enum TWidgetParamsMask {
	WIDGET_PARAMS_MASK_BREAK_TIME = (1 << 0),
	WIDGET_PARAMS_MASK_MAB_TIME = (1 << 1),
	WIDGET_PARAMS_MASK_REFRESH_RATE = (1 << 2),
	WIDGET_PARAMS_MASK_MODE = (1 << 3),
	WIDGET_PARAMS_MASK_THROTTLE = (1 << 4)
};

enum TWidgetMode {
	WIDGET_MODE_DMX_RDM = 0,	///< Both DMX and RDM firmware enabled.
	WIDGET_MODE_DMX = 1,		///< DMX firmware enabled
	WIDGET_MODE_RDM = 2,		///< RDM firmware enabled.
	WIDGET_MODE_RDM_SNIFFER = 3	///< RDM Sniffer firmware enabled.
};

struct TWidgetParams {
    uint32_t nSetList;
	uint8_t nBreakTime;		///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
	uint8_t nMabTime;		///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
	uint8_t nRefreshRate;	///< DMX output rate in packets per second. Valid range is 1 to 40.
	TWidgetMode tMode;
	uint8_t	nThrottle;		///< DMX send to host throttle
};

class WidgetParamsStore {
public:
	virtual ~WidgetParamsStore() {}

	virtual void Update(const struct TWidgetParams *pWidgetParams)=0;
	virtual void Copy(struct TWidgetParams *pWidgetParams)=0;
};

class WidgetParams {
public:
	WidgetParams(WidgetParamsStore *pWidgetParamsStore = nullptr);

	bool Load();
	void Set();

	void Dump();

	uint8_t GetBreakTime() {
		return m_tWidgetParams.nBreakTime;
	}

	uint8_t GetMabTime() {
		return m_tWidgetParams.nMabTime;
	}

	uint8_t GetRefreshRate() {
		return m_tWidgetParams.nRefreshRate;
	}

	TWidgetMode GetMode() {
		return m_tWidgetParams.tMode;
	}

	uint8_t GetThrottle() {
		return m_tWidgetParams.nThrottle;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tWidgetParams.nSetList & nMask) == nMask;
    }

private:
    WidgetParamsStore *m_pWidgetParamsStore;
    struct TWidgetParams m_tWidgetParams;
};

#endif /* WIDGETPARAMS_H_ */
