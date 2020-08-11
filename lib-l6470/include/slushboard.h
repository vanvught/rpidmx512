/**
 * @file slushengineboard.h
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
	SLUSH_IO_PORTA,
	SLUSH_IO_PORTB
};

enum TSlushIOPins {
	SLUSH_IO_PIN0,
	SLUSH_IO_PIN1,
	SLUSH_IO_PIN2,
	SLUSH_IO_PIN3,
	SLUSH_IO_PIN4,
	SLUSH_IO_PIN5,
	SLUSH_IO_PIN6,
	SLUSH_IO_PIN7
};

enum TSlushIOFSel {
	SLUSH_IO_FSEL_OUTP,
	SLUSH_IO_FSEL_INPT
};

#define SLUSH_L6470_RESET		GPIO_EXT_16 // 23 // RPI_V2_GPIO_P1_16 // RPI_V2_GPIO_P1_38

#define SLUSH_MTR0_STEPCLOCK	GPIO_EXT_7  // 4  // RPI_V2_GPIO_P1_07
#define SLUSH_MTR1_STEPCLOCK	GPIO_EXT_29 // 5  // RPI_V2_GPIO_P1_29
#define SLUSH_MTR2_STEPCLOCK	GPIO_EXT_31 // 6  // RPI_V2_GPIO_P1_31
#define SLUSH_MTR3_STEPCLOCK	GPIO_EXT_32 // 12 // RPI_V2_GPIO_P1_32

#define SLUSH_MTR0_BUSY			GPIO_EXT_36 // 16 // RPI_V2_GPIO_P1_36 // RPI_V2_GPIO_P1_35
#define SLUSH_MTR1_BUSY			GPIO_EXT_11 // 17 // RPI_V2_GPIO_P1_11
#define SLUSH_MTR2_BUSY			GPIO_EXT_12 // 18 // RPI_V2_GPIO_P1_12
#define SLUSH_MTR3_BUSY			GPIO_EXT_35 // 19 // RPI_V2_GPIO_P1_35

#define SLUSH_MTR0_CHIPSELECT	GPIO_EXT_18 // 24 // RPI_V2_GPIO_P1_18 // RPI_V2_GPIO_P1_24
#define SLUSH_MTR1_CHIPSELECT	GPIO_EXT_22 // 25 // RPI_V2_GPIO_P1_22
#define SLUSH_MTR2_CHIPSELECT	GPIO_EXT_37 // 26 // RPI_V2_GPIO_P1_37
#define SLUSH_MTR3_CHIPSELECT	GPIO_EXT_13 // 27 // RPI_V2_GPIO_P1_13

#define SLUSH_MTR_FLAG			GPIO_EXT_33 // 13 // RPI_V2_GPIO_P1_33

class SlushBoard {
public:
	SlushBoard();
	~SlushBoard();

	void setIOState(uint8_t, uint8_t, uint8_t);
	uint8_t getIOState(uint8_t, uint8_t);

	uint16_t getTempRaw();
	float getTemprature();

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
	void InitSpi();
	void InitI2c();
	void I2cSetup(uint8_t);
	uint8_t Mcp23017ReadReg(uint8_t);
	void Mcp23017WriteReg(uint8_t, uint8_t);
};

#endif /* SLUSHBOARD_H_ */
