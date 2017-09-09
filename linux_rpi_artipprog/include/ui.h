/**
 * @file ui.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef UI_H_
#define UI_H_

#include <stdint.h>
#include <time.h>

#include "inputset.h"
#include "display.h"

#include "artnetcontroller.h"
#include "artnetipprog.h"

enum TState {
	STATE_INIT = 0,
	STATE_SHOW_CURSOR_OFF,
	STATE_SHOW_CURSOR_ON,
	STATE_SHOW_DETAILS,
	STATE_IN_IPADDRESS,
	STATE_IN_SUBMASK,
	STATE_IPADDRESS_EDIT,
	STATE_SUBMASK_EDIT,
	STATE_SEND_IPPROG
};

class Ui {
public:
	Ui(const char *, const ArtNetController *, const InputSet *);
	~Ui(void);

	void Run(void);

private:
	void Show();
	void ShowDetails(void);
	void Update();

	void KeyNumeric(const int);
	void KeyEnter(void);
	void KeyEsc(void);
	void KeyUp(void);
	void KeyDown(void);
	void KeyLeft(void);
	void KeyRight(void);

	void HandleIpAddressDown(void);
	void HandleIpAddressUp(void);
	void SetIpAddressDigit(void);
	void UpdateIpAddressDigit(void);
	void HandleIpAddressChange(void);

	void SendIpProg(void);

#ifndef NDEBUG
	void DebugShowState(void);
#endif

private:
	ArtNetController *m_pArtNetController;
	InputSet *m_pInput;
	Display *m_pDisplay;
	uint8_t	m_nStartIndex;
	TState m_State;
	time_t m_nTimePrevious;
	uint8_t	m_nCursorRow;
	uint8_t	m_nCursorCol;
	uint8_t m_ShowDetailIndex;
	uint8_t m_IPAddress[4];
	uint32_t m_nIPAddress;
	uint8_t m_Submask[4];
	uint32_t m_nSubmask;
	char m_sDigit[4];
	int m_nDigitIndex;
	int m_nDigit;
	int m_nDigitValue;
	TArtNetIpProg m_ArtNetIpProg;
};

#endif /* UI_H_ */
