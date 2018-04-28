/**
 * @file oscparams.h
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

#ifndef OSCPARAMS_H_
#define OSCPARAMS_H_

#include <stdint.h>

class OSCParams {
public:
	OSCParams(void);
	~OSCParams(void);

	bool Load(void);
	void Dump(void);

	inline uint16_t GetIncomingPort(void) {
		return m_nIncomingPort;
	}

	inline  uint16_t GetOutgoingPort(void) {
		return m_nOutgoingPort;
	}

private:
	bool isMaskSet(uint16_t) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    uint32_t m_bSetList;
	uint16_t m_nIncomingPort;
	uint16_t m_nOutgoingPort;
};

#endif /* OSCPARAMS_H_ */
