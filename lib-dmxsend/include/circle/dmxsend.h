/**
 * @file dmxsender.h
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2016  R. Stange <rsta2@o2online.de>
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DMXSEND_H_
#define DMXSEND_H_

#include <stdint.h>

#include <circle/interrupt.h>
#include <circle/gpiopin.h>
#include <circle/spinlock.h>
#include <circle/types.h>

#include "lightset.h"

enum TDMXSendState {
	DMXSendIdle,
	DMXSendBreak,
	DMXSendMAB,
	DMXSendData,
	DMXSendInterPacket,
	DMXSendUnknown
};

enum {
	DMX_UNIVERSE_SIZE = 512	///< The number of slots in a DMX512 universe.
};

class DMXSend: public LightSet {
public:
	DMXSend(CInterruptSystem *);
	~DMXSend(void);

	boolean Initialize (void);

	void Start(uint8_t nPort);
	void Stop(uint8_t nPort);

	void SetData(/* unused */uint8_t, const uint8_t *, uint16_t);	// waits if transfer is active

	void SetDataLength(uint16_t);
	uint16_t GetDataLength(void) const;

	void SetDmxBreakTime(uint32_t);
	uint32_t GetDmxBreakTime(void) const;

	void SetDmxMabTime(uint32_t);
	uint32_t GetDmxMabTime(void) const;

	void SetDmxPeriodTime(uint32_t);
	uint32_t GetDmxPeriodTime(void) const;
	uint32_t GetDmxPeriodTimeRequested(void) const;

	void Print(void);

private:
	void ClearOutputData(void);
	boolean SetDataTry(const uint8_t *, const uint16_t);	// returns FALSE if transfer is active

	void SerialIRQHandler (void);
	static void SerialIRQHandler (void *pParam);

	void TimerIRQHandler (void);
	static void TimerIRQHandler (void *pParam);

private:
	CInterruptSystem *m_pInterruptSystem;
	boolean m_bIRQsConnected;

#if RASPPI == 3
	CGPIOPin m_GPIO32;
	CGPIOPin m_GPIO33;
#endif
	CGPIOPin m_TxDPin;
	CGPIOPin m_RxDPin;
	CGPIOPin m_DMXDataDirection;

	uint32_t m_OutputBreakTime;
	uint32_t m_OutputMabTime;
	uint32_t m_OutputPeriod;
	uint32_t m_OutputPeriodRequested;
	uint16_t m_OutputDataLength;
	uint8_t m_OutputBuffer[DMX_UNIVERSE_SIZE + 1];	///< SC + UNIVERSE SIZE

	volatile TDMXSendState m_State;
	volatile uint32_t m_TimerIRQMicros;
	volatile uint32_t m_SendBreakMicros;
	volatile unsigned m_CurrentSlot;

	CSpinLock m_SpinLock;
};

#endif /* DMXSEND_H_ */
