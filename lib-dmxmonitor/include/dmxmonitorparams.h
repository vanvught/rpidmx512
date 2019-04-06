/**
 * @file dmxmonitorparams.h
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

#ifndef DMXMONITORPARAMS_H_
#define DMXMONITORPARAMS_H_

#include <stdint.h>

#include "dmxmonitor.h"

struct TDMXMonitorParams {
    uint32_t nSetList;
    uint16_t nDmxStartAddress;
    uint16_t nDmxMaxChannels;
    TDMXMonitorFormat tFormat;
};

class DMXMonitorParamsStore {
public:
	virtual ~DMXMonitorParamsStore(void);

	virtual void Update(const struct TDMXMonitorParams *pDMXMonitorParams)=0;
	virtual void Copy(struct TDMXMonitorParams *pDMXMonitorParams)=0;
};

class DMXMonitorParams {
public:
	DMXMonitorParams(DMXMonitorParamsStore *pDMXMonitorParamsStore = 0);
	~DMXMonitorParams(void);

	bool Load(void);
	
	void Set(DMXMonitor *pDMXMonitor);
	
	void Dump(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const;

private:
    DMXMonitorParamsStore *m_pDMXMonitorParamsStore;
    struct TDMXMonitorParams m_tDMXMonitorParams;
};

#endif /* DMXMONITORPARAMS_H_ */
