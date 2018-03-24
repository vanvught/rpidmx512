/**
 * @file slushengineboard.h
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
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

#ifndef SLUSHBOARD_H_
#define SLUSHBOARD_H_

#include <stdint.h>

enum TSlushIOPorts {
	SLUSH_IO_PORTA = 0,
	SLUSH_IO_PORTB = 1
};

enum TSlushIOPins {
	SLUSH_IO_PIN0 = 0,
	SLUSH_IO_PIN1,
	SLUSH_IO_PIN2,
	SLUSH_IO_PIN3,
	SLUSH_IO_PIN4,
	SLUSH_IO_PIN5,
	SLUSH_IO_PIN6,
	SLUSH_IO_PIN7
};

enum TSlushIOFSel {
	SLUSH_IO_FSEL_OUTP = 0,
	SLUSH_IO_FSEL_INPT = 1
};

#define SLUSH_L6470_RESET		23 // RPI_V2_GPIO_P1_38

#define SLUSH_MTR0_STEPCLOCK	4
#define SLUSH_MTR1_STEPCLOCK	5
#define SLUSH_MTR2_STEPCLOCK	6
#define SLUSH_MTR3_STEPCLOCK	12

#define SLUSH_MTR0_BUSY			16 // RPI_V2_GPIO_P1_35
#define SLUSH_MTR1_BUSY			17
#define SLUSH_MTR2_BUSY			18
#define SLUSH_MTR3_BUSY			19

#define SLUSH_MTR0_CHIPSELECT	24 // RPI_V2_GPIO_P1_24
#define SLUSH_MTR1_CHIPSELECT	25
#define SLUSH_MTR2_CHIPSELECT	26
#define SLUSH_MTR3_CHIPSELECT	27

#define SLUSH_MTR_FLAG			13

class SlushBoard {
public:
	SlushBoard(void);
	~SlushBoard(void);

	void setIOState(uint8_t, uint8_t, uint8_t);
	uint8_t getIOState(uint8_t, uint8_t);

	uint16_t getTempRaw(void);
	float getTemprature(void);

private:
	float calcTemp(uint16_t);

public:
	void setIOState(TSlushIOPorts, TSlushIOPins, uint8_t);
	uint8_t getIOState(TSlushIOPorts, TSlushIOPins);

	uint8_t IORead(TSlushIOPorts);
	void IOWrite(TSlushIOPorts, uint8_t);

	void IOFSel(TSlushIOPorts, TSlushIOPins, TSlushIOFSel);
	void IOClr(TSlushIOPorts, TSlushIOPins);
	void IOSet(TSlushIOPorts, TSlushIOPins);
	uint8_t IOLev(TSlushIOPorts, TSlushIOPins);

private:
	void InitSpi(void);
	void InitI2c(void);
	void I2cSetup(uint8_t);
	uint8_t Mcp23017ReadReg(uint8_t);
	void Mcp23017WriteReg(uint8_t, uint8_t);
};

#endif /* SLUSHBOARD_H_ */
