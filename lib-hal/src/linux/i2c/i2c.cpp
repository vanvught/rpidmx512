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

#include "debug.h"

static int i2cbus;
static constexpr char fileName[] = "/dev/i2c-1";

void i2c_begin() {
	DEBUG_ENTRY

	if (i2cbus != 0) {
		close(i2cbus);
	}

	if ((i2cbus = open(fileName, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open i2c port %s \n", strerror(errno));
	}

	DEBUG_EXIT
}

void i2c_set_baudrate([[maybe_unused]] uint32_t baudrate) {
}

void i2c_set_address(uint8_t address) {
	if (i2cbus == 0) {
		fprintf(stderr, "i2c_begin has failed\n");
		return;
	}

	if (ioctl(i2cbus, I2C_SLAVE, address) < 0) {
		fprintf(stderr, "Failed to set address %s\n", strerror(errno));
		i2cbus = 0;
	}
}

uint8_t i2c_write(const char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "i2c_begin has failed\n");
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

uint8_t i2c_read(char *buf, uint32_t len) {
	if (i2cbus == 0) {
#if !defined (NDEBUG)
		fprintf(stderr, "i2c_begin has failed\n");
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

bool i2c_is_connected(const uint8_t nAddress, const uint32_t nBaudrate) {
	i2c_set_address(nAddress);

	uint8_t nResult;
	char buffer;

	if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
		nResult = i2c_read(&buffer, 1);
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = i2c_write(nullptr, 0);
	}

	return (nResult == 0) ? true : false;
}

void i2c_write_register(const uint8_t nRegister, const uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	i2c_write(buffer, 2);
}

void i2c_read_register(const uint8_t nRegister, uint8_t& nValue) {
	char buffer[1];

	buffer[0] = static_cast<char>(nRegister);

	i2c_write(buffer, 1);
	i2c_read(buffer, 1);

	nValue = buffer[0];
}
