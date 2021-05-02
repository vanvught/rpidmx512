#if !defined(ORANGE_PI)
/**
 * @file slushboard.cpp
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_i2c.h"

#include "slushboard.h"

#define SLUSH_MCP23017_RESET	GPIO_EXT_38 // 20 // RPI_V2_GPIO_P1_38
#define SLUSH_MCP23017_INTA		GPIO_EXT_40 // 21 // RPI_V2_GPIO_P1_40
#define SLUSH_MCP23017_INTB		GPIO_EXT_15 // 22 // RPI_V2_GPIO_P1_15

#define MAX1164_I2C_ADDRESS		0x36

#define MCP23017_I2C_ADDRESS	0x20

#define MCP23017_IODIRA		0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23017_IODIRB		0x01	///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
#define MCP23017_IOCON		0x0A	///< CONFIGURATION (IOCON) REGISTER
//							0x0B	///< CONFIGURATION (IOCON) REGISTER
#define MCP23017_GPPUA		0x0C	///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23017_GPPUB		0x0D	///< PULL-UP RESISTOR CONFIGURATION (GPPUB) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23017_GPIOA		0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23017_GPIOB		0x13	///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23017_OLATA		0x14	///< OUTPUT LATCH REGISTER (OLATA), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value
#define MCP23017_OLATB		0x15	///< OUTPUT LATCH REGISTER (OLATB), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value


SlushBoard::SlushBoard(void) {
	FUNC_PREFIX(gpio_fsel(SLUSH_L6470_RESET, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_set(SLUSH_L6470_RESET));

	FUNC_PREFIX(gpio_fsel(SLUSH_MTR0_CHIPSELECT, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR1_CHIPSELECT, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR2_CHIPSELECT, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR3_CHIPSELECT, GPIO_FSEL_OUTPUT));

	FUNC_PREFIX(gpio_set(SLUSH_MTR0_CHIPSELECT));
	FUNC_PREFIX(gpio_set(SLUSH_MTR1_CHIPSELECT));
	FUNC_PREFIX(gpio_set(SLUSH_MTR2_CHIPSELECT));
	FUNC_PREFIX(gpio_set(SLUSH_MTR3_CHIPSELECT));

	FUNC_PREFIX(gpio_fsel(SLUSH_MTR0_BUSY, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR1_BUSY, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR2_BUSY, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_fsel(SLUSH_MTR3_BUSY, GPIO_FSEL_INPUT));

	FUNC_PREFIX(gpio_clr(SLUSH_L6470_RESET));
	udelay(10000);
	FUNC_PREFIX(gpio_set(SLUSH_L6470_RESET));
	udelay(10000);

	FUNC_PREFIX(gpio_fsel(SLUSH_MCP23017_RESET, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_set(SLUSH_MCP23017_RESET));

	InitSpi();
	InitI2c();
}

SlushBoard::~SlushBoard(void) {

}

void SlushBoard::InitSpi(void) {
	FUNC_PREFIX(spi_begin());

	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(4000000));
	FUNC_PREFIX(spi_setDataMode(SPI_MODE3));
}

void SlushBoard::InitI2c(void) {
	char data;

	FUNC_PREFIX(i2c_begin());

	/*
	 *  MCP23017
	 *  Initializes as input, with pull-up
	 */

	I2cSetup(MCP23017_I2C_ADDRESS);

	Mcp23017WriteReg(MCP23017_IODIRA, 0xFF);
	Mcp23017WriteReg(MCP23017_IODIRB, 0xFF);

	Mcp23017WriteReg(MCP23017_GPPUA, 0xFF);
	Mcp23017WriteReg(MCP23017_GPPUB, 0xFF);

	/*
	 * MAX1164
	 * Initializes the first channel in single ended mode with Vdd as Vref
	 */

	I2cSetup(MAX1164_I2C_ADDRESS);

	data = 0x8a;
	FUNC_PREFIX(i2c_write(&data, 1));
	data = 0x01;
	FUNC_PREFIX(i2c_write(&data, 1));
}

