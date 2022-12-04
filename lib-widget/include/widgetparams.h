/**
 * @file widgetparams.h
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "widgetconfiguration.h"

struct TWidgetParams {
    uint32_t nSetList;
	uint8_t nBreakTime;		///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
	uint8_t nMabTime;		///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
	uint8_t nRefreshRate;	///< DMX output rate in packets per second. Valid range is 1 to 40.
	uint8_t tMode;
	uint8_t	nThrottle;		///< DMX send to host throttle
};

struct WidgetParamsMask {
	static constexpr auto BREAK_TIME = (1U << 0);
	static constexpr auto MAB_TIME = (1U << 1);
	static constexpr auto REFRESH_RATE = (1U << 2);
	static constexpr auto MODE = (1U << 3);
	static constexpr auto THROTTLE = (1U << 4);
};

#if defined (WIDGET_HAVE_FLASHROM)
class WidgetParamsStore {
public:
	virtual ~WidgetParamsStore() {}

	virtual void Update(const struct TWidgetParams *pWidgetParams)=0;
	virtual void Copy(struct TWidgetParams *pWidgetParams)=0;
};
#else
#endif

class WidgetParams {
public:
#if defined (WIDGET_HAVE_FLASHROM)
	WidgetParams(WidgetParamsStore *pWidgetParamsStore = nullptr);
#else
	WidgetParams();
#endif

	bool Load();
	void Set();

	void Dump();

	uint8_t GetBreakTime() const {
		return m_tWidgetParams.nBreakTime;
	}

	uint8_t GetMabTime() const {
		return m_tWidgetParams.nMabTime;
	}

	uint8_t GetRefreshRate() const {
		return m_tWidgetParams.nRefreshRate;
	}

	widget::Mode GetMode() const {
		return static_cast<widget::Mode>(m_tWidgetParams.tMode);
	}

	uint8_t GetThrottle() const {
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
#if defined (WIDGET_HAVE_FLASHROM)
    WidgetParamsStore *m_pWidgetParamsStore;
#endif
    TWidgetParams m_tWidgetParams;
};

#endif /* WIDGETPARAMS_H_ */
