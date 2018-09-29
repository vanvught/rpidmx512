/**
 * @file l6470dmxmodes.h
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

#ifndef L6470DMXMODES_H_
#define L6470DMXMODES_H_

#include <stdint.h>
#include <stdbool.h>

#include "l6470.h"
#include "l6470dmxmode.h"

#include "motorparams.h"
#include "modeparams.h"

class L6470DmxModes {

public:
	L6470DmxModes(TL6470DmxModes, uint16_t, L6470 *, MotorParams *, ModeParams *);
	~L6470DmxModes(void);

	void InitSwitch(void);
	void InitPos(void);

	void HandleBusy(void);
	bool BusyCheck(void);

	bool IsDmxDataChanged(const uint8_t *, uint16_t);
	void DmxData(const uint8_t *, uint16_t);

	void Start(void);
	void Stop(void);

	inline TL6470DmxModes GetMode(void) {
		return m_nMode;
	}

public: // RDM
	inline const uint8_t GetDmxStartAddress(void) const {
		return m_nDmxStartAddress;
	}

	inline void SetDmxStartAddress(uint16_t nDmxStartAddress) {
		m_nDmxStartAddress = nDmxStartAddress;
	}

	inline uint8_t GetDmxFootPrint(void) const {
		return m_DmxFootPrint;
	}

public:
	static uint8_t GetDmxFootPrintMode(uint8_t);

private:
	bool IsDmxDataChanged(const uint8_t *);

private:
	bool m_bIsStarted;

private:
	uint8_t m_nMotorNumber;
	TL6470DmxModes m_nMode;
	uint16_t m_nDmxStartAddress;
	L6470DmxMode *m_pDmxMode;
	uint8_t m_DmxFootPrint;
	uint8_t *m_pDmxData;
};

#endif /* L6470DMXMODES_H_ */