void SlushBoard::I2cSetup(uint8_t address) {
	FUNC_PREFIX(i2c_set_address(address));
	FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
}

uint8_t SlushBoard::Mcp23017ReadReg(uint8_t reg) {
	char data = reg;

	I2cSetup(MCP23017_I2C_ADDRESS);

	FUNC_PREFIX(i2c_write(&data, 1));
	FUNC_PREFIX(i2c_read(&data, 1));

	return data;
}

void SlushBoard::Mcp23017WriteReg(uint8_t reg, uint8_t data) {
	char buffer[2];

	buffer[0] = reg;
	buffer[1] = data;

	I2cSetup(MCP23017_I2C_ADDRESS);

	FUNC_PREFIX(i2c_write(buffer, 2));
}

void SlushBoard::setIOState(TSlushIOPorts nPort, TSlushIOPins nPinNumber, uint8_t state) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);

	IOFSel(nPort, nPinNumber, SLUSH_IO_FSEL_OUTP);

	if (state == 0) {
		IOClr(nPort, nPinNumber);
	} else {
		IOSet(nPort, nPinNumber);
	}
}

void SlushBoard::setIOState(uint8_t nPort, uint8_t nPinNumber, uint8_t state) {
	setIOState(static_cast<TSlushIOPorts>(nPort), static_cast<TSlushIOPins>(nPinNumber), state);
}


uint8_t SlushBoard::getIOState(TSlushIOPorts nPort, TSlushIOPins nPinNumber) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);

	IOFSel(nPort, nPinNumber, SLUSH_IO_FSEL_INPT);

	return IOLev(nPort, nPinNumber);
}

uint8_t SlushBoard::getIOState(uint8_t nPort, uint8_t nPinNumber) {
	return getIOState(static_cast<TSlushIOPorts>(nPort), static_cast<TSlushIOPins>(nPinNumber));
}

void SlushBoard::IOFSel(TSlushIOPorts nPort, TSlushIOPins nPinNumber, TSlushIOFSel fsel) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);
	assert(fsel <= SLUSH_IO_FSEL_INPT);

	uint8_t data = Mcp23017ReadReg(MCP23017_IODIRA + nPort);
	uint8_t pin = 1 << (nPinNumber % 8);

	if (fsel == SLUSH_IO_FSEL_OUTP) {
		data &= (~(pin));
	} else {
		data |= pin;
	}

	Mcp23017WriteReg(MCP23017_IODIRA + nPort, data);
}

void SlushBoard::IOClr(TSlushIOPorts nPort, TSlushIOPins nPinNumber) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);

	uint8_t data = IORead(nPort);

	data &= (~(1 << (nPinNumber % 8)));

	IOWrite(nPort, data);
}

void SlushBoard::IOSet(TSlushIOPorts nPort, TSlushIOPins nPinNumber) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);

	uint8_t data = IORead(nPort);

	data |= 1 << (nPinNumber % 8);

	IOWrite(nPort, data);
}

uint8_t SlushBoard::IOLev(TSlushIOPorts nPort, TSlushIOPins nPinNumber) {
	assert(nPort <= SLUSH_IO_PORTB);
	assert(nPinNumber <= SLUSH_IO_PIN7);

	uint8_t data = IORead(nPort);
	uint8_t pin = 1 << (nPinNumber % 8);

	if ((data & pin) == pin) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t SlushBoard::IORead(TSlushIOPorts nPort) {
	assert(nPort <= SLUSH_IO_PORTB);

	return Mcp23017ReadReg(MCP23017_GPIOA + nPort);
}

void SlushBoard::IOWrite(TSlushIOPorts nPort, uint8_t data) {
	assert(nPort <= SLUSH_IO_PORTB);

	Mcp23017WriteReg(MCP23017_OLATA + nPort, data);
}
#endif
