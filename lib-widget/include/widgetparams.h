/**
 * @file widgetparams.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

class WidgetParamsStore {
public:
#if defined (WIDGET_HAVE_FLASHROM)
	static void Update(const struct TWidgetParams* pParams) {
		ConfigStore::Get()->Update(configstore::Store::WIDGET, pParams, sizeof(struct TWidgetParams));
	}

	static void Copy(struct TWidgetParams* pParams) {
		ConfigStore::Get()->Copy(configstore::Store::WIDGET, pParams, sizeof(struct TWidgetParams));
	}
#else
	static void IUpdate(const struct TWidgetParams* pParams) { }

	static void ICopy(struct TWidgetParams* pParams) { }
#endif
};

class WidgetParams {
public:
	WidgetParams();

	void Load();
	void Set();

	uint8_t GetBreakTime() const {
		return m_Params.nBreakTime;
	}

	uint8_t GetMabTime() const {
		return m_Params.nMabTime;
	}

	uint8_t GetRefreshRate() const {
		return m_Params.nRefreshRate;
	}

	widget::Mode GetMode() const {
		return static_cast<widget::Mode>(m_Params.tMode);
	}

	uint8_t GetThrottle() const {
		return m_Params.nThrottle;
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    TWidgetParams m_Params;
};

#endif /* WIDGETPARAMS_H_ */
