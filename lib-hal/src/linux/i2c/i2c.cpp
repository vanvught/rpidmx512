/**
 * @file i2c.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

 #include "firmware/debug/debug_debug.h"

static int i2cbus;
static constexpr char fileName[] = "/dev/i2c-1";

void I2cBegin() {
	DEBUG_ENTRY();

	if (i2cbus != 0) {
		close(i2cbus);
	}

	if ((i2cbus = open(fileName, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open i2c port %s \n", strerror(errno));
	}

	DEBUG_EXIT();
}

void I2cSetBaudrate([[maybe_unused]] uint32_t baudrate) {
}

void I2cSetAddress(uint8_t address) {
	if (i2cbus == 0) {
		fprintf(stderr, "I2cBegin has failed\n");
		return;
	}

	if (ioctl(i2cbus, I2C_SLAVE, address) < 0) {
		fprintf(stderr, "Failed to set address %s\n", strerror(errno));
		i2cbus = 0;
	}
}

uint8_t I2cWrite(const char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "I2cBegin has failed\n");
#endif
		return 1;
	}

	if (write(i2cbus, buf, len) != static_cast<ssize_t>(len)) {
#if !defined (NDEBUG)
		fprintf(stderr, "Failed to write i2c %s\n", strerror(errno));
#endif
		return 1;
	}

	return 0;
}

uint8_t I2cRead(char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "I2cBegin has failed\n");
#endif
		return 1;
	}

	if (read(i2cbus, buf, len) != static_cast<ssize_t>(len)) {
#if !defined (NDEBUG)
		fprintf(stderr, "Failed to read i2c %s\n", strerror(errno));
#endif
		return 1;
	}

	return 0;
}

bool I2cIsConnected(const uint8_t address, const uint32_t nBaudrate) {
	I2cSetAddress(address);

	uint8_t nResult;
	char buffer;

	if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
		nResult = I2cRead(&buffer, 1);
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = I2cWrite(nullptr, 0);
	}

	return (nResult == 0) ? true : false;
}

void I2cWriteReg(const uint8_t nRegister, const uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	I2cWrite(buffer, 2);
}

void I2cReadReg(const uint8_t nRegister, uint8_t& nValue) {
	char buffer[1];

	buffer[0] = static_cast<char>(nRegister);

	I2cWrite(buffer, 1);
	I2cRead(buffer, 1);

	nValue = buffer[0];
}
