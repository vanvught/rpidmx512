/**
 * @file l6470dmxmode.h
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

#ifndef L6470DMXMODE_H_
#define L6470DMXMODE_H_

#include <stdint.h>

enum TL6470DmxModes {
	L6470DMXMODE0 = 0,
	L6470DMXMODE1,
	L6470DMXMODE2,
	L6470DMXMODE3,
	L6470DMXMODE4,
	L6470DMXMODE5,
	L6470DMXMODE6,
	L6470DMXMODE_UNDEFINED = 255
};

class L6470DmxMode {
public:
	virtual ~L6470DmxMode(void);

	virtual void InitSwitch(void);
	virtual void InitPos(void);

	virtual void Start(void)= 0;
	virtual void Stop(void)= 0;

	virtual void HandleBusy(void);
	virtual bool BusyCheck(void);

	virtual void Data(const uint8_t *)= 0;
};

#endif /* L6470DMXMODE_H_ */
