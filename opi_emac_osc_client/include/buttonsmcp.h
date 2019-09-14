/**
 * @file buttonsmcp.h
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

#ifndef BUTTONSMCP_H_
#define BUTTONSMCP_H_

#include <stdint.h>
#include <stdbool.h>

#include "buttonsset.h"
#include "oscclientled.h"

#include "oscclient.h"

class ButtonsMcp: public ButtonsSet, OscClientLed {
public:
	ButtonsMcp(OscClient *pOscClient);
	~ButtonsMcp(void);

	bool Start(void);
	void Stop(void);

	void Run(void);

	void SetLed(uint8_t nLed, bool bOn);

private:
	OscClient *m_pOscClient;
	bool m_bIsConnected;
	uint8_t m_nButtons;
	uint8_t m_nButtonsPrevious;
	uint8_t m_nPortB;
};

#endif /* BUTTONSMCP_H_ */
